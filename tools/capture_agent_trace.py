"""One COM9 owner: verify agent tracing, then capture physical clicks."""
import time, pathlib, serial
out=pathlib.Path('D:/esp32/magepie-esp32/build_logs/agent-physical-trace.log')
s=serial.Serial(port=None,baudrate=921600,timeout=.1);s.port='COM9';s.dtr=False;s.rts=False;s.open()
with out.open('w',encoding='utf8',buffering=1) as f:
    f.write('CAPTURE_OPEN\n')
    raw=b''; deadline=time.monotonic()+12
    while time.monotonic()<deadline:
        c=s.read(8192);raw+=c
        if b'[boot] ready' in raw:break
    f.write(raw.decode('utf8','replace'))
    s.write(b'VB');s.flush();raw=b'';deadline=time.monotonic()+20
    while time.monotonic()<deadline:
        c=s.read(8192);raw+=c;f.write(c.decode('utf8','replace'))
        if b'[sheettest] END' in raw:break
    if b'[agenttrace]' not in raw or b'[sheettest] END total=6 failed=0' not in raw:
        f.write('\nTRACE_VERIFICATION_FAILED\n');raise RuntimeError('Agent trace self-test failed')
    s.write(b'4q');s.flush()
    f.write('\nPHYSICAL_CAPTURE_READY synthetic self-test complete\n')
    print('PHYSICAL_CAPTURE_READY',flush=True)
    deadline=time.monotonic()+600
    while time.monotonic()<deadline:
        c=s.read(8192)
        if c:f.write(c.decode('utf8','replace'))
    s.write(b'Vq');s.flush();f.write('\nCAPTURE_END\n')
s.close()
