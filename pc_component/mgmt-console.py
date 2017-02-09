import threading
import recordingStream
import pimatrix
import time

deviceMan = pimatrix.deviceManager()


def printMenu():
    print "\n"
    print "Welcome to Pi-Matrix Management Console"
    print "Device connected:", deviceMan.numDevices, "\n"
    print "1. Discover devices"
    if deviceMan.numDevices > 0:
        print "2. Connected devices' detail"
        print "----------------------------"
        print "3. Record over network"
        print "4. Record to disk"
        print "5. Use device as LVCSR"
        print "----------------------"
        print "9. Disconnect from all devices"
        print "0. Shutdown all devices"

while(True):
    printMenu()
    choice = input("Choice: ");
    if choice == 1:
        print "Scanning......"
        deviceMan.discoverDevices()
        deviceMan.tabulateDevice()
    
    elif choice == 2:
        deviceMan.tabulateDevice()

    elif choice == 3:
        currentDateAndTime = time.strftime("%Y%m%d_%H%M%S", time.localtime())
        streamerList = [recordingStream.RecordingStream(device, currentDateAndTime) for device in deviceMan.deviceList]
        deviceMan.sendCommand("rec2net")
        for streamer in streamerList:
            streamer.start()
        raw_input("Press Enter to stop recording...\n")
        for streamer in streamerList:
            streamer.continue_recording = False

    elif choice == 4:
        deviceMan.sendCommand("rec2sd")

    elif choice == 5:
        pass

    elif choice == 9:
        deviceMan.disconnectAll()
        #break
    
    elif choice == 0:
        if raw_input("Shutdown all devices? (y/n)") == "y":
            deviceMan.sendCommand("shutdown")
            break
        else:
            print "Abort"
    else:
        print "Not a valid choice"

print "Thank you for using Pi-Matrix Management Console"
print "Have a nice day!"