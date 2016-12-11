#pragma once
#include<iostream>
#include<cmath>
#include<unistd.h>
#define PI 3.14159265
	const uint16_t kMicarrayBufferSize = 128;
	const uint16_t kMicrophoneChannels = 1;
	const uint32_t kSamplingRate = 16000;

	class MicrophoneArray{
	public:
		MicrophoneArray() {
			count = 0;
		};

		~MicrophoneArray() {};

		bool Read() {			
			count++;
			usleep(8000);
			return true;
		};

		void SetGain(int16_t gain) { gain_ = gain; }
		uint16_t Channels() { return kMicrophoneChannels; }

		uint32_t SamplingRate() { return kSamplingRate; }

		uint32_t NumberOfSamples() {
			return kMicarrayBufferSize / kMicrophoneChannels;
		}

		int16_t At(int16_t sample, int16_t channel) {
			
			if (count > 125&&count<300)
				return sin(sample * PI / 6.4) * 8000;
			else
				return 0;
		}

	private:
		//int16_t raw_data_[kMicarrayBufferSize];
		uint32_t count;
		int16_t gain_;
	};