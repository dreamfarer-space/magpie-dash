import time, serial, pathlib
path=pathlib.Path('D:/esp32/magepie-esp32/build_logs/physical-taps.log')
s=serial.Serial(port=None,baudrate=921600,timeout=.1); s.port='COM9'; s.dtr=False; s.rts=False; s.open()
with path.open('w',encoding='utf8',buffering=1) as f:
    f.write('CAPTURE_OPEN\n')
    raw=b''; end=time.monotonic()+15
    while time.monotonic()<end:
        c=s.read(4096); raw+=c
        if b'[boot] ready' in raw: break
    f.write(raw.decode('utf8','replace'))
    s.write(b'Vq'); s.flush()
    end=time.monotonic()+180; buf=b''; confirmed=False
    while time.monotonic()<end:
        c=s.read(8192)
        if not c: continue
        f.write(c.decode('utf8','replace'))
        buf=(buf+c)[-4096:]
        if not confirmed and b'[dbg] verbose on' in buf:
            f.write('\nCAPTURE_READY\n'); print('CAPTURE_READY',flush=True); confirmed=True
    s.write(b'Vq'); s.flush(); f.write('\nCAPTURE_END\n')
s.close()
