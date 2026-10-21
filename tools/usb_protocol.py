"""Length+CRC framing; no network listener, only a fixed read-only path set."""
import re, zlib
MAX_BODY=262144
PERIODS=('today','7d','30d','all')
PATHS={'/v1/magpie/quotas','/dash/agents'}
PATHS.update('/v1/magpie/usage?period='+p for p in PERIODS)
PATHS.update('/v1/magpie/usage/requests?period=%s&limit=12'%p for p in PERIODS)
def allowed_path(path):return path in PATHS

def frame(kind,seq,body):
    if kind not in ('H','R','C') or not isinstance(body,bytes) or len(body)>MAX_BODY or seq<0:
        raise ValueError('invalid frame')
    return f'!MG1 {kind} {seq} {len(body)} {zlib.crc32(body)&0xffffffff}\n'.encode('ascii')+body

def parse_request(line):
    m=re.search(rb'MG_GET ([0-9]+) ([^\r\n ]+)',line)
    if not m:return None
    try:path=m[2].decode('ascii')
    except UnicodeDecodeError:return None
    if not allowed_path(path):return None
    return int(m[1]),path
