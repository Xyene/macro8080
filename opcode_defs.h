#include "terminal.h"

#define assert(_) 	return;

static uint8_t parity_table[] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

#define F_SZP(X) \
	F_S = X & 0x80; \
	F_Z = X == 0; \
	F_P = parity_table[X];

#define PUSH_STACK_8(val) \
	memory[--SP] = val; \

#define PUSH_STACK_16(val) \
	PUSH_STACK_8((val) >> 8); \
	PUSH_STACK_8((val) & 0xFF);

#define POP_STACK_8() \
	(memory[SP++])

#define POP_STACK_16() \
	(((uint16_t) POP_STACK_8()) | (((uint16_t) POP_STACK_8()) << 8))

#define NOP() \
	; \
DONE

#define LXI(WX, d16) \
    WX = d16; \
DONE

#define STAX(WX) \
    memory[WX] = A; \
DONE

#define INX(WX) \
    WX++; \
DONE

#define INR(X) \
  F_A = (X & 0xF) == 0xF; \
	X++; \
	F_SZP(X); \
DONE

#define DCR(X) \
  F_A = (X & 0xF) != 0; \
	X--; \
	F_SZP(X); \
DONE

#define MVI(X, d8) \
    X = d8; \
DONE

#define RLC() \
   temp8 = A & 0x80; \
   A <<= 1; \
   A |= !!temp8; \
   F_C = temp8; \
DONE

#define DAD(WX) \
	F_C = !!(((((uint32_t) HL) & 0xFFFF) + WX) & 0x10000); \
  HL += WX; \
DONE

#define LDAX(WX) \
    A = memory[WX]; \
DONE

#define DCX(WX) \
    WX--; \
DONE

#define RRC() \
    temp8 = A & 0x1; \
	A >>= 1; \
	A |= temp8 << 7; \
	F_C = temp8; \
DONE

#define RAL() \
  temp8 = !!F_C; \
	F_C = A & 0x80; \
	A <<= 1; \
	A |= temp8; \
DONE

#define RAR() \
  temp8 = !!F_C; \
	F_C = A & 0x1; \
	A >>= 1; \
	A |= temp8 << 7; \
DONE

#define SHLD(a16) \
	temp16 = a16; \
  memory[temp16] = L; \
	memory[temp16 + 1] = H; \
DONE

#define LHLD(a16) \
	temp16 = a16; \
  L = memory[temp16]; \
	H = memory[temp16 + 1]; \
DONE

#define CMA() \
    A = ~A; \
DONE

#define STA(a16) \
    memory[a16] = A; \
DONE

#define STC() \
    F_C = 1; \
DONE

#define LDA(a16) \
	A = memory[a16]; \
DONE

#define CMC() \
    F_C = !F_C; \
DONE

#define MOV(X, Y) \
    X = Y; \
DONE

#define HLT() \
    printf("Processor halted.\n"); \
    return; \
DONE

#define DO_ADD(what, by, carry) \
{ \
	temp8 = by; \
	temp16 = what + temp8 + carry; \
	F_C = !!(temp16 & 0x100); \
	F_A = ((what & 0xF) + (temp8 & 0xF) + carry) & 0x10; \
	temp8 = temp16; \
	F_SZP(temp8); \
}

#define DO_SUB(minu, subt, borrow) \
  DO_ADD(minu, (~subt) & 0xFF, !borrow) \
  F_C = !F_C;

#define DAA() \
{ \
  uint add = 0; \
  if (((A & 0xF) > 9) || !!F_A) { \
    add |= 0x06; \
  } \
  int carry = !!F_C; \
  if (((A & 0xF0) > 0x90) || \
      (((A & 0xF0) >= 0x90) && ((A & 0xF) > 9)) || !!F_C) { \
    add |= 0x60; \
    carry = 1; \
  } \
  DO_ADD(A, add, 0); \
  A = temp8; \
  F_C = carry; \
} \
DONE

#define ADD(X) \
	DO_ADD(A, X, 0) \
	A = temp8; \
DONE

#define ADC(X) \
  DO_ADD(A, X, !!F_C) \
	A = temp8; \
DONE

#define SUB(X) \
	DO_SUB(A, X, 0) \
	A = temp8; \
DONE

#define SBB(X) \
  DO_SUB(A, X, !!F_C) \
	A = temp8; \
DONE

#define ANA(X) \
  F_A = (X | A) & 0x08; \
	A &= X; \
	F_C = 0; \
	F_SZP(A); \
DONE

#define XRA(X) \
  A ^= X; \
	F_C = F_A = 0; \
	F_SZP(A); \
DONE

#define ORA(X) \
    A |= X; \
	F_A = F_C = 0; \
	F_SZP(A); \
DONE

#define CMP(X) \
	DO_SUB(A, X, 0) \
DONE

#define RNZ() \
    if (!F_Z) { PC = POP_STACK_16(); } \
DONE

#define POP(WX) \
    WX = POP_STACK_16(); \
DONE

