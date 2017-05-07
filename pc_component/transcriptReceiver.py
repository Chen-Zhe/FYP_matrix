import socket
import threading
import os
import time

class TranscriptReceiver(threading.Thread):
    def __init__(self, device):
        threading.Thread.__init__(self)
        self.keep_alive = True
        self.device = device

    def run(self):
        self.device.tcpConnection.send("S")
        utterance = 1
        currentDateAndTime = time.strftime("%Y%m%d_%H%M%S", time.localtime())
        fileName = "Recordings\\"+ currentDateAndTime +"_transcript.txt"
        text_file = open(fileName, "w")
        text_file.write("------Final Transcript------\n\n")

        self.device.tcpConnection.settimeout(1)
        while self.keep_alive:            
            try:
                buffer = self.device.tcpConnection.recv(1024)
                
                if len(buffer) == 0:
                    break
            except socket.timeout:
                continue

            results = str(buffer).rstrip().split("|")

            for res in results:
                if len(res) < 2:
                    continue

                if res[0] == '1':                
                    print "\r"+str(utterance)+". "+res[1:]+"\n"
                    text_file.write(str(utterance)+". "+res[1:]+"\n\n")
                    utterance+=1
                else:
                    if res[0] == '0':  
                        print "\r"+res[1:],
        
        text_file.close()
        self.device.tcpConnection.settimeout(None)
        os.system(fileName)
