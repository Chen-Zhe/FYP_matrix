import wave
import threading

class RecordingStream(threading.Thread):
    def __init__(self, device, dateAndTime):
        threading.Thread.__init__(self)
        self.seconds_recorded = 0
        self.continue_recording = True
        self.device = device
        self.dateAndTime = dateAndTime

    def run(self):
        printout = "Receiving from "+self.device.hostname+"...\n"
        print printout
        filename = "Recordings/"+self.device.hostname+"_"+self.dateAndTime+"_8ch.wav"
        out_sound = wave.open(filename, 'wb')
        # (num of channels, sampling width in bytes, sampling rate, num of frames, compression type, compression name)
        out_sound.setparams((8, 2, 16000, 0, 'NONE', 'not compressed'))

        expected_bytes = 16000 * 2 * 8 # 1 second
        bytes_received = 0

        while bytes_received<expected_bytes:
            file_buffer = self.device.tcpConnection.recv(4096) # it will not always receive the specified bytes

            file_buffer_length = len(file_buffer)

            if file_buffer_length == 0:
                print self.device.hostname, "connection closed"
                break

            bytes_received += file_buffer_length

            if bytes_received>=expected_bytes:
                self.seconds_recorded +=1
                #print self.seconds_recorded, "seconds recorded\r",
                if self.continue_recording:
                    bytes_received = bytes_received-expected_bytes                    
                else:
                    file_buffer = file_buffer[0:file_buffer_length - (bytes_received-expected_bytes)]
    
            out_sound.writeframes(file_buffer)

        out_sound.close()

        printout = self.device.hostname+" streamed for "+str(self.seconds_recorded//60)+" minutes "+str(self.seconds_recorded%60)+" seconds\n"

        print printout