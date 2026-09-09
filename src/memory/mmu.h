#pragma once
#include <cstdint>
#include "Cartridge.h"
#include "../cpu/timer.h"

class MMU {
private:
	timer timer_;

	bool romEnabled_FLAG = true;
	Cartridge& cart;
	uint8_t wram[0x2000] = {};
	uint8_t vram[0x2000] = {};
	uint8_t oam[0xA0] = {};
	uint8_t hram[0x7F] = {};
	uint8_t bootRom[256] = {};
	uint8_t ie = {0x00};
	uint8_t if_reg = {0x00};
	uint8_t buffer = {0};
	std::string serialOutput;

	

public:
	//crazy parameterized contructor 
	MMU(Cartridge& cartridge);

	void tick(uint8_t cycles);

	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

	void requestInterrupt(uint8_t bit);
	void clearInterrupt(uint8_t bit);
};
