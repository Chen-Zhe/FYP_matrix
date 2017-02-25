#include "sharedResources.h"

std::unique_ptr<libsocket::inet_stream> tcpConnection;
LedController *LedCon;
matrixCreator::MicrophoneArray microphoneArray;