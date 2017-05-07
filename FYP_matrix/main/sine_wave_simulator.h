#pragma once
#include<iostream>
#include<cmath>
#include<unistd.h>
using namespace std;

#define PI 3.14159265
class MicrophoneArray {
public:
	int16_t gain_ = 0;

	MicrophoneArray() {};

	~MicrophoneArray() {};

	void Read() {
		usleep(8000);//normal recording
		//usleep(100);//stress testing
	};
	void SetGain(int16_t gain) { gain_ = gain; }
	uint16_t Channels() { return 8; }

	uint32_t SamplingRate() { return 16000; }

	uint32_t NumberOfSamples() {
		return 128;
	}

	int16_t At(int16_t sample, int16_t channel) {
		return sin(sample * PI / 6.4) * 8000;
	}

};