import socket
import datetime
s = socket.socket()
host = "192.168.43.157"
port = 8002

f = open('test.wav','wb')
start = datetime.datetime.utcnow()
s.connect((host, port))

while True:
    file_buffer = s.recv(4096)

    if len(file_buffer) == 0:
        break
    f.write(file_buffer)
end = datetime.datetime.utcnow()
print "finished in " + str(end-start)
s.close       # Close the connection
