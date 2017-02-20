#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <grpc++/grpc++.h>

#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <google/cloud/speech/v1beta1/cloud_speech.grpc.pb.h>

#include <netinet/in.h>
#include <libsocket/inetserverstream.hpp>
#include <libsocket/inetclientdgram.hpp>
#include <libsocket/exception.hpp>
#include <libsocket/socket.hpp>

#include "LedController.h"

#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

#include "../webrtc-vad/vad/include/webrtc_vad.h"

using google::cloud::speech::v1beta1::RecognitionConfig;
using google::cloud::speech::v1beta1::Speech;
using google::cloud::speech::v1beta1::StreamingRecognizeRequest;
using google::cloud::speech::v1beta1::StreamingRecognizeResponse;


#define FRAME_SIZE 480
#define NUM_CHANNELS 8
#define SHIFT_SIZE 7680
const int32_t numFramesPerShift = SHIFT_SIZE / FRAME_SIZE;
const int32_t frameByteSize = FRAME_SIZE * sizeof(int16_t);
#define VAD_DOA_Q "/vad_doa_q"
#define REC_VAD_Q "/rec_vad_q"
#define ENC_RCO_Q "/enc_rco_q"
#define NTP_TIMESTAMP_DELTA 2208988800ull

namespace matrixCreator = matrix_hal;

void * stopMonitor(void * null);

void *voiceActivityDetector(void *null);
void *SpeechRecognition(void *null);
void *StreamingSpeechRecognition(void *null);

void syncTime(string ip);

int16_t originalBuffer[3][NUM_CHANNELS][SHIFT_SIZE];
int16_t emptyBuffer[SHIFT_SIZE] = { 0 };

typedef std::unique_ptr<grpc::ClientReaderWriter<StreamingRecognizeRequest,StreamingRecognizeResponse>> RecognitionDataStreamer;

LedController *LedCon;

bool running = true;

std::unique_ptr<libsocket::inet_stream> transcriptReceiver;

int main() {
	//initilization of all message queues, parameters, devices, etc.

	//init microphone array and LEDs
	matrixCreator::WishboneBus bus;
	bus.SpiInit();

	matrixCreator::MicrophoneArray microphoneArray;

	LedCon = new LedController(&bus);

	LedCon->Image.leds[0].red = 3;
	LedCon->Image.leds[8].red = 3;
	LedCon->Image.leds[17].red = 3;
	LedCon->Image.leds[26].red = 3;

	
	try {
		libsocket::inet_stream_server tcpServer("0.0.0.0", "8000", LIBSOCKET_IPv4);

		//signal the user that the server is ready
		std::cout << "Ready to accept transcript connection" << std::endl;

		transcriptReceiver = tcpServer.accept2();
		syncTime(transcriptReceiver->gethost());
	}
	catch (const libsocket::socket_exception& exc)
	{
		std::cout << exc.mesg << std::endl;
		return 0;
	}

	//------init all queues------
	//init recorder -> VAD queue to send 1 frame of normalized recording
	struct mq_attr rec_vad_attr;
	rec_vad_attr.mq_flags = 0;
	rec_vad_attr.mq_maxmsg = 10;
	rec_vad_attr.mq_msgsize = frameByteSize;
	rec_vad_attr.mq_curmsgs = 0;
	mqd_t toVad = mq_open(REC_VAD_Q, O_CREAT | O_WRONLY, 0644, &rec_vad_attr);

	//init recorder -> VAD queue to send VAD result
	struct mq_attr vad_doa_attr;
	vad_doa_attr.mq_flags = 0;
	vad_doa_attr.mq_maxmsg = 10;
	vad_doa_attr.mq_msgsize = 4;
	vad_doa_attr.mq_curmsgs = 0;
	mq_open(VAD_DOA_Q, O_CREAT, 0644, &vad_doa_attr);

	int32_t buffer_switch = 0;

	//------init all threads------

	pthread_t VAD;
	pthread_t DOA;
	pthread_t monitor;

	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);
	pthread_create(&DOA, NULL, SpeechRecognition, (void *)NULL);
	pthread_create(&monitor, NULL, stopMonitor, (void *)NULL);

	microphoneArray.Setup(&bus);

	//------recorder thread------
	while (running) {
		int32_t step = 0;
		while (step < SHIFT_SIZE) {

			microphoneArray.Read();

			for (int32_t s = 0; s < 128; s++) {
				for (int32_t c = 0; c < NUM_CHANNELS; c++) {
					originalBuffer[buffer_switch][c][step] = microphoneArray.At(s, c);
				}
				step++;

				if (step%FRAME_SIZE == 0) {
					mq_send(toVad, (char*)&originalBuffer[buffer_switch][0][step - FRAME_SIZE], frameByteSize, 0);
				}
			}
		}
		buffer_switch = (buffer_switch + 1) % 3;
	}
}

