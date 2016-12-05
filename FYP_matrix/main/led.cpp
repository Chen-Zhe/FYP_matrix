/*
* Copyright 2016 <Admobilize>
* MATRIX Labs  [http://creator.matrix.one]
* This file is part of MATRIX Creator HAL
*
* MATRIX Creator HAL is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <iostream>

#include "LedController.h"
#include "../matrix-hal/cpp/driver/wishbone_bus.h"

namespace hal = matrix_hal;

int main() {
	hal::WishboneBus bus;

	bus.SpiInit();

	LedController LedCon = LedController(&bus);

	unsigned counter = 0;

	while (1) {
		for (hal::LedValue& led : LedCon.Image.leds) {
			led.red = 0;
			led.green = 0;
			led.blue = 0;
			led.white = 0;
		}
		LedCon.Image.leds[(counter / 2) % 35].red = 20;
		LedCon.Image.leds[(counter / 7) % 35].green = 30;
		LedCon.Image.leds[(counter / 11) % 35].blue = 30;
		LedCon.Image.leds[34 - (counter % 35)].white = 10;

		LedCon.updateLed();
		++counter;
		usleep(20000);
	}

	return 0;
}