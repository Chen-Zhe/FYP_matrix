#include <pthread.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <valarray>

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/everloop.h"
#include "../matrix-hal/cpp/driver/microphone_array.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

namespace matrixCreator = matrix_hal;
void *ledRun(void *everloopStruct);

struct LED {
	matrixCreator::Everloop Driver;
	matrixCreator::EverloopImage Image;
};

int main() {
	matrixCreator::WishboneBus bus;
	bus.SpiInit();

	matrixCreator::MicrophoneArray mics;
	mics.Setup(&bus);

	struct LED everloopLed;

	everloopLed.Driver.Setup(&bus);

	for (auto& led : everloopLed.Image.leds) led.red = 10;

	everloopLed.Driver.Write(&everloopLed.Image);

	uint16_t seconds_to_record = 10;

	int16_t buffer[mics.Channels()][seconds_to_record * mics.SamplingRate()];

	uint32_t step = 0;

	sleep(2);

	pthread_t ledThread;
	pthread_create(&ledThread, NULL, ledRun, (void *)&everloopLed);

	while (true) {
		mics.Read(); /* Reading 8-mics buffer from the FPGA */

		for (uint32_t s = 0; s < mics.NumberOfSamples(); s++) {
			for (uint16_t c = 0; c < mics.Channels(); c++) { /* mics.Channels()=8 */
				buffer[c][step] = mics.At(s, c);
			}
			step++;
		}
		if (step == seconds_to_record * mics.SamplingRate()) break;
	}

	pthread_cancel(ledThread);//TODO: change it to using mutex

	for (uint16_t c = 0; c < mics.Channels(); c++) {
		std::string filename = "mic_" + std::to_string(mics.SamplingRate()) +
			"_s16le_channel_" + std::to_string(c) + ".raw";
		std::ofstream os(filename, std::ofstream::binary);
		os.write((const char*)buffer[c],
			seconds_to_record * mics.SamplingRate() * sizeof(int16_t));

		os.close();
	}

	for (auto& led : everloopLed.Image.leds) {
		led.red = 0;
		led.green = 10;
		led.blue = 0;
	}
	everloopLed.Driver.Write(&everloopLed.Image);

	sleep(2);

	for (auto& led : everloopLed.Image.leds) {
		led.green = 0;
	}
	everloopLed.Driver.Write(&everloopLed.Image);

	return 0;
}


void *ledRun(void *everloopStruct)
{
	struct LED *everloopLed = (struct LED *) everloopStruct;

	unsigned counter = 0;

	while (1) {
		//draw red first, then green and blue
		//red=11, green=12, blue=12
		//draw start from counter
		int i = 0;
		for (; i < 11; i++) {
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].red = (i + 1) * 1.5;
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].green = 0;
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].blue = 0;
		}

		for (; i < 23; i++) {
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].green = (i - 10) * 1.5;
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].red = 0;
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].blue = 0;
		}

		for (; i < 35; i++) {
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].blue = (i - 22) * 1.5;
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].green = 0;
			everloopLed->Image.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].red = 0;
		}

		everloopLed->Driver.Write(&everloopLed->Image);
		counter = (counter + 1) % matrixCreator::kMatrixCreatorNLeds;
		usleep(100000);
	}

}
