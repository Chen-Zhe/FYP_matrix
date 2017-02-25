#pragma once

namespace matrixCreator = matrix_hal;
using namespace std;

std::unique_ptr<libsocket::inet_stream> tcpConnection;
LedController *LedCon;
matrixCreator::MicrophoneArray microphoneArray;

namespace GoogleSpeech {
	void setup();
	void *run(void *null);
	void stop();	
}