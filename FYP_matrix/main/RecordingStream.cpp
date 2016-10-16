#include <unistd.h>
#include <stdio.h>
#include <utility>
#include <memory>
#include <pthread.h>
#include <string>
#include <iostream>
#include <valarray>

#include "../libsocket/inetserverstream.hpp"
#include "../libsocket/exception.hpp"
#include "../libsocket/socket.hpp"
#include "../libsocket/select.hpp"

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/everloop.h"
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

#define BUFFER_SAMPLES_PER_CHANNEL	16384 //around 1 second, power of 2 for networking, trucation at PC side
#define HOST_NAME_MAX				20 //maximum number of characters for host name
#define STREAMING_CHANNELS			8 //Maxmium 8 channels

void *networkStream(void *null);

namespace matrixCreator = matrix_hal;

struct LED {
	matrixCreator::Everloop Driver;
	matrixCreator::EverloopImage Image;
};

struct LED ledRing; // Red = error, Blue = ready, Green = complete
char hostname[HOST_NAME_MAX];
bool networkConnected = false;

//double buffer of SAMPLES_PER_CHANNEL*8 samples each
int16_t buffer[2][BUFFER_SAMPLES_PER_CHANNEL][STREAMING_CHANNELS];

pthread_mutex_t bufferMutex[2] = { PTHREAD_MUTEX_INITIALIZER };

int main() {
	gethostname(hostname, HOST_NAME_MAX);

	matrixCreator::WishboneBus bus;
	bus.SpiInit();

	matrixCreator::MicrophoneArray microphoneArray;
	
	ledRing.Driver.Setup(&bus);
	
	//wait for network connection
	std::unique_ptr<libsocket::inet_stream> tcpConnection;
	try {
		libsocket::inet_stream_server tcpServer("0.0.0.0", "8000", LIBSOCKET_IPv4);

		//signal the user that the server is ready
		std::cout << hostname << " - TCP server ready" << std::endl;
		for (auto& led : ledRing.Image.leds) { led.red = 0; led.green = 0; led.blue = 8; }
		ledRing.Driver.Write(&ledRing.Image);
		
		tcpConnection = tcpServer.accept2();
	}
	catch (const libsocket::socket_exception& exc)
	{
		for (auto& led : ledRing.Image.leds) { led.red = 8; led.green = 0; led.blue = 0; }
		ledRing.Driver.Write(&ledRing.Image);

		std::cout << exc.mesg << std::endl;
		return 0;
	}
	
	networkConnected = true;

	uint32_t buffer_switch = 0;
	//lock down buffer 0 before spawning streaming thread
	pthread_mutex_lock(&bufferMutex[buffer_switch]);

	pthread_t networkStreamingThread;//spawn networking thread and pass the connection
	pthread_create(&networkStreamingThread, NULL, networkStream, (void *)&tcpConnection);

	for (auto& led : ledRing.Image.leds) { led.red = 0; led.green = 0; led.blue = 0; }
	ledRing.Driver.Write(&ledRing.Image);

	ledRing.Driver.Write(&ledRing.Image);
	microphoneArray.Setup(&bus);

	while (networkConnected){

		uint32_t step = 0;
		while (step < BUFFER_SAMPLES_PER_CHANNEL) {

			/* Reading 8-mics buffer from the FPGA
			The reading process is a blocking process that read in 8*128 samples every 8ms
			*/
			microphoneArray.Read();

			for (uint16_t s = 0; s < microphoneArray.NumberOfSamples(); s++) {
				for (uint16_t c = 0; c < STREAMING_CHANNELS; c++) {
					buffer[buffer_switch][step][c] = microphoneArray.At(s, c);
				}
				step++;
			}
		}
		pthread_mutex_lock(&bufferMutex[(buffer_switch + 1) % 2]);
		pthread_mutex_unlock(&bufferMutex[buffer_switch]);
		//std::cout << "Buffer " << buffer_switch << " Recorded" << std::endl;
		buffer_switch = (buffer_switch + 1) % 2;
	}

	//end of the program, signal the user that recording have been completed
	std::cout << "------ Recording ended ------" << std::endl;

	for (auto& led : ledRing.Image.leds) { led.red = 0;led.green = 8;led.blue = 0; }
	ledRing.Driver.Write(&ledRing.Image);

	sleep(1);

	for (auto& led : ledRing.Image.leds) { led.red = 0; led.green = 0; led.blue = 0; }
	ledRing.Driver.Write(&ledRing.Image);
	ledRing.Driver.Write(&ledRing.Image);

	return 0;
}


void *networkStream(void *connectionPointer)
{
	using std::unique_ptr;
	using libsocket::inet_stream;

	uint32_t bufferSwitch = 0;
	unique_ptr<inet_stream> *tcpConnection = (unique_ptr<inet_stream> *) connectionPointer;
	while (true) {
		pthread_mutex_lock(&bufferMutex[bufferSwitch]);

		try {
			(*tcpConnection)->
				snd(buffer[bufferSwitch], STREAMING_CHANNELS * BUFFER_SAMPLES_PER_CHANNEL * 2);
		}
		catch (const libsocket::socket_exception& exc)
		{
			//assume network disconnection means recording completed
			networkConnected = false; //set flag
			pthread_mutex_unlock(&bufferMutex[bufferSwitch]);//unlock mutex
			pthread_exit(NULL);//terminate itself
		}

		pthread_mutex_unlock(&bufferMutex[bufferSwitch]);
		std::cout << "Buffer " << bufferSwitch << " Sent" << std::endl;
		bufferSwitch = (bufferSwitch + 1) % 2;
	}
}
