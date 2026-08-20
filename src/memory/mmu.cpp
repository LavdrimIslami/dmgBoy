#include "mmu.h"
#include "Cartridge.h"
#include <iostream>
#include "..//cpu/timer.h"


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
		if (address == 0xFF0F) {
			return if_reg | 0xE0;   // upper 3 bits always read as 1
		}
		if (address == 0xFF01) {
			return this->buffer;
		}
		if (address == 0xFF04) {
			return timer_.getDIV();
		}
		if (address == 0xFF05) {
			return timer_.getTIMA();
		}
		if (address == 0xFF06) {
			return timer_.getTMA();
		}
		if (address == 0xFF07) {
			return timer_.getTAC();
		}
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
		if (address == 0xFF0F) {
			if_reg = value & 0x1F;  // only lower 5 bits are writable
			return;
		}
		if (address == 0xFF01) {
			this->buffer = value;
		}
		if (address == 0xFF02 && value == 0x81) {
			std::cout << this->buffer;
			std::cout.flush();
			serialOutput += this->buffer;
			if (serialOutput.find("Passed") != std::string::npos ||
				serialOutput.find("Failed") != std::string::npos) {
				exit(0);
			}
		}
		if (address == 0xFF04) {
			if (timer_.handleDIV()) {
				requestInterrupt(2);
			}
		}
		if (address == 0xFF05) {
			timer_.setTIMA(value);
		}
		if (address == 0xFF06) {
			timer_.setTMA(value);
		}
		if (address == 0xFF07) {
			timer_.setTAC(value);
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

void MMU::requestInterrupt(uint8_t bit) {
	if_reg |= (1 << bit);
}
void MMU::clearInterrupt(uint8_t bit) {
	if_reg &= ~(1 << bit);
}

void MMU::tick(uint8_t cycles) {
	if (timer_.tick(cycles)) {
		requestInterrupt(2);
	}

}