"""Read-only magpie -> ESP32 USB bridge. Never binds a network listening port."""
import argparse, concurrent.futures, ctypes, datetime, json, logging, logging.handlers
import os, pathlib, queue, re, sys, time, urllib.request
import serial
from serial.tools import list_ports
from usb_protocol import frame, parse_request
import gw_relay

ROOT=pathlib.Path(__file__).resolve().parents[1]
LOGDIR=ROOT/'build_logs'; LOGDIR.mkdir(exist_ok=True)
STATUS=LOGDIR/'usb_bridge_status.json'
log=logging.getLogger('usbbridge');log.setLevel(logging.INFO)
h=logging.handlers.RotatingFileHandler(LOGDIR/'usb_bridge.log',maxBytes=1000000,backupCount=3,encoding='utf8')
h.setFormatter(logging.Formatter('%(asctime)s %(message)s'));log.addHandler(h)
if sys.stdout:
    log.addHandler(logging.StreamHandler(sys.stdout))
OPENER=urllib.request.build_opener(urllib.request.ProxyHandler({}))

def get_json(path):
    with OPENER.open('http://127.0.0.1:3425'+path,timeout=3) as r:
        if r.status!=200:raise RuntimeError('HTTP '+str(r.status))
        return json.load(r)
gw_relay.gw_get=get_json

ROW_FIELDS=('t','agent','agentName','provider','model','stop','in','out','cache_read','ms','status','session','hms')

USAGE_KEYS=('period','calls','errors','input','output','cache_read','cost','unpriced')
AGENT_KEYS=('id','name','icon','calls','input','output','cache_read','cost')
SERIES_KEYS=('label','input','output','cache_read','calls')  # cache_read feeds the chart

def slim_usage(data):
    """Keep only the fields fill_usage()/the chart read (12.8 KB -> a few KB)."""
    out={k:data.get(k) for k in USAGE_KEYS if data.get(k) is not None}
    out['agents']=[{k:a.get(k) for k in AGENT_KEYS if a.get(k) is not None}
                   for a in (data.get('agents') or [])]
    out['series']=[{k:x.get(k) for k in SERIES_KEYS if x.get(k) is not None}
                   for x in (data.get('series') or [])[:31]]
    return out

def local_hms(t):
    """ISO timestamp (with Z or an offset) -> local HH:MM:SS.

    magpie mixes UTC ("...Z") and PC-offset timestamps; the firmware used to copy
    the wall-clock part verbatim, so UTC rows showed up hours off and the route
    list looked out of order.
    """
    if not t:
        return None
    try:
        ts=datetime.datetime.fromisoformat(str(t).replace('Z','+00:00'))
        if ts.tzinfo is None:
            ts=ts.replace(tzinfo=datetime.timezone.utc)
        return ts.astimezone().strftime('%H:%M:%S')
    except ValueError:
        return None

def row_instant(r):
    try:
        ts=datetime.datetime.fromisoformat(str(r.get('t','')).replace('Z','+00:00'))
        if ts.tzinfo is None:
            ts=ts.replace(tzinfo=datetime.timezone.utc)
        return ts.timestamp()
    except ValueError:
        return 0.0

def slim_requests(data):
    """Drop the per-bucket series and unused row fields.

    The firmware needs the 12 newest rows plus three token sums; magpie sends a
    ~45 KB document (series dominates). Serialising that over a paced USB link
    and parsing it on the ESP32 was the bulk of the 1.9 s refresh.
    """
    # newest first by real instant: the gateway interleaves sources, so its own
    # order is not chronological (some rows carry Z timestamps).
    src=sorted(data.get('rows') or [], key=row_instant, reverse=True)[:12]
    rows=[]
    for r in src:
        row={k:r.get(k) for k in ROW_FIELDS if r.get(k) is not None}
        hms=local_hms(r.get('t'))
        if hms: row['hms']=hms
        rows.append(row)
    ser=data.get('series') or []
    return {'total':data.get('total'),
            'sum_in':sum(int(x.get('input') or 0) for x in ser),
            'sum_out':sum(int(x.get('output') or 0) for x in ser),
            'sum_cache':sum(int(x.get('cache_read') or 0) for x in ser),
            'rows':rows}

def fetch(path):
    t=time.monotonic()
    try:
        data=gw_relay.dash_agents() if path=='/dash/agents' else get_json(path)
        if path.startswith('/v1/magpie/usage/requests'):
            data=slim_requests(data)
        elif path.startswith('/v1/magpie/usage?period='):
            data=slim_usage(data)
        body=json.dumps(data,ensure_ascii=False,separators=(',',':')).encode('utf8')
        return body,round((time.monotonic()-t)*1000),None
    except Exception as e:
        return json.dumps({'_usb_error':type(e).__name__}).encode(),0,type(e).__name__

def save_status(st):
    st['updated_at']=datetime.datetime.now().astimezone().isoformat()
    tmp=STATUS.with_suffix('.tmp');tmp.write_text(json.dumps(st,ensure_ascii=False,indent=2),encoding='utf8');os.replace(tmp,STATUS)

