

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

	reg.AF = 0x01B0;
	reg.BC = 0x0013;
	reg.DE = 0x00D8;
	reg.HL = 0x014D;
	reg.SP = 0xFFFE;
	reg.PC = 0x0100;


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

	//write ldr16A
	instruction_table[0x02] = [this]()-> uint8_t {return op_ld_r16_A(); };
	instruction_table[0x12] = [this]()-> uint8_t {return op_ld_r16_A(); };

	//wire ldhl i d A
	instruction_table[0x22] = [this]()-> uint8_t {return op_ld_HLI_A(); };
	instruction_table[0x32] = [this]()-> uint8_t {return op_ld_HLD_A(); };

	//wire opldar16
	instruction_table[0x0A] = [this]() -> uint8_t {return op_ld_A_r16(); };
	instruction_table[0x1A] = [this]() -> uint8_t {return op_ld_A_r16(); };
	instruction_table[0x2A] = [this]() -> uint8_t {return op_ld_A_r16(); };
	instruction_table[0x3A] = [this]() -> uint8_t {return op_ld_A_r16(); };



	//wire opldn16sp
	instruction_table[0x08] = [this]()->uint8_t {return op_ld_n16_SP(); };

	//the 2 jrs
	instruction_table[0x18] = [this]()->uint8_t {return op_jr_e8(); };

	instruction_table[0x20] = [this]()->uint8_t {return op_jr_cc_e8(); };
	instruction_table[0x28] = [this]()->uint8_t {return op_jr_cc_e8(); };
	instruction_table[0x30] = [this]()->uint8_t {return op_jr_cc_e8(); };
	instruction_table[0x38] = [this]()->uint8_t {return op_jr_cc_e8(); };


	//rotations
	instruction_table[0x07] = [this]()->uint8_t {return op_rlca(); };
	instruction_table[0x0F] = [this]()->uint8_t {return op_rrca(); };
	instruction_table[0x17] = [this]()->uint8_t {return op_rla(); };
	instruction_table[0x1F] = [this]()->uint8_t {return op_rra(); };
	instruction_table[0x27] = [this]()->uint8_t {return op_daa(); };
	instruction_table[0x37] = [this]()->uint8_t {return op_scf(); };
	instruction_table[0x3F] = [this]()->uint8_t {return op_ccf(); };

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
			//std::cout << "stubbed at CB instruction: " << std::hex << i << std::endl;
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

uint8_t CPU::op_ld_r16_A() {
	//store value in Register A into byte pointed to by r16
	uint8_t val = (reg.AF >> 8) & 0xFF;

	uint16_t destination = getRegister16((opcode >> 4) & 0x03);

	mmu.write8(destination, val);

	return 8;
}

uint8_t CPU::op_ld_HLI_A() { 
	//store value in register A into the byte pointed by HL and increment HL
	uint8_t val = (reg.AF >> 8) & 0xFF;

	uint16_t destination = this->reg.HL;

	mmu.write8(destination, val);

	reg.HL += 1;


	return 8; 
}

uint8_t CPU::op_ld_HLD_A() { 
	uint8_t val = (reg.AF >> 8) & 0xFF;

	uint16_t destination = this->reg.HL;

	mmu.write8(destination, val);

	reg.HL -= 1;
	return 8; 
}

uint8_t CPU::op_ld_A_r16() {
	uint8_t id = (opcode >> 4) & 0x03;
	uint16_t add = getRegister16(id);

	uint8_t res = mmu.read8(add);

	setRegister(7, res);

	switch (id) {
	case 2:
		reg.HL += 1;
		break;

	case 3:
		reg.HL -= 1;
		break;
	default:
		break;
	}
	return 8;
}

uint8_t CPU::op_ld_n16_SP() {
	//Store SP & $FF at address n16 and SP >> 8 at address n16 + 1.

	uint8_t lo_val = fetch();
	uint8_t hi_val = fetch();

	uint16_t address = (hi_val << 8) | lo_val;  //n16

	mmu.write8(address, (reg.SP & 0xFF));

	mmu.write8(address + 1, (reg.SP >> 8));

	return 20;
}

