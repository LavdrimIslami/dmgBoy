#pragma once
#include "../memory/mmu.h"

class timer {
private:
	timer(MMU& mmu);

public:

	uint8_t tick(uint8_t cycles);

};