#define POP_PSW() \
	temp8 = POP_STACK_8(); \
	F_C = temp8 & (1 << 0); \
	F_P = temp8 & (1 << 2); \
	F_A = temp8 & (1 << 4); \
	F_Z = temp8 & (1 << 6); \
	F_S = temp8 & (1 << 7); \
	A = POP_STACK_8(); \
DONE

#define JNZ(a16) \
    PC = !F_Z ? a16 : PC + 2; \
DONE

#define JMP(a16) \
    PC = a16; \
DONE

#define CNZ(a16) \
    if (!F_Z) { CALL(a16) } else { PC += 2; } \
DONE

#define PUSH(WX) \
    PUSH_STACK_16(WX); \
DONE

#define PUSH_PSW() \
	PUSH_STACK_16(((uint16_t) F) | (((uint16_t) A) << 8)); \
DONE

#define ADI(d8) \
	DO_ADD(A, d8, 0) \
	A = temp8; \
DONE

#define RST(num) \
	PUSH_STACK_16(PC); \
	PC = (num) * 8; \
DONE

#define RZ() \
    if (F_Z) { PC = POP_STACK_16(); } \
DONE

#define RET() \
    PC = POP_STACK_16(); \
DONE

#define JZ(a16) \
    PC = F_Z ? a16 : PC + 2; \
DONE

#define CZ(a16) \
    if (F_Z) { CALL(a16) } else { PC += 2; } \
DONE

#define CALL(a16) \
    PUSH_STACK_16(PC + 2); \
	  PC = a16; \
DONE

#define ACI(d8) \
  DO_ADD(A, d8, !!F_C); \
  A = temp8; \
DONE

#define RNC() \
    if (!F_C) { PC = POP_STACK_16(); } \
DONE

#define JNC(a16) \
    PC = !F_C ? a16 : PC + 2; \
DONE

#define OUT(d8) \
    handle_out(d8, A); \
DONE

#define CNC(a16) \
    if (!F_C) { CALL(a16) } else { PC += 2; } \
DONE

#define SUI(d8) \
	DO_SUB(A, d8, 0); \
	A = temp8; \
DONE

#define RC() \
    if (F_C) { PC = POP_STACK_16(); } \
DONE

#define JC(a16) \
    PC = F_C ? a16 : PC + 2; \
DONE

uint8_t handle_in(uint8_t dev) {
	switch (dev) {
		case 16:
			return has_keyboard_input() ? 0x03 : 0x02;
		case 17: {
			char ret = has_keyboard_input() ? read_keyboard_input() : '\0';
			return ret;
		}
    case 255:
      return 0;
		default:
			printf("Invalid device read %02X\n", dev);
      return 0;
	}
}

#define IN(dev) \
	A = handle_in(dev); \
DONE

#define CC(a16) \
    if (F_C) { CALL(a16) } else { PC += 2; } \
DONE

#define SBI(d8) \
    DO_SUB(A, d8, !!F_C) \
	  A = temp8; \
DONE

#define RPO() \
    if (!F_P) { PC = POP_STACK_16(); } \
DONE

#define JPO(a16) \
    PC = !F_P ? a16 : PC + 2; \
DONE

#define XTHL() \
    temp16 = POP_STACK_16(); \
	PUSH_STACK_16(HL); \
	HL = temp16; \
DONE

#define CPO(a16) \
    if (!F_P) { CALL(a16) } else { PC += 2; } \
DONE

#define ANI(d8) \
	temp8 = d8; \
  F_A = (temp8 | A) & 0x08; \
	A &= temp8; \
	F_C = 0; \
	F_SZP(A); \
DONE

#define RPE() \
    if (F_P) { PC = POP_STACK_16(); } \
DONE

#define PCHL() \
    PC = HL; \
DONE

#define JPE(a16) \
  PC = F_P ? a16 : PC + 2; \
DONE

#define XCHG() \
  temp16 = DE; \
	DE = HL; \
	HL = temp16; \
DONE

#define CPE(X) \
    if (F_P) { CALL(a16) } else { PC += 2; } \
DONE

#define XRI(d8) \
  A ^= d8; \
	F_C = F_A = 0; \
	F_SZP(A); \
DONE

#define RP() \
    if (!F_S) { PC = POP_STACK_16(); } \
DONE

#define JP(a16) \
    PC = !F_S ? a16 : PC + 2; \
DONE

#define DI() \
    INTE = 0; \
DONE

#define CP(a16) \
    if (!F_S) { CALL(a16) } else { PC += 2; } \
DONE

#define ORI(d8) \
	A |= d8; \
	F_A = F_C = 0; \
	F_SZP(A); \
DONE

#define RM() \
    if (F_S) { PC = POP_STACK_16(); } \
DONE

#define SPHL() \
    SP = HL; \
DONE

#define JM(a16) \
    PC = F_S ? a16 : PC + 2; \
DONE

#define EI() \
    INTE = 1; \
DONE

#define CM(a16) \
    if (F_S) { CALL(a16) } else { PC += 2; } \
DONE

#define CPI(d8) \
	DO_SUB(A, d8, 0) \
DONE