void *stopMonitor(void *null) {
	char command;
	transcriptReceiver->rcv(&command, 1);
	if (command == 'S') {
		running = false;
		pthread_exit(NULL);
	}
}

void *voiceActivityDetector(void *null) {
	int16_t internalFrameBuffer[FRAME_SIZE];

	VadInst *webRtcVad;
	WebRtcVad_Create(&webRtcVad);
	WebRtcVad_Init(webRtcVad);
	WebRtcVad_set_mode(webRtcVad, 2);

	int32_t vadPositiveMsg = 1;
	int32_t vadNegativeMsg = 0;

	int32_t vadPositiveCount = 0;
	bool extendVadFor1Frame = false;

	int32_t frameCount = numFramesPerShift; //count down counter for 10 frames

	mqd_t fromRecorder = mq_open(REC_VAD_Q, O_RDONLY);
	mqd_t toDoa = mq_open(VAD_DOA_Q, O_WRONLY);

	//------VAD thread------
	while (true) {

		frameCount--;//count-down counters

		mq_receive(fromRecorder, (char*)internalFrameBuffer, frameByteSize, NULL);

		vadPositiveCount += WebRtcVad_Process(webRtcVad, internalFrameBuffer, 480);

		if (frameCount == 0) {
			frameCount = numFramesPerShift;//reset count-down counter

			if (vadPositiveCount > 10 || (vadPositiveCount > 0 && extendVadFor1Frame)) {//voice activity detected

				if (vadPositiveCount > 10)
					extendVadFor1Frame = true;
				else
					extendVadFor1Frame = false;

				LedCon->updateLed();

				mq_send(toDoa, (char*)&vadPositiveMsg, 4, 0);
				std::cout << "Voice Detected" << std::endl;
			}
			else {//no voice activity				
				LedCon->turnOffLed();
				extendVadFor1Frame = false;

				mq_send(toDoa, (char*)&vadNegativeMsg, 4, 0);
				std::cout << "No Voice" << std::endl;
			}
			vadPositiveCount = 0;
		}

	}
	pthread_exit(NULL);
}

//Enhance speech (not done) and directly stream the speech segment to Google Speech API as well as save to disk
void *SpeechRecognition(void *null) {
	int32_t bufferSwitch = 0;
	bool streamStarted = false;
	const size_t dataChunkSize = SHIFT_SIZE * sizeof(int16_t);

	std::ofstream wholeRec("/home/pi/Recordings/whole_session.wav", std::ofstream::binary);
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
		uint16_t numChannels = 1;					// no.of channels
		uint32_t samplingRate = 16000;				// sampling rate (blocks per second)
		uint32_t byteRate = 32000;					// SampleRate * NumChannels * BitsPerSample/8
		uint16_t blockAlign = 2;					// NumChannels * BitsPerSample/8
		uint16_t bitsPerSample = 16;				// bits per sample, 8- 8bits, 16- 16 bits etc

													//data subchunk
		char data[4] = { 'd', 'a', 't', 'a' };		// DATA string or FLLR string
		uint32_t dataSize;							// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
	} header;

	wholeRec.write((const char*)&header, sizeof(WaveHeader));
	
	uint32_t counter = 0;

	int32_t shiftVadStatus;

	mqd_t fromVad = mq_open(VAD_DOA_Q, O_RDONLY);

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 4;
	attr.mq_curmsgs = 0;
	mqd_t toRecognition = mq_open(ENC_RCO_Q, O_CREAT | O_WRONLY, 0644, &attr);
	
	pthread_t RCO;
	pthread_create(&RCO, NULL, StreamingSpeechRecognition, (void *)NULL);

	// start grpc channel
	auto creds = grpc::GoogleDefaultCredentials();
	auto channel = grpc::CreateChannel("speech.googleapis.com", creds);
	
	// configure audio signal property
	StreamingRecognizeRequest configRequest;
	auto* streaming_config = configRequest.mutable_streaming_config();
	streaming_config->mutable_config()->set_sample_rate(16000);
	streaming_config->mutable_config()->set_encoding(RecognitionConfig::LINEAR16);
	streaming_config->mutable_config()->set_language_code("en-US");
	streaming_config->set_interim_results(true);
	streaming_config->set_single_utterance(true);

	// Begin a speech stub
	std::unique_ptr<Speech::Stub> speech(Speech::NewStub(channel));
	grpc::ClientContext *context = new grpc::ClientContext;
	RecognitionDataStreamer streamer = speech->StreamingRecognize(context);

	mq_send(toRecognition, (char*)&streamer, 4, 0);
	mq_send(toRecognition, (char*)context, 4, 0);

	StreamingRecognizeRequest streamingRequest;

	//------Speech Recognition thread------
	while (running) {
		mq_receive(fromVad, (char*)&shiftVadStatus, 4, NULL); //blocking
		counter++;

		if (shiftVadStatus == 0)
			wholeRec.write((const char*)emptyBuffer, SHIFT_SIZE * sizeof(int16_t));

		if (shiftVadStatus > 0) {

			if (!streamStarted) { //first audio frame, send configuration first
				streamStarted = true;
				streamer->Write(configRequest);

				//supposed to be -1, but due to c's implementation of modulo operation, it has to be +2
				wholeRec.write((const char*)originalBuffer[(bufferSwitch + 2) % 3][0], SHIFT_SIZE * sizeof(int16_t));
				streamingRequest.set_audio_content((void *)originalBuffer[(bufferSwitch + 2) % 3][0], dataChunkSize);
				streamer->Write(streamingRequest);
			}

			wholeRec.write((const char*)originalBuffer[bufferSwitch][0], SHIFT_SIZE * sizeof(int16_t));
			streamingRequest.set_audio_content((void *)originalBuffer[bufferSwitch][0], dataChunkSize);
			streamer->Write(streamingRequest);
		}
		else if (streamStarted) {
			streamer->WritesDone();
			grpc::Status status = streamer->Finish();
			if (!status.ok()) {
				std::cerr << "RPC: " << status.error_message() << std::endl;
				//If 'invalid authentication credential' error comes up, manually sync R-pi's time
			}


			// start a new speech stub
			speech = Speech::NewStub(channel);
			context = new grpc::ClientContext();			
			streamer = speech->StreamingRecognize(context);

			mq_send(toRecognition, (char*)&streamer, 4, 0);
			mq_send(toRecognition, (char*)context, 4, 0);

			streamStarted = false;
		}

		bufferSwitch = (bufferSwitch + 1) % 3;
	}

	header.dataSize = 32000 * counter;
	header.overallSize = header.dataSize + 36;
	wholeRec.seekp(0);
	wholeRec.write((const char*)&header, sizeof(WaveHeader));
	wholeRec.close();
	pthread_exit(NULL);
}

