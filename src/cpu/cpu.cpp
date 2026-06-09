

/*todolist:

Registers struct: AF, BC, DE, HL, SP, PC — plus F flag helpers (Z, N, H, C)

CPU::step() → fetch opcode at PC → decode → execute → return cycles

Implement the ~244 unprefixed opcodes (LD, INC, DEC, ADD, JP, CALL, RET…)

Implement the 0xCB prefix table (256 bit-rotation/test ops)

Implementation tip: use a function pointer table indexed by opcode byte
Flag math: H flag = carry out of bit 3; C flag = carry out of bit 7
Store flags in the F register high nibble: bit7=Z, 6=N, 5=H, 4=C

*/
#include "cpu.h"
#include "../memory/mmu.h"
#include <functional>
#include <iostream>


uint16_t CPU::getZeroFlag() {
	//bit 7 isolated is the zero flag on or off
	return(this->reg.AF & Z) != 0;
}

void CPU::setzeroflag(bool state) {
	if (state) {
		//crazy bitwise or to turn on the bit
		this->reg.AF = this->reg.AF | Z;
	}
	else {
		//crazy bitwise and to turn off the bit
		this->reg.AF = this->reg.AF & ~(uint16_t)Z;
	}
}
//point of the getters is to isolate from AF and retern if its set

uint16_t CPU:: getSubtractionFlag(){
	//bit 6 N
	return(this->reg.AF & N) != 0;
}
void CPU:: setSubtractionFlag(bool state){
	//ternary operator just to see if i can
	this->reg.AF = state ? (this->reg.AF | N) : (this->reg.AF & ~(uint16_t)N);
}

uint16_t CPU:: getHalfCarryFlag(){
	//bit5
	return(this->reg.AF & H) != 0;
}
void CPU:: setHalfCarryFlag(bool state){
	if (state) {
		this->reg.AF = reg.AF | H;
	}
	else { this->reg.AF = reg.AF & ~(uint16_t)H; }
}

uint16_t CPU:: getCarryFlag(){
	return(this->reg.AF & C) != 0;
}
void CPU:: setCarryFlag(bool state){
	if (state) {
		this->reg.AF = reg.AF | C;
	}
	else { this->reg.AF = reg.AF & ~(uint16_t)C; }
}



CPU::CPU(MMU& mmu) : mmu(mmu) {
	//fill my stubs
	fillStub();

	instruction_table[0x00] = [this]() -> uint8_t {return op_nop();};


	//group 1 wiring ldr8r8
	for(auto i = 0x40; i <= 0x7F; ++i) {
		if (i != 0x76) {
			instruction_table[i] = [this]() -> uint8_t {return op_ld_r8_r8(); };
		}
		else {
			instruction_table[0x76] = [this]() -> uint8_t {return op_halt(); };
		}
	}

	//wire ldr8n8
	//increments each 8th one
	for(auto i = 0x06; i <= 0x3E; i += 0x08){
		instruction_table[i] = [this]() -> uint8_t {return op_ld_r8_n8(); };
	}

	//wire ldr16n16
	for (auto i = 0x01; i <= 0x31; i += 0x10) {
		instruction_table[i] = [this]() -> uint8_t {return op_ld_r16_n16(); };
	}


	//wire incr8
	for (auto i = 0x04; i <= 0x3C; i += 0x08) {
		instruction_table[i] = [this]() -> uint8_t {return op_inc_r8(); };
	}

	//wire decr8

	for (auto i = 0x05; i <= 0x3D; i += 0x08) {
		instruction_table[i] = [this]() -> uint8_t {return op_dec_r8(); };
	}

	//wire inc16 and dec16

	for (auto i = 0x03; i <= 0x33; i += 0x10) {
		instruction_table[i] = [this]() -> uint8_t {return op_inc_r16();};
	}

	for (auto i = 0x0B; i <= 0x3B; i += 0x10) {
		instruction_table[i] = [this]() -> uint8_t {return op_dec_r16();};
	}


};

uint8_t CPU::fetch(){
	//read byte at pc
	uint16_t data = mmu.read8(this->reg.PC);

	//increment pc
	reg.PC += 1;

	return data;

}


uint8_t CPU::step() {
	opcode = fetch(); 
	
	//IF THIS THE FIRST READ THE NEXT ONE
	if (opcode == 0xCB) {
		opcode = fetch();
		return CB_table[opcode]();
	}
	return instruction_table[opcode]();
}

void CPU::fillStub() {
	for (int i = 0; i < 256; ++i) {
		instruction_table[i] = [i]() -> uint8_t {
			std::cout << "stubbed at instruction: " << std::hex<< i << std::endl;
			return 4;
			};

		CB_table[i] = [i]() -> uint8_t {
			std::cout << "stubbed at CB instruction: " << std::hex << i << std::endl;
			return 4;
			};
	}
}

