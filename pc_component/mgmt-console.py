import threading
import audioStreamReceiver
import pimatrix
import time
import transcriptReceiver
from timeServer import TimeServer

deviceMan = pimatrix.deviceManager()
winPerfTimer = TimeServer()

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
            print "5. LVCSR (Single Device)"
        else:
            print "8. Stop all devices' current task"
        print "----------------------"
        print "9. Disconnect from all devices"
        print "0. Shutdown all devices"

while(True):
    printMenu()
    rawChoice = raw_input("Choice: ");
    if len(rawChoice) == 0:
        choice = 1
    else:
        choice = int(rawChoice)

    if choice == 1:
        print "Scanning......"
        deviceMan.discoverDevices()
        deviceMan.tabulateDevice()
        raw_input("Press Enter to continue...")     
    
    elif choice == 2:
        deviceMan.tabulateDevice()
        raw_input("Press Enter to continue...")

    elif choice == 3:
        
        currentDateAndTime = time.strftime("%Y%m%d_%H%M%S", time.localtime())

        streamerList = [audioStreamReceiver.RecordingStream(device, currentDateAndTime) for device in deviceMan.deviceList]
        for streamer in streamerList:
            streamer.start()
        digit = chr((int(time.clock())+2)%10+48)
        deviceMan.sendCommand("rec2net", digit)

        raw_input("Press Enter to stop recording...")
        for streamer in streamerList:
            streamer.continue_recording = False

        deviceMan.sendCommand("stop")
        print "Stopping......"
        for streamer in streamerList:
            streamer.join()

    elif choice == 4:
        digit = chr((int(time.clock())+2)%10+48)
        deviceMan.sendCommand("rec2sd", digit)

    elif choice == 5:
        if deviceMan.numDevices == 1:
            sel = 1
        else:
            deviceMan.tabulateDevice()
            sel = input("Select one device to do LVCSR (1-"+ str(deviceMan.numDevices) +"): ")
            if sel<1 or sel>deviceMan.numDevices:
                raw_input("Invalid choice, press Enter to continue...")
                continue
            
        transcriber = transcriptReceiver.TranscriptReceiver(deviceMan.deviceList[sel-1])
        transcriber.start()

        raw_input("Press Enter to stop transcribing...\n\n")
        transcriber.keep_alive = False
        deviceMan.sendCommand("stop")
        print "Stopping..."
        transcriber.join()
    
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

print "Thank you for using Pi-Matrix Management Console"
print "Have a nice day!"
winPerfTimer.stop()