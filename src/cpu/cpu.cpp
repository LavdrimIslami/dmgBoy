

/*todolist:

Registers struct: AF, BC, DE, HL, SP, PC — plus F flag helpers (Z, N, H, C)

CPU::step() → fetch opcode at PC → decode → execute → return cycles

Implement the ~244 unprefixed opcodes (LD, INC, DEC, ADD, JP, CALL, RET…)

Implement the 0xCB prefix table (256 bit-rotation/test ops)

Implementation tip: use a function pointer table indexed by opcode byte
Flag math: H flag = carry out of bit 3; C flag = carry out of bit 7
Store flags in the F register high nibble: bit7=Z, 6=N, 5=H, 4=C

*/

