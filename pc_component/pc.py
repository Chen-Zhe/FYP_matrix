import socket
import datetime
import wave

#record a single audio file of 8 channels
seconds_to_record = 10

out_sound = wave.open('test.wav', 'wb')
# (num of channels, sampling width in bytes, sampling rate, num of frames, compression type, compression name)
out_sound.setparams((8, 2, 16000, 0, 'NONE', 'not compressed'))

connection = socket.socket()
connection.connect(("192.168.1.14", 8000))

print "connection established, recording..."

expected_bytes = 16000*2*8 * seconds_to_record
bytes_received = 0

while bytes_received<expected_bytes:
    file_buffer = connection.recv(4096) #will not always receive 4096 bytes

    file_buffer_length = len(file_buffer)
    bytes_received += file_buffer_length

    if bytes_received>=expected_bytes:
        file_buffer = file_buffer[0:file_buffer_length - (bytes_received-expected_bytes)]
    
    out_sound.writeframes(file_buffer)

connection.close
out_sound.close()

print str(seconds_to_record) + " second audio file received"

