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


#define FRAME_SIZE 512
#define NUM_CHANNELS 8
#define SHIFT_SIZE 5120
const int32_t numFramesPerShift = SHIFT_SIZE / FRAME_SIZE;
const int32_t frameByteSize = FRAME_SIZE * sizeof(float);
#define VAD_DOA_Q "/vad_doa_q"
#define REC_VAD_Q "/rec_vad_q"

namespace matrixCreator = matrix_hal;

float teagerEnergy(float frame[]);
void *voiceActivityDetector(void *null);
void *DOAcalculation(void *null);

float normalizedBuffer[2][NUM_CHANNELS][SHIFT_SIZE];
int16_t originalBuffer[2][NUM_CHANNELS][SHIFT_SIZE];

pthread_mutex_t normalizedBufferMutex[2] = { PTHREAD_MUTEX_INITIALIZER };

LedController *LedCon;

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
	mqd_t vad_doa = mq_open(VAD_DOA_Q, O_CREAT, 0644, &vad_doa_attr);

	int32_t buffer_switch = 0;

	//------init all threads------
	pthread_mutex_lock(&normalizedBufferMutex[buffer_switch]);

	pthread_t VAD;
	pthread_t DOA;
	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);
	pthread_create(&DOA, NULL, DOAcalculation, (void *)NULL);

	microphoneArray.Setup(&bus);

	//float vadBuffer[FRAME_SIZE];

	//------recorder thread------
	while (true) {
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
	while (true) {

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

			if (vadPositiveCount > 2 || (vadPositiveCount > 1 && extendVadFor1Frame)) {//voice activity detected
				
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


/*
void *DOAcalculation(void *null) {
	Doa DOA(16000, 8, 15360, SHIFT_SIZE);
	DOA.initialize();	
	DoaOutput result;
	uint32_t bufferSwitch = 0;

	int32_t shiftVadStatus;

	mqd_t fromVad = mq_open(VAD_DOA_Q, O_RDONLY);

	//------DOA thread------
	while (true) {
		pthread_mutex_lock(&normalizedBufferMutex[bufferSwitch]);
		mq_receive(fromVad, (char*)&shiftVadStatus, 4, NULL); //blocking

		result = DOA.processBuffer((float*)normalizedBuffer[bufferSwitch], (shiftVadStatus>0));
		if (result.hasDOA)
			std::cout << "theta1 = " << result.theta1 << " theta2 = " << result.theta2 << std::endl;

		pthread_mutex_unlock(&normalizedBufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 2;
	}

}
*/


void *DOAcalculation(void *null) {
	uint32_t bufferSwitch = 0;
	uint32_t name = 0;
	bool fileWritten = false;

	int32_t shiftVadStatus;

	mqd_t fromVad = mq_open(VAD_DOA_Q, O_RDONLY);

	std::string filename = "vad_" + std::to_string(name) + ".raw";
	std::ofstream *file = new std::ofstream(filename, std::ofstream::binary);

	//------DOA thread------
	while (true) {
		pthread_mutex_lock(&normalizedBufferMutex[bufferSwitch]);
		mq_receive(fromVad, (char*)&shiftVadStatus, 4, NULL); //blocking

		if (shiftVadStatus > 0) {
			file->write((const char*)originalBuffer[bufferSwitch][0], SHIFT_SIZE * sizeof(int16_t));
			fileWritten = true;
		}
		else if (fileWritten) {
			file->close();
			std::cout << "file closed" << std::endl;
			name++;
			filename = "vad_" + std::to_string(name) + ".pcm";
			delete file;
			file = new std::ofstream(filename, std::ofstream::binary);
			fileWritten = false;
		}

		pthread_mutex_unlock(&normalizedBufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 2;
	}

}