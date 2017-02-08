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
		ledOff = false;
		return Driver.Write(&Image);
	};

	bool turnOffLed() {
		if (!ledOff) {
			ledOff = true;
			return Driver.Write(&LedOff);
		}			
		else
			return true;
	}

private:
	matrixCreator::Everloop Driver;
	matrixCreator::EverloopImage LedOff;
	bool ledOff = true;
};