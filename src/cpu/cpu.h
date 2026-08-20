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

	bool IME_SCHEDULE;
	bool IME;
	bool halted = false;


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
	uint8_t op_ld_r16_A();
	uint8_t op_ld_HLI_A();
	uint8_t op_ld_HLD_A();
	uint8_t op_ld_A_r16();
	uint8_t op_ld_n16_SP();
	uint8_t op_jr_e8();
	uint8_t op_jr_cc_e8();

	uint8_t op_rlca();
	uint8_t op_rrca();
	uint8_t op_rla();
	uint8_t op_rra();
	uint8_t op_daa();
	uint8_t op_cpl();
	uint8_t op_scf();
	uint8_t op_ccf();
	

	//block 1
	uint8_t op_ld_r8_r8();
	uint8_t op_halt();

	//block 2
	uint8_t op_add_r8();
	uint8_t op_adc_r8();
	uint8_t op_sub_r8();
	uint8_t op_sbc_r8();
	uint8_t op_and_r8();
	uint8_t op_xor_r8();
	uint8_t op_or_r8();
	uint8_t op_cp_r8();

	//block 3
	uint8_t op_add_n8();
	uint8_t op_adc_n8();
	uint8_t op_sub_n8();
	uint8_t op_sbc_n8();
	uint8_t op_and_n8();
	uint8_t op_xor_n8();
	uint8_t op_or_n8();
	uint8_t op_cp_n8();

	uint8_t op_ret_cc();
	uint8_t op_ret();
	uint8_t op_reti();
	uint8_t op_jp_cc_n16();
	uint8_t op_jp_n16();
	uint8_t op_jp_HL();
	uint8_t op_call_cc_n16();
	uint8_t op_call_n16();
	uint8_t op_rst();

	uint8_t op_pop_r16();
	uint8_t op_pop_af();
	uint8_t op_push_r16();
	uint8_t op_push_af();
	uint8_t op_ldh_C_A();
	uint8_t op_ldh_n8_A();
	uint8_t op_ld_n16_A();
	uint8_t op_ldh_A_C();
	uint8_t op_ldh_A_n8();
	uint8_t op_ld_A_n16();
	uint8_t op_add_SP_e8();
	uint8_t op_add_HL_r16();
	uint8_t op_ld_HL_SP_e8();
	uint8_t op_ld_SP_HL();
	uint8_t op_di();
	uint8_t op_ei();


	//cb table
	uint8_t op_rlc_r8();
	uint8_t op_rrc_r8();
	uint8_t op_rl_r8();
	uint8_t op_rr_r8();
	uint8_t op_sla_r8();
	uint8_t op_sra_r8();
	uint8_t op_swap_r8();
	uint8_t op_srl_r8();
	uint8_t op_bit_b_r8();
	uint8_t op_res_b_r8();
	uint8_t op_set_b_r8();


	void fillStub();

};  