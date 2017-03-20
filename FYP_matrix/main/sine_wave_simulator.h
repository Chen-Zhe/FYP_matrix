#pragma once
#include<iostream>
#include<cmath>
#include<unistd.h>
#define PI 3.14159265
class MicrophoneArray {
public:
	uint32_t count;
	int16_t gain_ = 0;

	MicrophoneArray() {
		count = 0;
	};

	~MicrophoneArray() {};

	void Read() {
		cout << count << endl;
		count++;
		usleep(8000);
	};
	void SetGain(int16_t gain) { gain_ = gain; }
	uint16_t Channels() { return 8; }

	uint32_t SamplingRate() { return 16000; }

	uint32_t NumberOfSamples() {
		return 128;
	}

	int16_t At(int16_t sample, int16_t channel) {
		if (count > 125 && count<300)
			return sin(sample * 3.14159265 / 6.4) * 8000;
		else
			return 0;
	}

};