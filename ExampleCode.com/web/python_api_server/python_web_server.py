#!/usr/bin/env python
# -*- coding: utf8 -*-
import httplib
import os
import socket
import SocketServer
import SimpleHTTPServer
import re
import ssl

PORT = 1443
RMSA_WEB_SUTDOWN ='rmsa_web_shutdown'



class RmsaHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        if None != re.search('/healthcheck', self.path):
            #This URL will trigger our sample function and send what it returns back to the browser
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write('rmsa-alive') #call sample function here
            return
        elif None != re.search('/rule-info', self.path):
            # Load Rule info
            h = httplib.HTTPSConnection('10.0.8.11:1443')
            h.request('GET', '/healthcheck')
            r = h.getresponse()
            rr = r.read()
            print('Content:' + rr)
            #This URL will trigger our sample function and send what it returns back to the browser
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write('rule-info') #call sample function here
            return
        elif None != re.search('/api/shutdown-server', self.path):
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            f = open(RMSA_WEB_SUTDOWN, 'w')
            f.close()
            self.wfile.write(str('bye bye')) #call sample function here
            return
        else:
            #serve files, and directory listings by following self.path from
            #current working directory
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write('Not Supported API') #call sample function here
            #SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

#socket reuse 옵션 설정
SocketServer.TCPServer.allow_reuse_address = True
httpd = SocketServer.ThreadingTCPServer(('', PORT), RmsaHandler)
#SSL Wrapping
httpd.socket = ssl.wrap_socket(httpd.socket, certfile='sniper.pem', keyfile='sniper.pem')

def start_server():
  try:
    if os.path.isfile(RMSA_WEB_SUTDOWN):
      os.remove(RMSA_WEB_SUTDOWN)
    print "serving at port", PORT
    httpd.serve_forever()
  except KeyboardInterrupt:
    shutdown_server();

def shutdown_server():
  if os.path.isfile(RMSA_WEB_SUTDOWN):
    os.remove(RMSA_WEB_SUTDOWN)
  print "server is shutdown ", PORT
  httpd.shutdown();
