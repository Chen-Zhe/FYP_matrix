import threading
import recordingStream
import pimatrix
import time
from ntpserver import ntpServer

deviceMan = pimatrix.deviceManager()

def printMenu():
    print "\n"
    print "Welcome to Pi-Matrix Management Console"
    print "Device connected:", deviceMan.numDevices, "\n"
    print "1. Discover devices"
    if deviceMan.numDevices > 0:
        print "2. Connected devices' detail"
        print "----------------------------"
        if not deviceMan.deviceBusy:
            print "3. Record over network"
            print "4. Record to disk"
            print "5. Use device as LVCSR"
        else:
            print "8. Stop all devices' current task"
        print "----------------------"
        print "9. Disconnect from all devices"
        print "0. Shutdown all devices"

ntp = ntpServer()

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
        digit = chr(((ord(time.strftime("%S", time.localtime())[1])-48)+3)%10+48)
        currentDateAndTime = time.strftime("%Y%m%d_%H%M%S", time.localtime())

        streamerList = [recordingStream.RecordingStream(device, currentDateAndTime) for device in deviceMan.deviceList]
        for streamer in streamerList:
            streamer.start()
        deviceMan.sendCommand("rec2net", digit)
                
        raw_input("Press Enter to stop recording...")
        for streamer in streamerList:
            streamer.continue_recording = False
        
        deviceMan.sendCommand("stop")
        print("stopping......")
        time.sleep(1)

    elif choice == 4:
        digit = chr(((ord(time.strftime("%S", time.localtime())[1])-48)+3)%10+48)
        deviceMan.sendCommand("rec2sd", digit)

    elif choice == 5:
        pass
    
    elif choice == 8:
        deviceMan.sendCommand("stop")

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

ntp.stop()
print "Thank you for using Pi-Matrix Management Console"
print "Have a nice day!"