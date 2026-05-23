#pragma once
#include "..//memory/mmu.h"

class CPU {
private:
	struct registers
	{
		uint16_t AF; //accumulator and flags HI: A
		uint16_t BC; // HI: B LO: C
		uint16_t DE; // HI: D LO: C
		uint16_t HL; //HI: H LO: L
		uint16_t SP; //stack pointer
		uint16_t PC; //program 

		//f flag helpers
		uint8_t z; //bit 7 zero flag
		uint8_t n; //bit 6 subtraction flag BCD
		uint8_t h; //bit 5 half carry flag BCD
		uint8_t c; //bit 4 carry flag
	};
public:

};