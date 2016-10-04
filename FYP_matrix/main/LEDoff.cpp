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

	everloop.Write(&ledRing);

	return 0;
}