uint8_t CPU::op_jr_e8() {

	int8_t val = fetch();

	reg.PC += val;

	return 12;
}

uint8_t CPU::op_jr_cc_e8() {

	int8_t val = fetch();

	uint8_t cc = (opcode >> 3) & 0x03; 

	switch (cc) {
	case 0:
		if (!getZeroFlag())
		{
			reg.PC += val;
			return 12;
		}
		else { return 8; }

	case 1:
		if (getZeroFlag())
		{
			reg.PC += val;
			return 12;
		}
		else { return 8; }

	case 2:
		if (!getCarryFlag())
		{
			reg.PC += val;
			return 12;
		}
		else { return 8; }

	case 3:
		if (getCarryFlag())
		{
			reg.PC += val;
			return 12;
		}
		else { return 8; }
	default:
		return 8;
	}
	
}


uint8_t CPU::op_rlca() {
	
	uint8_t val = getRegister(7);


	uint8_t result = (val << 1) | (val >> 7);

	setRegister(7, result);
	
	setCarryFlag((val >> 7) & 0x01);
	setzeroflag(0);
	setHalfCarryFlag(0);
	setSubtractionFlag(0);

	return 4;
}


uint8_t CPU::op_rrca() {
	uint8_t val = getRegister(7);

	uint8_t result = (val >> 1) | (val << 7);
	setRegister(7, result);

	setCarryFlag(val & 0x01);
	setzeroflag(0);
	setHalfCarryFlag(0);
	setSubtractionFlag(0);

	return 4;
}


uint8_t CPU::op_rla(){
	//ion even know what it means to rotate through a flag
	//but whatever lets learn it right here right now

	uint8_t hold = getCarryFlag();
	uint8_t val = getRegister(7);
	uint8_t result = (val << 1) | hold;


	setCarryFlag((val >> 7) & 0x01);
	setRegister(7, result);

	setzeroflag(0);
	setHalfCarryFlag(0);
	setSubtractionFlag(0);

	return 4;
	
}
uint8_t CPU::op_rra(){
	uint8_t hold = getCarryFlag();
	uint8_t val = getRegister(7);

	uint8_t result = (val >> 1) | (hold << 7);

	setCarryFlag(val & 0x01);
	setRegister(7, result);
	setzeroflag(0);
	setHalfCarryFlag(0);
	setSubtractionFlag(0);

	return 4;

}


uint8_t CPU::op_daa(){
	uint8_t offset{ 0 };

	bool shouldCarry = false;

	uint8_t val = getRegister(7);

	uint8_t halfCarry = getHalfCarryFlag();

	uint8_t carry = getCarryFlag();

	uint8_t sub = getSubtractionFlag();
	
	if ((sub == 0 && (val & 0xF) > 0x09) || halfCarry == 1) {
		offset |= 0x06;
	}

	if ((sub == 0 && val > 0x99) || carry == 1) {
		offset |= 0x60;
		shouldCarry = true;
	}

	uint8_t output = (sub == 0) ? val += offset : val -= offset;

	if (output == 0) {
		setzeroflag(1);
	}
	else { setzeroflag(0); }

	setCarryFlag(shouldCarry);

	setHalfCarryFlag(0);
	return 4;
}
uint8_t CPU::op_cpl(){
	uint8_t val = getRegister(7);

	setRegister(7, ~val);

	setSubtractionFlag(1);
	setHalfCarryFlag(1);
	return 4;
}
uint8_t CPU::op_scf(){
	setSubtractionFlag(0);
	setHalfCarryFlag(0);
	setCarryFlag(true);
	return 4;
}
uint8_t CPU::op_ccf(){
	setSubtractionFlag(0);
	setHalfCarryFlag(0);
	setCarryFlag(!getCarryFlag());
	return 4;
}


uint8_t CPU::op_add_r8(){
	
}
uint8_t CPU::op_adc_r8(){}
uint8_t CPU::op_sub_r8(){}
uint8_t CPU::op_sbc_r8(){}
uint8_t CPU::op_and_r8(){}
uint8_t CPU::op_xor_r8(){}
uint8_t CPU::op_or_r8() {}
uint8_t CPU::op_cp_r8() {}