#pragma once
#include "..//memory/mmu.h"
#include <functional>
#include <array>


class CPU {
private:
	MMU& mmu;

	struct registers
	{
		uint16_t AF; //accumulator and flags HI: A
		uint16_t BC; // HI: B LO: C
		uint16_t DE; // HI: D LO: E
		uint16_t HL; //HI: H LO: L
		uint16_t SP; //stack pointer
		uint16_t PC; //program 
	};
		
	//f flag helpers are z(7 zero n 6 sub h 5 half carry c 4 carry
	const uint16_t Z = 0x0080;
	const uint16_t N = 0x0040;
	const uint16_t H = 0x0020;
	const uint16_t C = 0x0010;


	registers reg;

	//jump table array of function pointerss
	std::array<std::function<uint8_t()>, 256> instruction_table;

	std::array<std::function<uint8_t()>, 256> CB_table;

	uint8_t opcode;


public:
	CPU(MMU& mmu);

	uint16_t getZeroFlag();
	void setzeroflag(bool state);

	uint16_t getSubtractionFlag();
	void setSubtractionFlag(bool state);

	uint16_t getHalfCarryFlag();
	void setHalfCarryFlag(bool state);

	uint16_t getCarryFlag();
	void setCarryFlag(bool state);


	uint8_t step();
	uint8_t fetch();

	uint8_t getRegister(uint8_t id);
	uint16_t getRegister16(uint16_t id);
	void setRegister(uint8_t id, uint8_t val);
	void setRegister16(uint8_t id, uint16_t val);

	//block 0
	uint8_t op_nop();
	uint8_t op_ld_r8_n8();
	uint8_t op_ld_r16_n16();
	uint8_t op_inc_r8();
	uint8_t op_dec_r8();
	uint8_t op_inc_r16();
	uint8_t op_dec_r16();
	

	//block 1
	uint8_t op_ld_r8_r8();
	uint8_t op_halt();




	void fillStub();

};  