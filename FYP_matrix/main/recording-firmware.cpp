#include <unistd.h>
#include <utility>
#include <memory>
#include <pthread.h>
#include <string>
#include <iostream>
#include <valarray>
#include <sstream>
#include <fstream>
#include <wiringPi.h>
#include <mqueue.h>
#include <fcntl.h>

#include <libsocket/inetserverstream.hpp>
#include <libsocket/inetserverdgram.hpp>
#include <libsocket/inetclientdgram.hpp>
#include <libsocket/exception.hpp>

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

#include "sharedResources.h"

#define BUFFER_SAMPLES_PER_CHANNEL	16000 //1 second of recording
#define STREAMING_CHANNELS			8 //Maxmium 8 channels
const int32_t bufferByteSize = STREAMING_CHANNELS * BUFFER_SAMPLES_PER_CHANNEL * sizeof(int16_t);

#define HOST_NAME_LENGTH			20 //maximum number of characters for host name
#define COMMAND_LENGTH				1

void *record2Remote(void* null);
void *record2Disk(void* null);

int32_t syncTime();

void irInterrupt();
void * motionDetection(void * null);
void *udpBroadcastReceiver(void *null);
void * recorder(void * null);

char sysInfo[COMMAND_LENGTH + HOST_NAME_LENGTH];

bool pcConnected = false;
bool recording = false;
//double buffer of SAMPLES_PER_CHANNEL*8 samples each
int16_t buffer[2][BUFFER_SAMPLES_PER_CHANNEL][STREAMING_CHANNELS];

pthread_mutex_t bufferMutex[2] = { PTHREAD_MUTEX_INITIALIZER };

char* status = &sysInfo[0];
char* hostname = &sysInfo[1];

char commandArgument;

std::unique_ptr<libsocket::inet_stream> tcpConnection;
LedController *LedCon;
matrixCreator::MicrophoneArray microphoneArray;

int main(int argc, char *argv[]) {

	gethostname(hostname, HOST_NAME_LENGTH);
	*status = 'I';
	matrixCreator::WishboneBus bus;
	bus.SpiInit();

	microphoneArray.Setup(&bus);

	LedCon = new LedController(&bus);
	char command = '\0';

	pthread_t udpThread;
	pthread_create(&udpThread, NULL, udpBroadcastReceiver, NULL);

	if (argc > 1) {
		char channel = argv[1][0];
		if ('0' <= channel && channel <= '8')
			GoogleSpeech::channelToSend = channel - 48;
	}

	if (argc <= 2) {
		pthread_t motionDetect;
		pthread_create(&motionDetect, NULL, motionDetection, NULL);
	}

	GoogleSpeech::setup();

	//stand by
	libsocket::inet_stream_server tcpServer("0.0.0.0", "8000", LIBSOCKET_IPv4);
	cout << hostname << " - TCP server listening :8000\n";

	for (matrixCreator::LedValue& led : LedCon->Image.leds) {
		led.red = 0; led.green = 0; led.blue = 2;
	}
	LedCon->updateLed();

	//wait for network connection
	while(true){
		try {

			tcpConnection = tcpServer.accept2();

			//connected
			for (matrixCreator::LedValue& led : LedCon->Image.leds) {
				led.red = 0; led.green = 2; led.blue = 0;
			}
			LedCon->updateLed();
			pcConnected = true;

			tcpConnection->snd(sysInfo, COMMAND_LENGTH + HOST_NAME_LENGTH);
			syncTime();

			pthread_t recorderThread;

			while (tcpConnection->rcv(&command, 1, MSG_WAITALL)) {
				switch (command) {
				case 'N': {//record to network
					if (*status == 'I') {
						*status = 'N';
						tcpConnection->rcv(&commandArgument, 1, MSG_WAITALL);
						pthread_create(&recorderThread, NULL, recorder, NULL);
						break;
					}
				}
				case 'L': {//record to disk
					if (*status == 'I') {
						*status = 'L';
						tcpConnection->rcv(&commandArgument, 1, MSG_WAITALL);
						pthread_create(&recorderThread, NULL, recorder, NULL);
					}

					break;
				}

				case 'S': {//google speech
					if (*status == 'I') {
						*status = 'S';						
						pthread_create(&recorderThread, NULL, GoogleSpeech::run, NULL);
					}

					break;
				}

				case 'I': { //stop everything

					switch (*status) {
						case 'I': break;
						case 'L': {
							recording = false;
							pthread_join(recorderThread, NULL);
							break;
						}
						case 'N': {
							recording = false;
							pthread_join(recorderThread, NULL);
							break;
						}
						case 'S': {
							GoogleSpeech::stop();
							pthread_join(recorderThread, NULL);
							break;
						}
					}

					*status = 'I';
					break;
				}

				case 'T': {
					LedCon->turnOffLed(); system("sudo shutdown now");
					break;
				}
				default: cout << "unrecognized command" << endl;
				}
				command = '\0';
				LedCon->turnOffLed();
			}
		}
	
		catch (const libsocket::socket_exception& exc)
		{
			//error
			cout << exc.mesg << endl;
		}
		pcConnected = false;
		cout << "Remote PC at " << tcpConnection->gethost() << ":" << tcpConnection->getport() << " disconnected" << endl;
		tcpConnection->destroy();
		LedCon->turnOffLed();		
	}

	LedCon->updateLed();
	sleep(1);
	LedCon->turnOffLed();

	return 0;
}