uint8_t CPU:: getRegister(uint8_t id) {
	switch (id) {
	case 0:
		return (reg.BC >> 8) & 0xFF;

	case 1:
		return reg.BC & 0xFF;

	case 2:
		return (reg.DE >> 8) & 0xFF;
		
	case 3:
		return reg.DE & 0xFF;

	case 4:
		return (reg.HL >> 8) & 0xFF;
	
	case 5:
		return reg.HL & 0xFF;
		
	case 6:
		return mmu.read8(reg.HL);

	case 7:
		return (reg.AF >> 8) & 0xFF;

	default:
		break;

	}
}
void CPU::setRegister(uint8_t id, uint8_t val) {
	switch (id) {
	case 0:
		reg.BC = (reg.BC & 0x00FF) | ((uint16_t)val << 8);
		break;

	case 1:
		reg.BC = (reg.BC & 0xFF00) | val;
		break;

	case 2:
		reg.DE = (reg.DE & 0x00FF) | ((uint16_t)val << 8);
		break;

	case 3:
		reg.DE = (reg.DE & 0xFF00) | val;
		break;
	case 4:
		reg.HL = (reg.HL & 0x00FF) | ((uint16_t)val << 8);
		break;

	case 5:
		reg.HL = (reg.HL & 0xFF00) | val;
		break;

	case 6:
		mmu.write8(reg.HL, val);
		break;

	case 7:
		reg.AF = (reg.AF & 0x00FF) | ((uint16_t)val << 8);
		break;

	default:
		break;

	}
}

uint16_t CPU::getRegister16(uint16_t id) {
	switch (id) {
	case 0:
		return reg.BC;

	case 1:
		return reg.DE;

	case 2:
		return reg.HL;

	case 3:
		return reg.SP;
	default:
		break;
	}
	
}

void CPU::setRegister16(uint8_t id, uint16_t val) {
	switch (id) {
	case 0:
		reg.BC = val;
		break;

	case 1:
		reg.DE = val;
		break;

	case 2:
		reg.HL = val;
		break;

	case 3:
		reg.SP = val;
		break;

	default:
		break;
	}

}




//1 M CYCLE = 4 T CYCLES
//gbop t values = what handlers return 

//what does it read / write
//what is the operation
//what happens to flags - untouched 0 force clear 1 force set ZHC computed
//what does it return 


uint8_t CPU::op_nop() {
	return 4;
}

uint8_t CPU::op_ld_r8_r8() {
	
	uint8_t source = opcode & 0x07;
	uint8_t destination = (opcode >> 3) & 0x07;
	uint8_t val = getRegister(source);
	setRegister(destination, val);

	if (source == 6 || destination == 6) {
		return 8;
	}
	else { return 4; }


}

uint8_t CPU::op_halt() {
	//case for ime being called

	//ime not called

	//whatever
	
	return 4; 

}

//group 0

uint8_t CPU::op_ld_r8_n8() {
	//load value n8 into r8
	//2 cycles 2 bytes
	//no flag

	uint8_t destination = (opcode >> 3) & 0x07;

	uint8_t val = fetch();

	setRegister(destination, val);

	return 8;
}


uint8_t CPU::op_ld_r16_n16() {
	//load value n16 into register r16
	//3 cycle 3 byte no flag


	//dest is in bits 5 4. 2 bit field
	//want 54 in loweset position
	//idk how the mask works but its 03
	uint16_t destination = (opcode >> 4) & 0x03;

	//what!
	uint8_t lo_val = fetch();
	uint8_t hi_val = fetch();


	uint16_t val = (hi_val << 8) | lo_val;

	setRegister16(destination, val);

	return 12;
	
}

uint8_t CPU::op_inc_r8() {
	//inc val of register r8 by 1
	uint8_t destination = (opcode >> 3) & 0x07;

	uint8_t val = getRegister(destination);

	//H set if overflow from bit 3s
	setHalfCarryFlag((val & 0x0F) == 0xF);

	val += 1;

	//set Z flag if result is 0
	setzeroflag(val == 0);

	//N = 0
	setSubtractionFlag(0);

	setRegister(destination, val);
	
	if (destination == 6) { return 12; }
	else
	{
		return 4;
	};
	
}

uint8_t CPU::op_dec_r8() {
	uint8_t destination = (opcode >> 3) & 0x07;
	uint8_t val = getRegister(destination);

	//h borrw from bit 4
	setHalfCarryFlag((val & 0xF) == 0);

	val -= 1;

	setzeroflag(val == 0);

	setSubtractionFlag(1);

	setRegister(destination, val);

	if (destination == 6) { return 12; }
	else
	{
		return 4;
	};
}

uint8_t CPU::op_inc_r16() {
	
	uint8_t destination = (opcode >> 4) & 0x03;
	uint16_t val = getRegister16(destination);


	val += 1;

	setRegister16(destination, val);

	return 8;
}

uint8_t CPU::op_dec_r16() {
	uint8_t destination = (opcode >> 4) & 0x03;
	uint16_t val = getRegister16(destination);

	val -= 1;

	setRegister16(destination, val);

	return 8;
}

