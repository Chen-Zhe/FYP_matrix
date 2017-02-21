#include <iostream>
#include <cmath>
#include <fstream>
#include <unistd.h>
#include <string>

#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>

#include "LedController.h"

#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

#include "../doa/Doa.h"
#include "../webrtc-vad/vad/include/webrtc_vad.h"

#define FRAME_SIZE 480 //must divide shift size without remainder
#define NUM_CHANNELS 8
#define SHIFT_SIZE 7680 //must be a multiple of 128
const int32_t numFramesPerShift = SHIFT_SIZE / FRAME_SIZE;
const int32_t frameByteSize = FRAME_SIZE * sizeof(int16_t);
#define VAD_DOA_Q "/vad_doa_q"
#define REC_VAD_Q "/rec_vad_q"

namespace matrixCreator = matrix_hal;

void *voiceActivityDetector(void *null);
void *DOAcalculation(void *null);

//float normalizedBuffer[3][NUM_CHANNELS][SHIFT_SIZE];
int16_t originalBuffer[3][NUM_CHANNELS][SHIFT_SIZE];

//pthread_mutex_t normalizedBufferMutex[3] = { PTHREAD_MUTEX_INITIALIZER };

LedController *LedCon;

int main(int argc, char *argv[]) {
	//initilization of all message queues, parameters, devices, etc.

	//init microphone array and LEDs
	matrixCreator::WishboneBus bus;
	bus.SpiInit();

	matrixCreator::MicrophoneArray microphoneArray;

	LedCon = new LedController(&bus);
	LedCon->turnOffLed();

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
	//pthread_mutex_lock(&normalizedBufferMutex[buffer_switch]);

	pthread_t VAD;
	pthread_t DOA;
	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);
	pthread_create(&DOA, NULL, DOAcalculation, (void *)NULL);

	microphoneArray.Setup(&bus);

	//------recorder thread------
	while (true) {
		int32_t step = 0;
		while (step < SHIFT_SIZE) {

			microphoneArray.Read();

			for (int32_t sample = 0; sample < microphoneArray.NumberOfSamples(); sample++) {
				for (int32_t ch = 0; ch < NUM_CHANNELS; ch++) {
					//normalizedBuffer[buffer_switch][c][step] = microphoneArray.At(s, c) / 32768.0;
					originalBuffer[buffer_switch][ch][step] = microphoneArray.At(sample, ch);
				}
				step++;

				if (step%FRAME_SIZE == 0) {
					mq_send(toVad, (char*)&originalBuffer[buffer_switch][0][step - FRAME_SIZE],frameByteSize, 0);
				}
			}
		}
		//pthread_mutex_lock(&normalizedBufferMutex[(buffer_switch + 1) % 3]);
		//pthread_mutex_unlock(&normalizedBufferMutex[buffer_switch]);
		buffer_switch = (buffer_switch + 1) % 3;
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
				
				//LedCon->updateLed();

				mq_send(toDoa, (char*)&vadPositiveMsg, 4, 0);
				std::cout << "Voice Detected" << std::endl;
			}
			else {//no voice activity				
				//LedCon->turnOffLed();
				extendVadFor1Frame = false;

				mq_send(toDoa, (char*)&vadNegativeMsg, 4, 0);
				std::cout << "No Voice" << std::endl;
			}
			vadPositiveCount = 0;			
		}

	}
}


void *DOAcalculation(void *null) {
	float internalBuffer[NUM_CHANNELS][SHIFT_SIZE];
	Doa DOA(16000, 8, 30720, SHIFT_SIZE);
	DOA.initialize();	
	DoaOutput result;
	uint32_t bufferSwitch = 0;

	int32_t shiftVadStatus;

	mqd_t fromVad = mq_open(VAD_DOA_Q, O_RDONLY);

	//------DOA thread------
	while (true) {
		//pthread_mutex_lock(&normalizedBufferMutex[bufferSwitch]);

		for (int ch = 0; ch < NUM_CHANNELS; ch++) {
			for (int sample = 0; sample < SHIFT_SIZE; sample++) {
				internalBuffer[ch][sample] = originalBuffer[bufferSwitch][ch][sample];
			}
		}

		mq_receive(fromVad, (char*)&shiftVadStatus, 4, NULL); //blocking

		result = DOA.processBuffer((float*)internalBuffer, shiftVadStatus);
		if (result.hasDOA) {
			std::cout << "theta1 = " << result.theta1 << " theta2 = " << result.theta2 << std::endl;

			int ledOffset = 35 * result.theta2 / 360;
			for (int i = 34; i >= 0; i--) {
				if ((35 + 28 + ledOffset - i) % 35 <= 2 || (35 + 11 + ledOffset - i) % 35 <= 2) {
					LedCon->Image.leds[i].red = 3;
				}
				else {
					LedCon->Image.leds[i].red = 0;
				}
			}
			LedCon->updateLed();
		}
		else
			LedCon->turnOffLed();

		//pthread_mutex_unlock(&normalizedBufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 3;
	}

}


/*
//re-purposed for VAD speech segment recording
void *DOAcalculation(void *null) {
	uint32_t bufferSwitch = 0;
	uint32_t name = 0;
	bool fileWritten = false;

	int32_t shiftVadStatus;

	mqd_t fromVad = mq_open(VAD_DOA_Q, O_RDONLY);

	std::string filename = "vad_" + std::to_string(name) + ".pcm";
	std::ofstream *file;

	//------DOA thread------
	while (true) {
		//pthread_mutex_lock(&normalizedBufferMutex[bufferSwitch]);
		mq_receive(fromVad, (char*)&shiftVadStatus, 4, NULL); //blocking

		if (shiftVadStatus > 0) {
			if (!fileWritten) {
				file = new std::ofstream(filename, std::ofstream::binary);
				//supposed to be -1, but due to c's implementation of modulo operation, it has to be +2
				file->write((const char*)originalBuffer[(bufferSwitch + 2) % 3][0], SHIFT_SIZE * sizeof(int16_t));
				fileWritten = true;
			}				

			file->write((const char*)originalBuffer[bufferSwitch][0], SHIFT_SIZE * sizeof(int16_t));
		}
		else if (fileWritten) {
			file->close();
			std::cout << "file closed" << std::endl;
			name++;
			filename = "vad_" + std::to_string(name) + ".pcm";
			delete file;			
			fileWritten = false;
		}

		//pthread_mutex_unlock(&normalizedBufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 3;
	}

}
*/