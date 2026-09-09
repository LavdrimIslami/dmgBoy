#include "mmu.h"
#include "Cartridge.h"
#include <iostream>
#include "..//cpu/timer.h"


MMU::MMU(Cartridge& cartridge) : cart(cartridge), ppu_(vram,oam) {}


uint8_t MMU::read8(uint16_t address) {
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
		return vram[address - 0x8000];
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		//external cart ram
		return cart.read(address);
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
		//OAM 
		return oam[address - 0xFE00];
	}
	else if (address >= 0xFEA0 && address <= 0xFEFF) {
		//prohibited
		//can't catch me yet
		return 0xFF;
	}
	else if (address >= 0xFF00 && address <= 0xFF7F) {
		if (address == 0xFF00) {
			uint8_t result = joypadSelect | 0xCF;
			if ((joypadSelect & 0x10) == 0) result &= dpadState;
			if ((joypadSelect & 0x20) == 0) result &= actionState;
			return result;
		}
		if (address == 0xFF0F) {
			return if_reg | 0xE0;
		}
		if (address == 0xFF46) return dmaReg;
		if (address >= 0xFF40 && address <= 0xFF4B) {
			return ppu_.readRegister(address);
		}
		if (address == 0xFF01) {
			return this->buffer;
		}
		if (address == 0xFF04) return timer_.getDIV();
		if (address == 0xFF05) return timer_.getTIMA();
		if (address == 0xFF06) return timer_.getTMA();
		if (address == 0xFF07) return timer_.getTAC();
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
		cart.write(address, value);

	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		cart.write(address, value);

	}
	else if (address >= 0x8000 && address <= 0x9FFF) {
		//VRAM
		vram[address - 0x8000] = value;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		//external cart ram
		cart.write(address, value);

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
		oam[address - 0xFE00] = value;
	}
	else if (address >= 0xFEA0 && address <= 0xFEFF) {
		//prohibited
		//can't catch me yet


	}
	else if (address >= 0xFF00 && address <= 0xFF7F) {
		if (address == 0xFF00) {
			joypadSelect = (joypadSelect & 0x0F) | (value & 0x30);
			return;
		}
		if (address == 0xFF0F) {
			if_reg = value & 0x1F;
			return;
		}
		if (address == 0xFF46) {
			dmaReg = value;
			uint16_t sourceBase = (uint16_t)value << 8;
			for (int i = 0; i < 0xA0; i++) {
				oam[i] = read8(sourceBase + i);
			}
			return;
		}
		if (address >= 0xFF40 && address <= 0xFF4B) {
			ppu_.writeRegister(address, value);
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
			if (timer_.handleDIV()) requestInterrupt(2);
		}
		if (address == 0xFF05) timer_.setTIMA(value);
		if (address == 0xFF06) timer_.setTMA(value);
		if (address == 0xFF07) timer_.setTAC(value);
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
	uint8_t ppuInts = ppu_.tick(cycles);
	if (ppuInts & 0x01) {
		requestInterrupt(0); // VBlank
	}
	if (ppuInts & 0x02) {
		requestInterrupt(1); // STAT
	}

}

void MMU::handleInput(bool isDpad, uint8_t bit, bool isPressed) {
	uint8_t& state = isDpad ? dpadState : actionState;
	bool wasUnpressed = (state & (1 << bit)) != 0;

	if (isPressed) {
		state &= ~(1 << bit); // 0 = pressed
		// Check if this specific group is currently selected by the CPU
		bool isSelected = isDpad ? !(joypadSelect & 0x10) : !(joypadSelect & 0x20);
		if (wasUnpressed && isSelected) {
			requestInterrupt(4); // Joypad interrupt
		}
	}
	else {
		state |= (1 << bit);  // 1 = unpressed
	}
}