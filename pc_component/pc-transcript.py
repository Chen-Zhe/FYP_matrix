import socket
import datetime
import threading
from ntpserver import ntpServer

class DisplayTranscript(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.keep_alive = False
        self.connection = socket.socket()

    def run(self):
        utterance = 1
        
        self.connection.connect(("192.168.0.101", 8000))

        print "connection established, printing transcript\n"

        while self.keep_alive:
            buffer = self.connection.recv(128) # it will not always receive the specified bytes

            if len(buffer) == 0:
                break

            results = str(buffer).rstrip().split("|")

            for res in results:
                if len(res) < 2:
                    continue

                if res[0] == '1':                
                    print "\r", str(utterance)+".", res[1:], "\n"
                    utterance+=1
                else:
                    print "\r", res[1:],
        
        self.connection.close()

ntp = ntpServer()

raw_input("Press Enter to connect...")

thread = DisplayTranscript()
thread.keep_alive = True
thread.start()
raw_input("Press Enter again to disconnect...\n")
thread.connection.send("S")
thread.keep_alive = False
ntp.stop()
thread.join()