inline int32_t syncTime() {
	uint32_t currentEpochTime;
	tcpConnection->rcv(&currentEpochTime, 4);
	char command[25];
	sprintf(command, "sudo date -s @%d", currentEpochTime);
	cout << "Sys time synced with remote PC: " << flush;;
	return system(command);
}

int irPulseCount = 0;

void irInterrupt() {
	irPulseCount++;
}

void *motionDetection(void *null) {
	pthread_t recorderThread;
	
	//setup
	//system("gpio edge 16 both");
	pinMode(16, INPUT);
	pinMode(13, OUTPUT);
	pinMode(5, OUTPUT);

	digitalWrite(13, HIGH);
	digitalWrite(5, HIGH);

	sleep(1);
	//setup pin 16 (IR) as interrupt
	cout << "Motion detection enabled" << endl;
	wiringPiISR(16, INT_EDGE_FALLING, &irInterrupt);

	while (true) {
		if (!pcConnected && irPulseCount > 30 && irPulseCount < 90) {
			if (*status == 'I') {
				LedCon->turnOffLed();
				*status = 'L';
				pthread_create(&recorderThread, NULL, recorder, NULL);
			}
			else if (*status == 'L') {
				*status = 'I';
				recording = false;
				pthread_join(recorderThread, NULL);
				LedCon->updateLed();
			}
		}
		irPulseCount = 0;
		sleep(1);		
	}

}

void *udpBroadcastReceiver(void *null) {
	string remoteIP;
	string remotePort;
	string buffer;

	remoteIP.resize(16);
	remotePort.resize(16);
	buffer.resize(32);
	
	//start server
	libsocket::inet_dgram_server udpServer("0.0.0.0", "8001", LIBSOCKET_IPv4);
	cout << hostname << " - UDP server listening :8001\n";

	while (true) {
		try {
			udpServer.rcvfrom(buffer, remoteIP, remotePort);
			
			if (!pcConnected && buffer.compare("live long and prosper") == 0) {
				cout << "Remote PC at " << remoteIP << endl;
				udpServer.sndto("peace and long life", remoteIP, remotePort);
			}
			
		}
		catch (const libsocket::socket_exception& exc)
		{
			//error
			for (matrixCreator::LedValue& led : LedCon->Image.leds) {
				led.red = 8; led.green = 0; led.blue = 0;
			}
			LedCon->updateLed();

			cout << exc.mesg << endl;
		}
	}
	
}

inline double syncRecording(char expectedSecondLSD) {
	struct syncPacket {
		uint32_t rxTimeInt;
		uint32_t rxTimeFrac;
	} packet;
	string ip;
	string port;
	libsocket::inet_dgram_client udp(LIBSOCKET_IPv4);
	udp.sndto("N", tcpConnection->gethost(), "1230");
	udp.rcvfrom((void*)&packet, 8, ip, port);

	int32_t secondDiff = expectedSecondLSD - 48 - packet.rxTimeInt % 10;
	if (secondDiff < 0) secondDiff += 10;

	return secondDiff * 1000000 - packet.rxTimeFrac;
}

void *recorder(void* null) {	
	int32_t buffer_switch = 0;
	int32_t writeInitDiscard = 0;

	recording = true;
	//lock down buffer 0 before spawning streaming thread
	pthread_mutex_lock(&bufferMutex[buffer_switch]);

	pthread_t workerThread;
	switch (*status) {
	case 'N':pthread_create(&workerThread, NULL, record2Remote, NULL); break;
	case 'L':pthread_create(&workerThread, NULL, record2Disk, NULL); break;
	}

	if (pcConnected) {
		int32_t samplesToWait;

		microphoneArray.Read();
		samplesToWait = syncRecording(commandArgument)*0.016 - 128;
		while (samplesToWait > 128) {
			microphoneArray.Read();
			samplesToWait -= 128;
		}
		//one more read to go
		writeInitDiscard = samplesToWait;		
	}
	cout << "------ Recording starting ------" << endl;

	microphoneArray.Read();
	while (recording) {
		int32_t step = 0;
		bool bufferFull = false;

		//fill the first partial buffer
		for (int32_t s = writeInitDiscard; s < 128; s++) {
			for (int32_t c = 0; c < STREAMING_CHANNELS; c++) {
				buffer[buffer_switch][step][c] = microphoneArray.At(s, c);
			}
			step++;
		}

		while (!bufferFull) {
			int32_t s = 0;
			
			microphoneArray.Read(); //The reading process is a blocking process that read in 8*128 samples every 8ms

			for (s = 0; s < 128; s++) {
				for (int32_t c = 0; c < STREAMING_CHANNELS; c++) {
					buffer[buffer_switch][step][c] = microphoneArray.At(s, c);
				}
				step++;
				if (step == BUFFER_SAMPLES_PER_CHANNEL) {
					bufferFull = true;
					break;
				}
			}
		}
		pthread_mutex_lock(&bufferMutex[(buffer_switch + 1) % 2]);
		pthread_mutex_unlock(&bufferMutex[buffer_switch]);
		buffer_switch = (buffer_switch + 1) % 2;
	}

	pthread_mutex_unlock(&bufferMutex[buffer_switch]);
	pthread_join(workerThread, NULL);
	cout << "------ Recording ended ------" << endl;
	pthread_exit(NULL);
}

