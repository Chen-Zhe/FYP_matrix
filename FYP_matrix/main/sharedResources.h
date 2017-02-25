#pragma once
#include <libsocket/inetserverstream.hpp>
#include <libsocket/exception.hpp>

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

#include "LedController.h"

namespace matrixCreator = matrix_hal;
using namespace std;

extern std::unique_ptr<libsocket::inet_stream> tcpConnection;
extern LedController *LedCon;
extern matrixCreator::MicrophoneArray microphoneArray;

namespace GoogleSpeech {
	void setup();
	void *run(void *null);
	void stop();	
}