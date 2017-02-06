import socket

class device():
    def __init__(self,name,addr,connection):
        self.hostname = name;
        self.ip = addr
        self.connection = connection


def discoverDevices(subnet):
    subnet = "192.168."+subnet+"."
    devices = []
    for i in range(100,250):
        ip = subnet+str(i)
        print ip
        try:
            connection = socket.socket()
            connection.connect((ip, 8000))
            devices.append(device(connection.recv(3),ip,connection))
        except:
            pass
    return devices