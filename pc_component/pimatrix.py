from socket import *

class device():
    def __init__(self,name,addr,connection):
        self.hostname = name;
        self.ip = addr
        self.connection = connection


def discoverDevices(subnet):
    s=socket(AF_INET, SOCK_DGRAM)
    s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
    s.sendto('Remote',('255.255.255.255',8001))

    #return devices