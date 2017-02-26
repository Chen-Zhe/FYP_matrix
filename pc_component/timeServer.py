import socket
import struct
import time
import threading
import math

class workThread(threading.Thread):
    def __init__(self,socket):
        threading.Thread.__init__(self)
        self.socket = socket
        self.work = True

    def run(self):
        while self.work:
            try:
                data, addr = self.socket.recvfrom(1024)
                rxTimestamp = time.clock()
                packet = struct.pack("II",
                                     int(rxTimestamp),
                                     int(math.modf(rxTimestamp)[0]*1000000),
                                     )
                self.socket.sendto(packet, addr)
                #print rxTimestamp
            except socket.error:
                continue
  
class TimeServer():
    def  __init__(self):
        self.soc = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        self.soc.bind(("0.0.0.0",1230))

        self.workThread = workThread(self.soc)
        time.clock()

        print "Windows Perf Timer :1230"
        self.workThread.start()

    def stop(self):
        self.workThread.work = False
        self.soc.sendto("stop",("localhost",1230))
        self.workThread.join()
        self.soc.close()
        