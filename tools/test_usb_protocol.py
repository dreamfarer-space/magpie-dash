import unittest, zlib
from usb_protocol import frame, parse_request, allowed_path, MAX_BODY

class ProtocolTests(unittest.TestCase):
    def test_crc_and_length_are_bytes_not_characters(self):
        body='{"name":"智能体"}'.encode('utf8')
        h,b=frame('R',7,body).split(b'\n',1)
        self.assertEqual(h.decode().split(), ['!MG1','R','7',str(len(body)),str(zlib.crc32(body)&0xffffffff)])
        self.assertEqual(b,body)
    def test_control_frame_roundtrip(self):
        h,b=frame('C',0,b'q').split(b'\n',1)
        self.assertEqual(h.decode().split(), ['!MG1','C','0','1',str(zlib.crc32(b'q')&0xffffffff)])
        self.assertEqual(b,b'q')
    def test_too_large_rejected(self):
        with self.assertRaises(ValueError):frame('R',1,b'x'*(MAX_BODY+1))
    def test_parse_valid_request_among_logs(self):
        self.assertEqual(parse_request(b'[boot] ready MG_GET 21 /v1/magpie/usage?period=today\r\n'),(21,'/v1/magpie/usage?period=today'))
    def test_logs_are_not_requests(self):
        self.assertIsNone(parse_request(b'[gw] ok=1 prov=6'))
    def test_only_readonly_dashboard_paths(self):
        for p in ['/v1/magpie/quotas','/dash/agents']+['/v1/magpie/usage?period=%s'%x for x in ('today','7d','30d','all')]                  +['/v1/magpie/usage/requests?period=%s&limit=12'%x for x in ('today','7d','30d','all')]:
            self.assertTrue(allowed_path(p),p)
        for p in ['/v1/chat/completions','/admin','http://evil.invalid','/dash/agents/../secrets','/v1/magpie/usage?period=today&other=1','/v1/magpie/usage/requests?period=30d&limit=999']:
            self.assertFalse(allowed_path(p),p)
    def test_invalid_request_denied(self):
        self.assertIsNone(parse_request(b'MG_GET 2 /v1/chat/completions'))
if __name__=='__main__':unittest.main()
