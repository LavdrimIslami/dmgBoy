#pragma once
#include <cstdint>
#include "Cartridge.h"

class MMU {
private:
	bool romEnabled_FLAG = true;
	Cartridge& cart;
	uint8_t wram[0x2000] = {};
	uint8_t hram[0x7F] = {};
	uint8_t bootRom[256] = {};
	uint8_t ie = {0};
	uint8_t buffer = {0};

public:
	//crazy parameterized contructor 
	MMU(Cartridge& cartridge);
	
	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

};
