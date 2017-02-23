import socket
import struct
import time
import Queue
import threading
import select
import math

class RecvThread(threading.Thread):
    def __init__(self, socket, queue):
        threading.Thread.__init__(self)
        self.socket = socket
        self.taskQueue = queue
        self.work = True

    def run(self):
        while self.work:
            rlist,wlist,elist = select.select([self.socket],[],[],1);
            if len(rlist) != 0:
                #print "Received %d packets" % len(rlist)
                for tempSocket in rlist:
                    try:
                        data,addr = tempSocket.recvfrom(1024)
                        recvTimestamp = time.clock()
                        self.taskQueue.put((addr,recvTimestamp))
                    except socket.error,msg:
                        print msg;

class SendThread(threading.Thread):
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
        #self.taskQueue = Queue.Queue()

        #self.recvThread = RecvThread(self.soc, self.taskQueue)
        self.sendThread = SendThread(self.soc)
        time.clock()

        print "Windows Perf Timer :1230"
        #self.recvThread.start()
        self.sendThread.start()

    def stop(self):
        #self.recvThread.work = False
        self.sendThread.work = False
        
        #self.recvThread.join()
        self.sendThread.join()
        self.soc.close()