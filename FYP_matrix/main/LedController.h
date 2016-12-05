#pragma once

#include "../matrix-hal/cpp/driver/everloop_image.h"
#include "../matrix-hal/cpp/driver/everloop.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

using namespace std;
namespace matrixCreator = matrix_hal;

class LedController {
public:
	matrixCreator::EverloopImage Image;

	LedController(matrixCreator::WishboneBus *bus) {
		Driver.Setup(bus);
	};

	~LedController() {};

	bool updateLed() {
		return Driver.Write(&Image);
	};

	bool turnOffLed() {
		return Driver.Write(&LedOff);
	}

private:
	matrixCreator::Everloop Driver;
	matrixCreator::EverloopImage LedOff;
};