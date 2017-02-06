import socket
import threading
import recordingStream
import pimatrix

devices=[]

print "Welcome to Pi-Matrix Management Console"
print "Please select one of the following functionalities"
print "1. Discover Devices"
print "2. Record over network"
print "0. Disconnect from all devices"

choice = input("Choice: ");
if choice == 1:
    subnet = raw_input("Please complete the current subnet address 192.168.(0~255): ")
    devices = pimatrix.discoverDevices(subnet)
    print len(devices)
