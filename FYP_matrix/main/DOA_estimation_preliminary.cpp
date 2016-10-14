#include <unistd.h>
#include "../headers_for_rpi/fftw3.h"
#include <string.h>
#include <stdint.h>

#include <string>
#include <fstream>
#include <iostream>
#include <valarray>

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/everloop.h"
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

const int N = 128;

class Correlation {
 public:
  Correlation(int N);
  ~Correlation();

  void Exec(int16_t* a, int16_t* b);
  float* Result();

 private:
  void Corr(float* out, float* x, float* y);

  int order_;
  float* in_;
  float* A_;
  float* B_;
  float* C_;
  float* c_;

  fftwf_plan forward_plan_a_;
  fftwf_plan forward_plan_b_;
  fftwf_plan inverse_plan_;
};

Correlation::Correlation(int N) : order_(N) {
  in_ = (float*)fftwf_malloc(sizeof(float) * N);
  A_ = (float*)fftwf_malloc(sizeof(float) * N);
  B_ = (float*)fftwf_malloc(sizeof(float) * N);
  C_ = (float*)fftwf_malloc(sizeof(float) * N);
  c_ = (float*)fftwf_malloc(sizeof(float) * N);

  forward_plan_a_ = fftwf_plan_r2r_1d(N, in_, A_, FFTW_R2HC, FFTW_ESTIMATE);
  forward_plan_b_ = fftwf_plan_r2r_1d(N, in_, B_, FFTW_R2HC, FFTW_ESTIMATE);
  inverse_plan_ = fftwf_plan_r2r_1d(N, C_, c_, FFTW_HC2R, FFTW_ESTIMATE);
}

Correlation::~Correlation() {
  fftwf_destroy_plan(forward_plan_a_);
  fftwf_destroy_plan(forward_plan_b_);
  fftwf_destroy_plan(inverse_plan_);

  fftwf_free(in_);
  fftwf_free(A_);
  fftwf_free(B_);
  fftwf_free(C_);
  fftwf_free(c_);
}

float* Correlation::Result() { return c_; }

void Correlation::Exec(int16_t* a, int16_t* b) {
  for (int i = 0; i < order_; i++) in_[i] = a[i];
  fftwf_execute(forward_plan_a_);
  for (int i = 0; i < order_; i++) in_[i] = b[i];
  fftwf_execute(forward_plan_b_);
  Corr(C_, A_, B_);
  fftwf_execute(inverse_plan_);
  for (int i = 0; i < order_; i++) c_[i] = c_[i] / order_;
}

void Correlation::Corr(float* out, float* x, float* y) {
  memset(reinterpret_cast<void*>(out), 0, order_ * sizeof(float));

  out[0] = x[0] * y[0];                             // r0
  out[order_ / 2] = x[order_ / 2] * y[order_ / 2];  // r(n/2)

  for (int j = 1; j < order_ / 2; j++) {
    float a = x[j];
    float b = x[order_ - j];
    float c = y[j];
    float d = -y[order_ - j];
    out[j] += a * c - b * d;           // Re
    out[order_ - j] += b * c + a * d;  // Im
  }
}

namespace hal = matrix_hal;

int main() {
  hal::WishboneBus bus;
  bus.SpiInit();

  hal::MicrophoneArray mics;
  mics.Setup(&bus);

  hal::Everloop everloop;
  everloop.Setup(&bus);

  hal::EverloopImage LEDring;

  int16_t buffer[mics.Channels()][mics.SamplingRate()];

  Correlation corr(N);
  std::valarray<float> current_mag(4);
  std::valarray<float> current_index(4);

  while (true) {
    mics.Read(); /* Reading 8-mics buffer from de FPGA */

    for (uint32_t s = 0; s < mics.NumberOfSamples(); s++) {
      for (uint16_t c = 0; c < mics.Channels(); c++) { /* mics.Channels()=8 */
        buffer[c][s] = mics.At(s, c);
      }
    }

    for (int channel = 0; channel < 4; channel++) {
      corr.Exec(buffer[channel], buffer[channel + 4]);

      float* c = corr.Result();

      int index = 0;
      float m = c[0];
      for (int i = 1; i < N; i++)
        if (c[i] > m) {
          index = i;
          m = c[i];
        }
      current_mag[channel] = m;
      current_index[channel] = index;
    }

    int dir = 0;
    int index = current_index[0];
    float mag = current_mag[0];
    for (int channel = 1; channel < 4; channel++) {
      if (mag < current_mag[channel]) {
        dir = channel;
        mag = current_mag[channel];
        index  = current_index[channel];
      }
    }
	for (auto& led : LEDring.leds) led.blue = 0;
    if(mag>2e8){
       std::cout << dir << "\t" << index << "\t" <<  mag << std::endl;
	
	
	switch (dir) {
	case 0:
		if (index > 100) LEDring.leds[23].blue = 10;
		else if (index < 20) LEDring.leds[6].blue = 10;
		break;
	case 1:
		if (index > 100) LEDring.leds[27].blue = 10;
		else if (index < 20) LEDring.leds[10].blue = 10;
		break;
	case 2:
		if (index > 100) LEDring.leds[31].blue = 10;
		else if (index < 20) LEDring.leds[14].blue = 10;
		break;
	case 3:
		if (index > 100) LEDring.leds[0].blue = 10;
		else if (index < 20) LEDring.leds[18].blue = 10;
		break;
	}

	
	}
	everloop.Write(&LEDring);
	sleep(1);
  }


  return 0;
}