void *record2Disk(void* null) {
	uint32_t bufferSwitch = 0;
	char dateAndTime[16];
	struct tm tm;
	matrixCreator::EverloopImage rotatingRing;

	do {//fix the file size to less than 2GB maximum, create new file when recording continues
		rotatingRing.leds[0].red = 5;
		LedCon->updateLed(rotatingRing);

		uint32_t counter = 0;

		time_t t = time(NULL);
		tm = *localtime(&t);
				
		sprintf(dateAndTime, "%d%02d%02d_%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);

		ostringstream filenameStream;
		filenameStream << "/home/pi/Recordings/" << hostname << "_" << dateAndTime << "_8ch.wav";
		string filename = filenameStream.str();

		ofstream file(filename, std::ofstream::binary);

		// WAVE file header format
		struct WaveHeader {
			//RIFF chunk
			char RIFF[4] = { 'R', 'I', 'F', 'F' };
			uint32_t overallSize;						// overall size of file in bytes
			char WAVE[4] = { 'W', 'A', 'V', 'E' };		// WAVE string

														//fmt subchunk
			char fmt[4] = { 'f', 'm', 't', ' ' };		// fmt string with trailing null char
			uint32_t fmtLength = 16;					// length of the format data
			uint16_t audioFormat = 1;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
			uint16_t numChannels = 8;					// no.of channels
			uint32_t samplingRate = 16000;				// sampling rate (blocks per second)
			uint32_t byteRate = 256000;					// SampleRate * NumChannels * BitsPerSample/8
			uint16_t blockAlign = 16;					// NumChannels * BitsPerSample/8
			uint16_t bitsPerSample = 16;				// bits per sample, 8- 8bits, 16- 16 bits etc

														//data subchunk
			char data[4] = { 'd', 'a', 't', 'a' };		// DATA string or FLLR string
			uint32_t dataSize;							// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
		} header;

		file.write((const char*)&header, sizeof(WaveHeader));

		while (recording && counter < 8191) {

			pthread_mutex_lock(&bufferMutex[bufferSwitch]);

			file.write((const char*)buffer[bufferSwitch], bufferByteSize);
			
			pthread_mutex_unlock(&bufferMutex[bufferSwitch]);
			bufferSwitch = (bufferSwitch + 1) % 2;

			rotatingRing.leds[counter%matrixCreator::kMatrixCreatorNLeds].red = 0;
			counter++;
			rotatingRing.leds[counter%matrixCreator::kMatrixCreatorNLeds].red = 5;
			LedCon->updateLed(rotatingRing);
		}

		header.dataSize = bufferByteSize * counter;
		header.overallSize = header.dataSize + 36;
		file.seekp(0);
		file.write((const char*)&header, sizeof(WaveHeader));
		file.close();

	} while (recording);

	pthread_exit(NULL);
}

void *record2Remote(void* null)
{
	uint32_t bufferSwitch = 0;
	matrixCreator::EverloopImage rotatingRing;

	rotatingRing.leds[17].red = 5;
	LedCon->updateLed(rotatingRing);

	while (recording) {
		pthread_mutex_lock(&bufferMutex[bufferSwitch]);

		try {
			tcpConnection->snd(buffer[bufferSwitch], bufferByteSize);
			//cout << "sending" << endl;
		}
		catch (const libsocket::socket_exception& exc)
		{
			//network disconnection means recording completed
			recording = false; //set flag
			*status = 'I';
			pthread_mutex_unlock(&bufferMutex[bufferSwitch]);//unlock mutex
			break;
		}

		
		pthread_mutex_unlock(&bufferMutex[bufferSwitch]);
		//cout << "Buffer " << bufferSwitch << " Sent" << endl;
		bufferSwitch = (bufferSwitch + 1) % 2;

		if (bufferSwitch) {
			rotatingRing.leds[0].red = 5;
			rotatingRing.leds[17].red = 0;
		}
		else {
			rotatingRing.leds[17].red = 5;
			rotatingRing.leds[0].red = 0;
		}
		LedCon->updateLed(rotatingRing);
	}
		
	pthread_exit(NULL);//terminate itself
}
