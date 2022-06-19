from http.server import BaseHTTPRequestHandler, HTTPServer
from ftplib import FTP_TLS, error_perm
import datetime

class handler(BaseHTTPRequestHandler):
    def do_POST(self):

        content_len = int(self.headers.get('Content-Length'))
        post_body = self.rfile.read(content_len).decode()

        try:
            ftp = FTP_TLS('192.168.43.254', user='client', passwd='')
            ftp = ftp.prot_p()
        
        except error_perm as e:
            print(e)
            self.send_response(403)
        
        print("Connected to FTP server")
        
        mydate = datetime.datetime.now()
        filename = mydate.strftime("%d_%m_%Y_counterdata-1.csv")

        with open(filename, "a") as f:
            f.write(post_body)
        
        file = open(filename, 'rb')
        ftp.storbinary(f'STOR {filename}', file)
        file.close()
        ftp.quit()

        message = "OK"
        self.wfile.write(message.encode("utf-8"))

        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()

with HTTPServer(('192.168.43.133', 8080), handler) as server:
    server.serve_forever()