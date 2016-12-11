#pragma once
#include<iostream>
#include<cmath>

	const uint16_t kMicarrayBufferSize = 128;
	const uint16_t kMicrophoneChannels = 1;
	const uint32_t kSamplingRate = 16000;

	class MicrophoneArray{
	public:
		MicrophoneArray();

		~MicrophoneArray();

		bool Read();

		void SetGain(int16_t gain) { gain_ = gain; }
		uint16_t Channels() { return kMicrophoneChannels; }

		uint32_t SamplingRate() { return kSamplingRate; }

		uint32_t NumberOfSamples() {
			return kMicarrayBufferSize / kMicrophoneChannels;
		}

		int16_t& At(int16_t sample, int16_t channel) {
			return sample;
		}

	private:
		//int16_t raw_data_[kMicarrayBufferSize];
		int16_t gain_;
	};