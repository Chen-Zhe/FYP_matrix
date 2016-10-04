#include <unistd.h>
#include <iostream>

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/everloop.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

namespace matrixCreator = matrix_hal;

int main() {
	matrixCreator::WishboneBus bus;

	bus.SpiInit();

	matrixCreator::Everloop everloop;
	matrixCreator::EverloopImage ledRing;

	everloop.Setup(&bus);

	unsigned counter = 0;

	while (1) {
		//draw red first, then green and blue
		//red=11, green=12, blue=12
		//draw start from counter
		int i = 0;
		for (; i < 11; i++) {
				ledRing.leds[(i+counter) % matrixCreator::kMatrixCreatorNLeds].red = (i+1)*2;
				ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].green = 0;
				ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].blue = 0;
		}

		for (; i < 23; i++) {
			ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].green = (i-10)*2;
			ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].red = 0;
			ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].blue = 0;
		}

		for (; i < 35; i++) {
			ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].blue = (i-22)*2;
			ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].green = 0;
			ledRing.leds[(i + counter) % matrixCreator::kMatrixCreatorNLeds].red = 0;
		}

		everloop.Write(&ledRing);
		counter = (counter + 1) % matrixCreator::kMatrixCreatorNLeds;
		usleep(200000);
	}

	return 0;
}