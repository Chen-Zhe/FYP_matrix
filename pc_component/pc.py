import socket
import datetime
import wave
import threading

class RecordStream(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.seconds_recorded = 0
        self.continue_recording = False

    def run(self):
        out_sound = wave.open('test.wav', 'wb')
        # (num of channels, sampling width in bytes, sampling rate, num of frames, compression type, compression name)
        out_sound.setparams((8, 2, 16000, 0, 'NONE', 'not compressed'))

        connection = socket.socket()
        connection.connect(("192.168.1.100", 8000))

        print "connection established, recording..."

        expected_bytes = 16000 * 2 * 8 # 1 second
        bytes_received = 0

        while bytes_received<expected_bytes:
            file_buffer = connection.recv(4096) # it will not always receive 4096 bytes

            file_buffer_length = len(file_buffer)
            bytes_received += file_buffer_length

            if bytes_received>=expected_bytes:
                self.seconds_recorded +=1
                print self.seconds_recorded, "seconds recorded\r",
                if self.continue_recording:
                    bytes_received = bytes_received-expected_bytes                    
                else:
                    file_buffer = file_buffer[0:file_buffer_length - (bytes_received-expected_bytes)]
    
            out_sound.writeframes(file_buffer)

        connection.close
        out_sound.close()

        print str(self.seconds_recorded) + " second audio file saved"


raw_input("Press Enter to start recording...")

thread = RecordStream()
thread.continue_recording = True
thread.start()
raw_input("Press Enter again to stop recording...\n")
thread.continue_recording = False
thread.join()
