#include <pthread.h>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"
#include "LedController.h"
#include "../doa/Doa.h"
#include <mqueue.h>
#include <fcntl.h>
//#include <bits/fcntl-linux.h>
//#include <errno.h>

#define FRAME_SIZE 384
#define NUM_CHANNELS 8
#define VAD_DOA_Q "/vad_doa_Q"
#define REC_VAD_Q "/rec_vad_Q"


namespace matrixCreator = matrix_hal;

double teagerEnergy(float frame[]);
void *voiceActivityDetector(void *null);
void *DOAcalculation(void *null);

float buffer[2][NUM_CHANNELS][FRAME_SIZE], doaFrameBuffer[NUM_CHANNELS][FRAME_SIZE];

pthread_mutex_t bufferMutex[2] = { PTHREAD_MUTEX_INITIALIZER };

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

	//init recorder -> VAD queue to send 1 frame of normalized recording
	struct mq_attr rec_vad_attr;
	rec_vad_attr.mq_flags = 0;
	rec_vad_attr.mq_maxmsg = 10;
	rec_vad_attr.mq_msgsize = FRAME_SIZE*sizeof(float);
	rec_vad_attr.mq_curmsgs = 0;
	mqd_t toVad = mq_open(VAD_DOA_Q, O_CREAT | O_WRONLY, 0644, &rec_vad_attr);

	//init recorder -> VAD queue to send VAD result
	struct mq_attr vad_doa_attr;
	vad_doa_attr.mq_flags = 0;
	vad_doa_attr.mq_maxmsg = 10;
	vad_doa_attr.mq_msgsize = 4;
	vad_doa_attr.mq_curmsgs = 0;
	mq_open(VAD_DOA_Q, O_CREAT, 0644, &vad_doa_attr);


	int32_t buffer_switch = 0;

	pthread_mutex_lock(&bufferMutex[buffer_switch]);

	pthread_t VAD;
	

	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);
	microphoneArray.Setup(&bus);

	while (true) {
		uint32_t step = 0;
		while (step < FRAME_SIZE) {

			microphoneArray.Read();

			for (uint32_t s = 0; s < microphoneArray.NumberOfSamples(); s++) {
				for (uint32_t c = 0; c < NUM_CHANNELS; c++) {
					buffer[buffer_switch][c][step] = microphoneArray.At(s, c) / 32768.0;
				}
				step++;
			}
		}
		pthread_mutex_lock(&bufferMutex[(buffer_switch + 1) % 2]);
		pthread_mutex_unlock(&bufferMutex[buffer_switch]);
		buffer_switch = (buffer_switch + 1) % 2;
	}
}

void *voiceActivityDetector(void *null) {
	uint32_t bufferSwitch = 0;

	double alpha = 0.9;
	double nf1 = 0.02;
	double nf2 = 0.03;

	double fac1 = 1.175;
	double fac2 = 1.5;
	double th1 = fac1*nf1;
	double th2 = fac2*nf2;

	double tge1;

	int32_t vadPositiveCount = 0;
	bool extendVadFor1Frame = true;
	int32_t frameCount = 10; //count down counter for 10 frames

	int32_t noiseFrames = 20;

	char msg[4]="T";
	mqd_t messageQueue = mq_open(VAD_DOA_Q, O_WRONLY);

	pthread_t DOA;
	//pthread_create(&DOA, NULL, DOAcalculation, (void *)NULL);

	while (true) {
		pthread_mutex_lock(&bufferMutex[bufferSwitch]);

		frameCount--;//count-down counters

		tge1 = sqrt(fabs(teagerEnergy(buffer[bufferSwitch][0])));

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
			frameCount = 10;//reset count-down counter

			if (vadPositiveCount > 2 || (vadPositiveCount > 1 && extendVadFor1Frame)) {//voice activity detected		
				
				if (vadPositiveCount > 2)
					extendVadFor1Frame = true;
				else
					extendVadFor1Frame = false;
				
				LedCon->updateLed();

				std::cout << "msg" << std::endl;
				//mq_send(messageQueue, msg, 4, 0);
			}
			else {//no voice activity			
				LedCon->turnOffLed();
				extendVadFor1Frame = false;
			}
			vadPositiveCount = 0;			
		}

		pthread_mutex_unlock(&bufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 2;
	}
}

double teagerEnergy(float frame[]) {
	double tgm = 0.0;

	for (int32_t i = 0; i < FRAME_SIZE - 2; i++) {
		double item = frame[i + 1] * frame[i + 1] - frame[i] * frame[i + 2];		
		tgm += item;//calculate mean
	}

	return tgm / (FRAME_SIZE - 2);
}


void *DOAcalculation(void *null) {
	Doa DOA(16000,8, 15360, FRAME_SIZE);
	DOA.initialize();	
	DoaOutput result;
	
	char msg[4];
	mqd_t messageQueue = mq_open(VAD_DOA_Q, O_RDONLY);

	while (true) {
		mq_receive(messageQueue, msg, 4, NULL); //blocking I/O
		std::cout << "doa" << std::endl;
		result = DOA.processBuffer((float*)doaFrameBuffer, true);
		if (result.hasDOA)
			std::cout << "theta1 = " << result.theta1 << " theta2 = " << result.theta2 << std::endl;
	}

}