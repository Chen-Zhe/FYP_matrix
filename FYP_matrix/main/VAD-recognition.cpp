#include <iostream>
#include <cmath>
#include <fstream>
#include <unistd.h>
#include <string>
#include <grpc++/grpc++.h>

#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <google/cloud/speech/v1beta1/cloud_speech.grpc.pb.h>


#include <libsocket/inetserverstream.hpp>
#include <libsocket/exception.hpp>
#include <libsocket/socket.hpp>

#include "LedController.h"

#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

using google::cloud::speech::v1beta1::RecognitionConfig;
using google::cloud::speech::v1beta1::Speech;
using google::cloud::speech::v1beta1::StreamingRecognizeRequest;
using google::cloud::speech::v1beta1::StreamingRecognizeResponse;


#define FRAME_SIZE 512
#define NUM_CHANNELS 8
#define SHIFT_SIZE 5120
const int32_t numFramesPerShift = SHIFT_SIZE / FRAME_SIZE;
const int32_t frameByteSize = FRAME_SIZE * sizeof(float);
#define VAD_DOA_Q "/vad_doa_q"
#define REC_VAD_Q "/rec_vad_q"
#define ENC_RCO_Q "/enc_rco_q"

namespace matrixCreator = matrix_hal;

float teagerEnergy(float frame[]);
void *voiceActivityDetector(void *null);
void *SpeechEnhancement(void *null);
void *StreamingSpeechRecognition(void *null);

float normalizedBuffer[2][NUM_CHANNELS][SHIFT_SIZE];
int16_t originalBuffer[2][NUM_CHANNELS][SHIFT_SIZE];

typedef std::unique_ptr<grpc::ClientReaderWriter<StreamingRecognizeRequest,StreamingRecognizeResponse>> RecognitionDataStreamer;

pthread_mutex_t normalizedBufferMutex[2] = { PTHREAD_MUTEX_INITIALIZER };

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

	for (matrixCreator::LedValue& led : LedCon->Image.leds) {
		led.red = 10;
	}

	
	try {
		libsocket::inet_stream_server tcpServer("0.0.0.0", "8000", LIBSOCKET_IPv4);

		//signal the user that the server is ready
		std::cout << "Ready to accept transcript connection" << std::endl;

		transcriptReceiver = tcpServer.accept2();
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
	pthread_mutex_lock(&normalizedBufferMutex[buffer_switch]);

	pthread_t VAD;
	pthread_t DOA;
	
	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);
	pthread_create(&DOA, NULL, SpeechEnhancement, (void *)NULL);
	

	microphoneArray.Setup(&bus);

	//------recorder thread------
	while (running) {
		uint32_t step = 0;
		while (step < SHIFT_SIZE) {

			microphoneArray.Read();

			for (uint32_t s = 0; s < microphoneArray.NumberOfSamples(); s++) {
				for (uint32_t c = 0; c < NUM_CHANNELS; c++) {
					normalizedBuffer[buffer_switch][c][step] = microphoneArray.At(s, c) / 32768.0;
					originalBuffer[buffer_switch][c][step] = microphoneArray.At(s, c);
				}
				step++;

				if (step%FRAME_SIZE == 0) {
					mq_send(toVad, (char*)&normalizedBuffer[buffer_switch][0][step - FRAME_SIZE],frameByteSize, 0);
				}
			}
		}
		pthread_mutex_lock(&normalizedBufferMutex[(buffer_switch + 1) % 2]);
		pthread_mutex_unlock(&normalizedBufferMutex[buffer_switch]);
		buffer_switch = (buffer_switch + 1) % 2;
	}
}

