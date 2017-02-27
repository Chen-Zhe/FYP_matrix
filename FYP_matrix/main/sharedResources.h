#pragma once
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
	extern int channelToSend;
}