void *StreamingSpeechRecognition(void *null) {
	StreamingRecognizeResponse response;
	RecognitionDataStreamer streamer;
	grpc::ClientContext *context;
	mqd_t fromEnhancer = mq_open(ENC_RCO_Q, O_RDONLY);

	while(running){
		mq_receive(fromEnhancer, (char*)&streamer, 4, NULL);
		mq_receive(fromEnhancer, (char*)&context, 4, NULL);

		while (streamer->Read(&response)) {  // Returns false when no more to read.
											 // Dump the transcript of all the results.
			std::cout << "response received" << std::endl;
			try {
				for (int r = 0; r < response.results_size(); ++r) {
					auto result = response.results(r);

					if (result.stability() < 0.8 && !result.is_final()) continue;

					char is_final = result.is_final()? '1' : '0';

					for (int a = 0; a < result.alternatives_size(); ++a) {
						auto alternative = result.alternatives(a);
						*transcriptReceiver << is_final + alternative.transcript() + '|';					
					}
				}

			}

			catch (const libsocket::socket_exception& exc)
			{
				std::cout << "Network Disconnected" << std::endl;
				running = false;
				pthread_exit(NULL);//terminate itself
			}
		}
		
		delete context;
	}

}

void syncTime(string ip) {

	string remoteIP;
	string remotePort;

	remoteIP.resize(16);
	remotePort.resize(16);
	// Total: 384 bits or 48 bytes.
	typedef struct
	{

		unsigned li : 2;       // Only two bits. Leap indicator.
		unsigned vn : 3;       // Only three bits. Version number of the protocol.
		unsigned mode : 3;       // Only three bits. Mode. Client will pick mode 3 for client.

		uint8_t stratum;         // Eight bits. Stratum level of the local clock.
		uint8_t poll;            // Eight bits. Maximum interval between successive messages.
		uint8_t precision;       // Eight bits. Precision of the local clock.

		uint32_t rootDelay;      // 32 bits. Total round trip delay time.
		uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
		uint32_t refId;          // 32 bits. Reference clock identifier.

		uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
		uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

		uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
		uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

		uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
		uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

		uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
		uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

	} ntp_packet;

	ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	memset(&packet, 0, sizeof(ntp_packet));

	// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

	*((char *)&packet) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

	libsocket::inet_dgram_client sock(LIBSOCKET_IPv4);
	sock.sndto(&packet, 48, ip, "1230");
	sock.rcvfrom(&packet, 48, remoteIP, remotePort);


	// These two fields contain the time-stamp seconds as the packet left the NTP server.
	// The number of seconds correspond to the seconds passed since 1900.
	// ntohl() converts the bit/byte order from the network's to host's "endianness".

	packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
	packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

										  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
										  // Subtract 70 years worth of seconds from the seconds since 1900.
										  // This leaves the seconds since the UNIX epoch of 1970.
										  // (1900)------------------(1970)**************************************(Time Packet Left the Server)

	time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);

	char command[30];
	sprintf(command, "sudo date -s '@%d'", txTm);
	
	if (system(command) == -1)
		cout << "Unable to set system time" << endl;
	else
		cout << "Time synchronised with remote PC" << endl;
}