void *voiceActivityDetector(void *null) {
	float internalNormalizedFrameBuffer[FRAME_SIZE];

	float alpha = 0.9;
	float nf1 = 0.02;
	float nf2 = 0.03;

	float fac1 = 1.175;
	float fac2 = 1.2;
	float th1 = fac1*nf1;
	float th2 = fac2*nf2;

	float tge1;

	int32_t vadPositiveMsg = 1;
	int32_t vadNegativeMsg = 0;

	int32_t vadPositiveCount = 0;
	bool extendVadFor1Frame = true;

	int32_t frameCount = numFramesPerShift; //count down counter for 10 frames

	int32_t noiseFrames = 20;

	mqd_t fromRecorder = mq_open(REC_VAD_Q, O_RDONLY);
	mqd_t toDoa = mq_open(VAD_DOA_Q, O_WRONLY);

	//------VAD thread------
	while (running) {

		frameCount--;//count-down counters
		
		mq_receive(fromRecorder, (char*)internalNormalizedFrameBuffer, frameByteSize, NULL);

		tge1 = sqrt(fabs(teagerEnergy(internalNormalizedFrameBuffer)));

		if (tge1 < th1 || noiseFrames > 0) {
			if (tge1 < nf1) alpha = 0.98;
			else alpha = 0.9;
			
			noiseFrames--;
			nf1 = fmin(alpha*nf1 + (1 - alpha)*tge1, 0.02);
			th1 = fac1*pow(nf1, 1.5);
			th2 = fac2*nf1;
		}
		
		if (tge1 > th2) vadPositiveCount++;

		if (frameCount == 0) {
			frameCount = numFramesPerShift;//reset count-down counter

			if (vadPositiveCount > 2 || (vadPositiveCount > 0 && extendVadFor1Frame)) {//voice activity detected
				
				if (vadPositiveCount > 2)
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
}

float teagerEnergy(float frame[]) {
	float tgm = 0.0;

	for (int32_t i = 0; i < FRAME_SIZE - 2; i++) {
		float item = frame[i + 1] * frame[i + 1] - frame[i] * frame[i + 2];
		tgm += item;//calculate mean
	}

	return tgm / (FRAME_SIZE - 2);
}


//Enhance speech (not done) and directly stream the speech segment to Google Speech API
void *SpeechEnhancement(void *null) {
	uint32_t bufferSwitch = 0;
	bool streamStarted = false;
	const size_t dataChunkSize = SHIFT_SIZE * sizeof(int16_t);

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

	StreamingRecognizeRequest streamingRequest;

	//------Speech Recognition thread------
	while (running) {
		pthread_mutex_lock(&normalizedBufferMutex[bufferSwitch]);
		mq_receive(fromVad, (char*)&shiftVadStatus, 4, NULL); //blocking

		if (shiftVadStatus > 0) {

			if (!streamStarted) { //first audio frame, send configuration first
				streamStarted = true;
				streamer->Write(configRequest);
			}

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

			delete context;

			// start a new speech stub
			speech = Speech::NewStub(channel);
			context = new grpc::ClientContext();			
			streamer = speech->StreamingRecognize(context);

			mq_send(toRecognition, (char*)&streamer, 4, 0);

			streamStarted = false;
		}

		pthread_mutex_unlock(&normalizedBufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 2;
	}

}

void *StreamingSpeechRecognition(void *null) {
	StreamingRecognizeResponse response;
	RecognitionDataStreamer streamer;
	mqd_t fromEnhancer = mq_open(ENC_RCO_Q, O_RDONLY);

	while(true){
		mq_receive(fromEnhancer, (char*)&streamer, 4, NULL);

		while (streamer->Read(&response)) {  // Returns false when no more to read.
											 // Dump the transcript of all the results.
			std::cout << "response received" << std::endl;
			try {
				for (int r = 0; r < response.results_size(); ++r) {
					auto result = response.results(r);
					*transcriptReceiver << "Result stability: " << std::to_string(result.stability()) << "\n";
					for (int a = 0; a < result.alternatives_size(); ++a) {
						auto alternative = result.alternatives(a);
						*transcriptReceiver << std::to_string(alternative.confidence()) << "\t"
							<< alternative.transcript() << "\n";
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
	}

}