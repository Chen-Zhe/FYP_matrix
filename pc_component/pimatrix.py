from socket import *
import time
import struct

class device():
    def __init__(self,addr):
        self.hostname = "";
        self.status = "";
        self.ip = addr
        self.tcpConnection = socket(AF_INET, SOCK_STREAM);

class deviceManager():
    def __init__(self):
        self.deviceList = []
        self.numDevices = 0
        self.deviceBusy = False

    def discoverDevices(self):
        newDevices = []

        udpSocket=socket(AF_INET, SOCK_DGRAM)
        udpSocket.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
        udpSocket.sendto('live long and prosper',('255.255.255.255',8001))
    
        while True:
            try:
                udpSocket.settimeout(1)
                data, (ip, port) = udpSocket.recvfrom(32)
                if str(data) == "peace and long life":
                    newDevices.append(device(ip))
            except timeout:
                break
    
        for pimatrix in newDevices:
            pimatrix.tcpConnection.connect((pimatrix.ip, 8000))
            data = pimatrix.tcpConnection.recv(21)
            
            pimatrix.tcpConnection.send(struct.pack("I", int(time.time())))

            pimatrix.hostname=str(data[1:]).rstrip(" \t\r\n\0")
            pimatrix.status=str(data[0])
            if not pimatrix.status == 'I':
                self.deviceBusy = True
        
        self.deviceList.extend(newDevices)
        self.numDevices += len(newDevices)

    def tabulateDevice(self):
        if self.numDevices>0:
            num = 1
            print "\n#\tHostname\tIP\t\tStatus"
            print "-\t--------\t--\t\t------"
            for pimatrix in self.deviceList:
                status = ""
                if pimatrix.status == "I":
                    status = "Idle"
                elif pimatrix.status == "L":
                    status = "Recording"
                elif pimatrix.status == "S":
                    status = "LVCSR"

                print str(num)+".\t"+pimatrix.hostname+"\t"+pimatrix.ip+"\t"+status
                num+=1
        else:
            print "No devices found"
        

    def sendCommand(self, command, para=''):
        commandKeyword = {
            "shutdown": "T",
            "rec2net": "N",
            "rec2sd": "L",
            "lvcsr":"S",
            "stop": "I"
            }
        
        status = commandKeyword[command]
        self.deviceBusy = (not status == "I")
        
        for pimatrix in self.deviceList:
            try:
                pimatrix.tcpConnection.send(commandKeyword[command]+para)
                pimatrix.status = status

            except:
                print "{0}({1}) timed out!".format(pimatrix.hostname,pimatrix.ip)
                self.deviceList.remove(pimatrix)
                self.numDevices -= 1
    
    def disconnectAll(self):
        for pimatrix in self.deviceList:
            pimatrix.tcpConnection.close()
        self.deviceList = []
        self.numDevices = 0

    def cleanTcpBuffer(self):
        for pimatrix in self.deviceList:
            pimatrix.tcpConnection.setblocking(False)
            while True:
                try:                
                    pimatrix.tcpConnection.recv(4096)
                except:
                    pimatrix.tcpConnection.setblocking(True)
                    break