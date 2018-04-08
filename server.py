#!/usr/bin/python
import os
import cgi
import sys
import time
import string
import signal
import subprocess
from os import sep
from subprocess import call
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

PORT_NUMBER = 80
proc = None
CURRENT_DIRECTORY = os.path.dirname(os.path.abspath(__file__)) + sep

def end_it_all():
    print('^C received, shutting down the web server and LED display process')
    server.socket.close()
    if proc != None:
        print("killing process")
        proc.kill()

# Handle signal termination
def sigterm_handler(_signo, _stack_frame):
    end_it_all()
    sys.exit(0)

#This class will handles any incoming request from
#the browser
class SignServerHandler(BaseHTTPRequestHandler):

    #Handler for the GET requests
    def do_GET(self):

        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()

        if self.path == "/":
            self.path = "index.html"
            try:
                filepath = CURRENT_DIRECTORY + self.path
                print(filepath)
                f = open(filepath)
                self.wfile.write(f.read())
            except IOError:
                self.send_error(404, 'File Not Found: %s' % self.path)
        else:
            self.wfile.write("")
        return

    def do_POST(self):
        global proc
        print(self.path)
        print("!!! POST OCCURED !!!")
        ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
        if ctype == 'multipart/form-data':
            postvars = cgi.parse_multipart(self.rfile, pdict)
        elif ctype == 'application/x-www-form-urlencoded':
            length = int(self.headers.getheader('content-length'))
            postvars = cgi.parse_qs(self.rfile.read(length), keep_blank_values=1)
        else:
            postvars = {}

        print(postvars)
        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()

        try:
            if proc != None:
                print("killing process")
                proc.kill()

            command = [
                CURRENT_DIRECTORY + "sce_sign",
                "--set-text", postvars['text'][0],
                "--set-brightness", postvars['brightness'][0],
                "--set-speed", postvars['speed'][0],
                "--set-background-color", postvars['background'][0][1:],
                "--set-font-color", postvars['font'][0][1:],
                "--set-border-color", postvars['border'][0][1:],
                "--set-font-filename", CURRENT_DIRECTORY + "fonts/9x18B.bdf",
            ]
            print(command)

            proc = subprocess.Popen(command)
            self.wfile.write("SUCCESS")
        except  Exception, err:
            print Exception, err
            print("FAILED TO EXECUTE SHELL CALL")
            self.wfile.write("FAILED TO EXECUTE SHELL CALL")
            return

        print("/n/n")
        return

try:
    #Create a web server and define the handler to manage the
    #incoming request
    signal.signal(signal.SIGTERM, sigterm_handler)
    server = HTTPServer(('', PORT_NUMBER), SignServerHandler)
    print('Started httpserver on port ' , PORT_NUMBER)

    #Wait self.pathforever for incoming htto requests
    server.serve_forever()

except KeyboardInterrupt:
    end_it_all()