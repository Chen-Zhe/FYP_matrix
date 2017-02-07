import socket
import threading
import recordingStream
import pimatrix

deviceMan = pimatrix.deviceManager()

print "Welcome to Pi-Matrix Management Console"
print "Please select one of the following functionalities"
print "1. Discover devices"
print "2. Record over network"
print "3. Reocrd to disk"
print "4. Use device as LVCSR"
print "5. Shutdown all devices"
print "0. Disconnect from all devices"

while(True):
    choice = input("Choice: ");
    if choice == 1:
        print "Please wait...Scanning..."
        deviceMan.discoverDevices()
        deviceMan.tabulateDevice()

    elif choice == 5:
        if raw_input("Are you sure? (y/n)") == "y":
            deviceMan.sendCommand("T")
            break