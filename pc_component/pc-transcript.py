import socket
import datetime
import threading

class DisplayTranscript(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.keep_alive = False

    def run(self):
        connection = socket.socket()
        connection.connect(("192.168.0.101", 8000))

        print "connection established, printing transcript"

        while self.keep_alive:
            buffer = connection.recv(128) # it will not always receive the specified bytes
            print buffer

        connection.close


raw_input("Press Enter to connect...")

thread = DisplayTranscript()
thread.keep_alive = True
thread.start()
raw_input("Press Enter again to disconnect...\n")
thread.keep_alive = False
thread.join()
