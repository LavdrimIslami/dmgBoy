#include "mmu.h"
#include "Cartridge.h"
#include <iostream>


MMU::MMU(Cartridge& cartridge) : cart(cartridge) {}


uint8_t MMU::read8(uint16_t address) {
	//id love to try out the ... operator but Im pretty sure thats GNU only
	if (address >= 0x0000 && address <= 0x3FFF) {
		//if (romEnabled_FLAG) {
		//	//serve from bootrom
		//	//fill it up later tho
		//	return;
		//}
		return cart.read(address);

	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		//to be changed with mbc
		//if (romEnabled_FLAG) {
		//	//serve from bootrom later
		//	return;
		//}
		return cart.read(address);

	}
	else if (address >= 0x8000 && address <= 0x9FFF) {
		//VRAM
		return 0xFF;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		//external cart ram
		return 0xFF;
	}
	else if (address >= 0xC000 && address <= 0xCFFF) {
		//bank 0
		return wram[address - 0xC000];
	}
	else if (address >= 0xD000 && address <= 0xDFFF) {
		//bank 1
		return wram[address - 0xC000];
	}
	else if (address >= 0xE000 && address <= 0xFDFF) {
		//echo ram mirrors wram
		return wram[address - 0xE000]; 
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		//OAM need ppu
		return 0xFF;
	}
	else if (address >= 0xFEA0 && address <= 0xFEFF) {
		//prohibited
		//can't catch me yet
		return 0xFF;
	}
	else if (address >= 0xFF00 && address <= 0xFF7F) {
		//IO need that 
		return 0xFF;
	}
	else if (address >= 0xFF80 && address <= 0xFFFE) {
		//high ram
		return hram[address - 0xFF80];
	}
	else if (address == 0xFFFF) {
		//interupt enable
		return ie;
	}
	else {
		return 0xFF;
	}

}

void MMU::write8(uint16_t address, uint8_t value) {
	if (address >= 0x0000 && address <= 0x3FFF) {
		//stub it

	}
	else if (address >= 0x4000 && address <= 0x7FFF) {


	}
	else if (address >= 0x8000 && address <= 0x9FFF) {
		//VRAM

	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		//external cart ram

	}
	else if (address >= 0xC000 && address <= 0xCFFF) {
		//bank 0
		wram[address - 0xC000] = value;
	}
	else if (address >= 0xD000 && address <= 0xDFFF) {
		//bank 1
		wram[address - 0xC000] = value;

	}
	else if (address >= 0xE000 && address <= 0xFDFF) {
		//echo ram mirrors wram
		wram[address - 0xE000] = value;
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		//OAM need ppu

	}
	else if (address >= 0xFEA0 && address <= 0xFEFF) {
		//prohibited
		//can't catch me yet


	}
	else if (address >= 0xFF00 && address <= 0xFF7F) {
		//IO need that 
		if (address == 0xFF01) {
			this->buffer = value;
		}
		if (address == 0xFF02 && value == 0x81) {
			std::cout << this->buffer;
			std::cout.flush();
		}

	}
	else if (address >= 0xFF80 && address <= 0xFFFE) {
		//high ram
		hram[address - 0xFF80] = value;
	} 
	else if (address >= 0xFFFF && address <= 0xFFFF) {
		//interupt enable
		ie = value;
	}
	else {

	}
}