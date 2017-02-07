from socket import *
import select
import time

class device():
    def __init__(self,addr):
        self.hostname = "";
        self.status = "";
        self.ip = addr
        self.tcpConnection = socket(AF_INET, SOCK_STREAM);

class deviceManager():
    def __init__(self):
        self.deviceList = []

    def discoverDevices(self):
        udpSocket=socket(AF_INET, SOCK_DGRAM)
        udpSocket.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
        udpSocket.sendto('Remote',('255.255.255.255',8001))
    
        while True:
            try:
                udpSocket.settimeout(1)
                data, address = udpSocket.recvfrom(32)
                if str(data) == "PiMatrix":
                    self.deviceList.append(device(address[0]))
            except timeout:
                break
    
        for pimatrix in self.deviceList:
            pimatrix.tcpConnection.connect((pimatrix.ip, 8000))
            data = pimatrix.tcpConnection.recv(21)
        
            pimatrix.hostname=str(data[1:]).rstrip(" \t\r\n\0")
            pimatrix.status=str(data[0])

    def tabulateDevice(self):
        print "Hostname\tIP\t\tStatus"
        print "--------\t--\t\t------"
        for pimatrix in self.deviceList:
            status = ""
            if pimatrix.status == "I":
                status = "Idle"
            elif pimatrix.status == "R":
                status = "Recording"
            elif pimatrix.status == "L":
                status = "LVCSR"

            print pimatrix.hostname+"\t"+pimatrix.ip+"\t"+status

    def sendCommand(self, command):
        for pimatrix in self.deviceList:
            pimatrix.tcpConnection.send(command)