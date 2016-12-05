#include <pthread.h>
#include <iostream>
#include <cmath>
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"
#include "LedController.h"

#define FRAME_SIZE 384

namespace matrixCreator = matrix_hal;

double teagerEnergy(double frame[]);
void *voiceActivityDetector(void *null);

double buffer[2][FRAME_SIZE];
pthread_mutex_t bufferMutex[2] = { PTHREAD_MUTEX_INITIALIZER };

LedController *LedCon;

int main() {
	matrixCreator::WishboneBus bus;
	bus.SpiInit();

	matrixCreator::MicrophoneArray microphoneArray;

	LedCon = new LedController(&bus);

	for (matrixCreator::LedValue& led : LedCon->Image.leds) {
		led.red = 10;
	}

	int32_t buffer_switch = 0;

	pthread_mutex_lock(&bufferMutex[buffer_switch]);

	pthread_t VAD;//spawn networking thread and pass the connection
	pthread_create(&VAD, NULL, voiceActivityDetector, (void *)NULL);

	microphoneArray.Setup(&bus);

	while (true) {
		uint32_t step = 0;
		while (step < FRAME_SIZE) {

			microphoneArray.Read();

			for (uint32_t s = 0; s < microphoneArray.NumberOfSamples(); s++) {
				buffer[buffer_switch][step] = (double)microphoneArray.At(s, 0)/32768.0;
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
	double fac2 = 2;
	double th1 = fac1*nf1;
	double th2 = fac2*nf2;

	double tge1;
	uint32_t init = 0;

	while (true) {
		pthread_mutex_lock(&bufferMutex[bufferSwitch]);

		tge1 = sqrt(fabs(teagerEnergy(buffer[bufferSwitch])));

		if (tge1 < th1 || init < 20) {
			if (tge1 < nf1) alpha = 0.98;
			else alpha = 0.9;
			init++;
			nf1 = fmin(alpha*nf1 + (1 - alpha)*tge1, 0.02);
		}
		
		th1 = fac1*nf1;
		th2 = fac2*nf1;

		if (tge1 > th2) {
			//voice activity detected
			LedCon->updateLed();
		}
		else {
			//no voice activity
			LedCon->turnOffLed();
		}

		pthread_mutex_unlock(&bufferMutex[bufferSwitch]);
		bufferSwitch = (bufferSwitch + 1) % 2;
	}
}

double teagerEnergy(double frame[]) {
	double tgm = 0.0;

	for (int32_t i = 0; i < FRAME_SIZE - 2; i++) {

		double item = frame[i + 1] * frame[i + 1] - frame[i] * frame[i + 2];

		//calculate mean
		tgm += item / (FRAME_SIZE - 2);
	}

	return tgm;
}