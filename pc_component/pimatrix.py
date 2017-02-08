from socket import *

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

    def discoverDevices(self):
        udpSocket=socket(AF_INET, SOCK_DGRAM)
        udpSocket.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
        udpSocket.sendto('live long and prosper',('255.255.255.255',8001))
    
        while True:
            try:
                udpSocket.settimeout(1)
                data, (ip, port) = udpSocket.recvfrom(32)
                if str(data) == "peace and long life":
                    self.deviceList.append(device(ip))
            except timeout:
                break
    
        for pimatrix in self.deviceList:
            pimatrix.tcpConnection.connect((pimatrix.ip, 8000))
            data = pimatrix.tcpConnection.recv(21)
        
            pimatrix.hostname=str(data[1:]).rstrip(" \t\r\n\0")
            pimatrix.status=str(data[0])
        
        self.numDevices = len(self.deviceList)

    def tabulateDevice(self):
        if self.numDevices>0:
            print "\nHostname\tIP\t\tStatus"
            print "--------\t--\t\t------"
            for pimatrix in self.deviceList:
                status = ""
                if pimatrix.status == "I":
                    status = "Idle"
                elif pimatrix.status == "L":
                    status = "Recording"
                elif pimatrix.status == "S":
                    status = "LVCSR"

                print pimatrix.hostname+"\t"+pimatrix.ip+"\t"+status
        else:
            print "No devices found"

    def sendCommand(self, command):
        commandKeyword = {
            "shutdown": "T",
            "rec2net": "N",
            "rec2sd": "L",
            "lvcsr": "S"
            }
        for pimatrix in self.deviceList:
            try:
                pimatrix.tcpConnection.send(commandKeyword[command])
                pimatrix.status = commandKeyword[command]
            except:
                print "{0}({1}) timed out!".format(pimatrix.hostname,pimatrix.ip)
                self.deviceList.remove(pimatrix)
                self.numDevices -= 1
    
    def disconnectAll(self):
        for pimatrix in self.deviceList:
            pimatrix.tcpConnection.close()
        self.deviceList = []
        self.numDevices = 0