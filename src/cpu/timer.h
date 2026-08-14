#pragma once
#include "../memory/mmu.h"

class timer {
private:
	

	uint16_t counter = { 0 };

public:
	timer(MMU& mmu);
	uint8_t tick(uint8_t cycles);

};