def port_name(preferred):
    ports=[p for p in list_ports.comports() if p.vid==0x303a and p.pid==0x1001]
    if any(p.device==preferred for p in ports):return preferred
    if len(ports)==1:return ports[0].device
    return None

def send_frame(s,kind,seq,body):
    raw=frame(kind,seq,body)
    # The MCU services at most 2048 bytes per UI iteration. Pace chunks so an
    # HTTP body cannot overflow its serial RX ring while the screen repaints.
    for start in range(0,len(raw),512):
        s.write(raw[start:start+512]);time.sleep(.006)
    s.flush()

def main():
    p=argparse.ArgumentParser();p.add_argument('--port',default='COM9');p.add_argument('--seconds',type=int,default=0)
    p.add_argument('--command',default='',help='debug command chars sent once inside a CRC-checked frame (e.g. q)')
    p.add_argument('--command-every',type=float,default=0,help='repeat that command every N seconds (stress test)')
    a=p.parse_args()
    mutex=None
    if os.name=='nt':
        k=ctypes.WinDLL('kernel32',use_last_error=True);k.CreateMutexW.restype=ctypes.c_void_p
        mutex=k.CreateMutexW(None,False,'Local\\MagpieDashUsbBridge')
        if ctypes.get_last_error()==183:
            log.info('Bridge already running; duplicate launch ignored');return
    st={'pid':os.getpid(),'connected':False,'mode':'USB-only','requests':0,'responses':0,'errors':0,'reconnects':0}
    end=time.monotonic()+a.seconds if a.seconds else float('inf')
    pool=concurrent.futures.ThreadPoolExecutor(max_workers=1)
    log.info('START pid=%s; loopback magpie:3425 -> USB; no LAN listener',os.getpid())
    try:
        while time.monotonic()<end:
            s=None
            try:
                name=port_name(a.port)
                if not name:
                    st.update(connected=False,port=None);save_status(st);time.sleep(2);continue
                s=serial.Serial(port=None,baudrate=921600,timeout=.1,write_timeout=3)
                s.port=name;s.dtr=False;s.rts=False;s.open()
                st.update(connected=True,port=name);st['reconnects']+=1;save_status(st)
                log.info('USB opened %s',name)
                next_hello=0;buf=b'';pending=None;pending_seq=0;pending_path='';queued=None
                cmd_due=time.monotonic()+3.0 if a.command else None
                next_cmd=time.monotonic()+1.5
                while time.monotonic()<end:
                    if time.monotonic()>=next_hello:
                        send_frame(s,'H',0,json.dumps({'epoch':int(time.time())}).encode())
                        next_hello=time.monotonic()+5
                    if a.command and time.monotonic()>=next_cmd:
                        send_frame(s,'C',0,a.command.encode())
                        next_cmd=time.monotonic()+(a.command_every or float('inf'))
                        log.info('COMMAND sent %r',a.command)
                    if pending and pending.done():
                        body,ms,error=pending.result()
                        send_frame(s,'R',pending_seq,body)
                        st['responses']+=1
                        if error:st['errors']+=1
                        st.update(last_request=pending_path,last_http_ms=ms,last_response_bytes=len(body))
                        log.info('RESPONSE seq=%d path=%s bytes=%d http_ms=%d error=%s',pending_seq,pending_path,len(body),ms,error)
                        pending=None;save_status(st)
                        if queued:
                            pending_seq,pending_path=queued;queued=None
                            pending=pool.submit(fetch,pending_path)
                    c=s.read(8192)
                    if not c:continue
                    buf+=c
                    if len(buf)>262144:buf=buf[-8192:]
                    while b'\n' in buf:
                        line,buf=buf.split(b'\n',1)
                        req=parse_request(line)
                        if req:
                            pending_seq,pending_path=req;st['requests']+=1
                            if pending and not pending.done():
                                # Serve the newest request after the current one instead of
                                # dropping it: a dropped request costs the board a whole cycle.
                                queued=(pending_seq,pending_path);log.info('Queued seq=%d path=%s',pending_seq,pending_path)
                            else:
                                pending=pool.submit(fetch,pending_path)
                        else:
                            text=line.decode('utf8','replace').strip()
                            if text:
                                if re.fullmatch(r'[0-9a-fA-F]{16,}',text):
                                    st['hex_dump_lines']=st.get('hex_dump_lines',0)+1
                                    continue
                                log.info('BOARD %s',text)
                                if '[gw] ok=' in text:
                                    st['last_board_data']=text;st['last_board_data_at']=time.time();save_status(st)
                                if '[usb] link up' in text:st['board_handshake']=True;save_status(st)
            except (serial.SerialException,OSError,ValueError) as e:
                st.update(connected=False,last_error=type(e).__name__);save_status(st)
                log.warning('USB unavailable (%s); reconnecting',type(e).__name__)
                time.sleep(2)
            finally:
                if s and s.is_open:s.close()
    finally:
        pool.shutdown(wait=False,cancel_futures=True);st['connected']=False;save_status(st)
        if mutex:k.CloseHandle(ctypes.c_void_p(mutex))
        log.info('STOP')
if __name__=='__main__':main()
