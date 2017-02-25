#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>

#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>

#include <grpc++/grpc++.h>
#include <google/cloud/speech/v1beta1/cloud_speech.grpc.pb.h>

#include <libsocket/inetserverstream.hpp>
#include <libsocket/exception.hpp>

#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

#include "../webrtc-vad/vad/include/webrtc_vad.h"

#include "sharedResources.h"

namespace GoogleSpeech {

using google::cloud::speech::v1beta1::RecognitionConfig;
using google::cloud::speech::v1beta1::Speech;
using google::cloud::speech::v1beta1::StreamingRecognizeRequest;
using google::cloud::speech::v1beta1::StreamingRecognizeResponse;

#define FRAME_SIZE 480
#define NUM_CHANNELS 1
#define SHIFT_SIZE 7680
const int32_t numFramesPerShift = SHIFT_SIZE / FRAME_SIZE;
const int32_t frameByteSize = FRAME_SIZE * sizeof(int16_t);
#define VAD_GCS_Q "/vad_gcs_q"
#define REC_VAD_Q "/rec_vad_q"
#define GCS_RES_Q "/gcs_res_q"

void *voiceActivityDetector(void *null);
void *SpeechRecognition(void *null);
void *RecognitionResultStream(void *null);

int16_t originalBuffer[3][NUM_CHANNELS][SHIFT_SIZE];
int16_t emptyBuffer[SHIFT_SIZE] = { 0 };

typedef std::unique_ptr<grpc::ClientReaderWriter<StreamingRecognizeRequest,StreamingRecognizeResponse>> RecognitionDataStreamer;
typedef grpc::ClientReaderWriter<StreamingRecognizeRequest, StreamingRecognizeResponse>* StreamerRawPointer;

bool recording = true;
bool recognizing = true;

void stop() { recording = false; }

void setup() {
	//------init all queues------
	//init recorder -> VAD queue to send 1 frame of normalized recording
	struct mq_attr rec_vad_attr;
	rec_vad_attr.mq_flags = 0;
	rec_vad_attr.mq_maxmsg = 10;
	rec_vad_attr.mq_msgsize = frameByteSize;
	rec_vad_attr.mq_curmsgs = 0;
	mq_open(REC_VAD_Q, O_CREAT, 0644, &rec_vad_attr);

	//init recorder -> VAD queue to send VAD result
	struct mq_attr vad_doa_attr;
	vad_doa_attr.mq_flags = 0;
	vad_doa_attr.mq_maxmsg = 10;
	vad_doa_attr.mq_msgsize = 4;
	vad_doa_attr.mq_curmsgs = 0;
	mq_open(VAD_GCS_Q, O_CREAT, 0644, &vad_doa_attr);

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 4;
	attr.mq_curmsgs = 0;
	mq_open(GCS_RES_Q, O_CREAT, 0644, &attr);
}

void *run(void *null) {
	//initialize control variables
	recording = true;
	recognizing = true;

	//init microphone array and LEDs
	for (matrixCreator::LedValue& led : LedCon->Image.leds) {
		led.red = 2; led.green = 0; led.blue = 0;
	}

	mqd_t toVad = mq_open(REC_VAD_Q, O_WRONLY);

	int32_t buffer_switch = 0;

	//------init all threads------

	pthread_t VAD;
	pthread_t googleSpeech;

	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);
	pthread_create(&googleSpeech, NULL, SpeechRecognition, (void *)NULL);

	//------recorder thread------
	while (recording) {
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

	recognizing = false;
	mq_close(toVad);
	pthread_join(VAD, NULL);
	pthread_join(googleSpeech, NULL);
	pthread_exit(NULL);
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
	mqd_t toGoogleSpeech = mq_open(VAD_GCS_Q, O_WRONLY);

	//------VAD thread------
	while (recognizing) {

		frameCount--;//count-down counters

		mq_receive(fromRecorder, (char*)internalFrameBuffer, frameByteSize, NULL);

		vadPositiveCount += WebRtcVad_Process(webRtcVad, internalFrameBuffer, 480);

		if (frameCount == 0) {
			frameCount = numFramesPerShift;//reset count-down counter

			if (vadPositiveCount > 8 || (vadPositiveCount > 0 && extendVadFor1Frame)) {//voice activity detected

				if (vadPositiveCount > 8)
					extendVadFor1Frame = true;
				else
					extendVadFor1Frame = false;

				LedCon->updateLed();

				mq_send(toGoogleSpeech, (char*)&vadPositiveMsg, 4, 0);
				std::cout << "Voice Detected" << std::endl;
			}
			else {//no voice activity				
				LedCon->turnOffLed();
				extendVadFor1Frame = false;

				mq_send(toGoogleSpeech, (char*)&vadNegativeMsg, 4, 0);
				std::cout << "No Voice" << std::endl;
			}
			vadPositiveCount = 0;
		}

	}
	mq_close(fromRecorder);
	mq_close(toGoogleSpeech);
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

	mqd_t fromVad = mq_open(VAD_GCS_Q, O_RDONLY);
	mqd_t toResultStream = mq_open(GCS_RES_Q, O_WRONLY);
	
	pthread_t resultStream;
	pthread_create(&resultStream, NULL, RecognitionResultStream, (void *)NULL);

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
	StreamerRawPointer streamerRawPtr = streamer.get();

	mq_send(toResultStream, (char*)&streamerRawPtr, 4, 0);
	mq_send(toResultStream, (char*)context, 4, 0);

	StreamingRecognizeRequest streamingRequest;

	//------Speech Recognition thread------
	while (recognizing) {
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
			streamerRawPtr = streamer.get();

			mq_send(toResultStream, (char*)&streamerRawPtr, 4, 0);
			mq_send(toResultStream, (char*)context, 4, 0);

			streamStarted = false;
		}

		bufferSwitch = (bufferSwitch + 1) % 3;
	}
	header.dataSize = 32000 * counter;
	header.overallSize = header.dataSize + 36;
	wholeRec.seekp(0);
	wholeRec.write((const char*)&header, sizeof(WaveHeader));
	wholeRec.close();

	streamer->WritesDone();
	mq_close(toResultStream);
	mq_close(fromVad);

	pthread_join(resultStream, NULL);
	pthread_exit(NULL);
}

void *RecognitionResultStream(void *null) {
	StreamingRecognizeResponse response;
	StreamerRawPointer streamer;
	grpc::ClientContext *context;
	mqd_t fromEnhancer = mq_open(GCS_RES_Q, O_RDONLY);

	while(recognizing){
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
						*tcpConnection << is_final + alternative.transcript() + '|';					
					}
				}

			}

			catch (const libsocket::socket_exception& exc)
			{
				std::cout << "Network Disconnected" << std::endl;
				recording = false;
				break;
			}
		}
		
		delete context;
	}
	mq_close(fromEnhancer);
	pthread_exit(NULL);
}

}