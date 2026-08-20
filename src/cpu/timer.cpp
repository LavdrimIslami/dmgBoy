#include "timer.h"
#include <array>

bool timer::tick(uint8_t cycles) {
	bool overFlow = false;

	uint8_t N = getSelectedBit();

	for (uint8_t i = 0; i < cycles; ++i) {
		uint8_t old_bit = (counter >> N & 1);

		counter += 1; 


		if (TAC & 0x04) {
			overFlow |= handleOverflow(old_bit, N);
		}
		}


	return overFlow;

}


uint8_t timer::getSelectedBit() {

	uint8_t isolate = this->TAC & 3;
	std::array<int, 4> lookup = { 9,3,5,7 };

	return lookup[isolate];
}


bool timer::handleOverflow(uint8_t old_bit, uint8_t N) {
	uint8_t new_bit = (counter >> N & 1);

	if (old_bit == 1 && new_bit == 0) {
		TIMA++;
		if (TIMA == 0) {
			TIMA = TMA;
			return true;
		}
	}
	return false;
}

uint8_t timer::getTIMA()  {
	return this->TIMA;
}
void timer::setTIMA(uint8_t value) {
	this->TIMA = value;
}

uint8_t timer::getTMA() { return this->TMA; }
void timer::setTMA(uint8_t value) { this->TMA = value; }

uint8_t timer::getTAC() { return this->TAC; }
void timer::setTAC(uint8_t value) { this->TAC = value; }

uint8_t timer::getDIV() { return counter >> 8; }

bool timer::handleDIV() {
	uint8_t N = getSelectedBit();

	uint8_t old_bit = (counter >> N & 1);

	counter = 0;

	return handleOverflow(old_bit, N);
}