import argparse, time, serial, pathlib, re, json
p=argparse.ArgumentParser(); p.add_argument('--label',default='baseline'); p.add_argument('--command', default='A'); p.add_argument('--seconds',type=int,default=45); a=p.parse_args()
s=serial.Serial(port=None, baudrate=921600,timeout=.1); s.port='COM9'; s.dtr=False; s.rts=False; s.open()
log=pathlib.Path('D:/esp32/magepie-esp32/build_logs')/('tap-'+a.label+'.log')
raw=b''; end=time.monotonic()+15
while time.monotonic()<end:
    raw+=s.read(8192)
    if b'[boot] ready' in raw: break
# No DTR/RTS changes after this point: same connection for commands and results.
s.write(a.command.encode()); s.flush(); end=time.monotonic()+a.seconds
while time.monotonic()<end:
    raw+=s.read(16384)
    if b'[taptest] END' in raw or b'[sheettest] END' in raw: break
s.close(); text=raw.decode('utf8','replace').replace('\r',''); log.write_text(text,encoding='utf8')
print('LOG',log)
for line in text.splitlines():
    if any(k in line for k in ('[taptest]', '[sheettest]', '[boot] ready', 'Guru', 'assert', 'ERROR')): print(line)
if a.command in ('A', 'B'):
    prefix = 'taptest' if a.command == 'A' else 'sheettest'
    m=re.search(r'\[' + prefix + r'\] END total=(\d+) failed=(\d+)',text)
    if not m: raise SystemExit('FAIL: no test completion')
    print('RESULT',json.dumps({'total':int(m[1]),'failed':int(m[2])})); raise SystemExit(int(m[2])>0)
