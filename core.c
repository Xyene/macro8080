#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <termio.h>
#endif

#ifdef CPM
#include "memory_cputest.h"
// #include "memory_8080exer.h"
#else
#include "memory_basic.h"
#endif

#define d8 memory[PC++]
#define a16 (d8 | (d8 << 8))
#define d16 a16

#define DONE goto done_opcode;
#include "opcode_defs.h"

void run_forever(void) {
  typedef union {
    uint16_t pair;
    uint8_t reg[2];
  } regpair_t;

  uint8_t A;
  regpair_t bc, de, hl;

#define B bc.reg[1]
#define C bc.reg[0]
#define BC bc.pair
#define D de.reg[1]
#define E de.reg[0]
#define DE de.pair
#define H hl.reg[1]
#define L hl.reg[0]
#define HL hl.pair
#define M memory[HL]

  uint8_t F_S, F_Z, F_P, F_C, F_A;

#define F                                                                \
  ((uint8_t)(!!F_C | (1 << 1) | (!!F_P << 2) | (0 << 3) | (!!F_A << 4) | \
             (0 << 5) | (!!F_Z << 6) | (!!F_S << 7)))

  uint16_t PC, SP;
  uint8_t INTE;

  uint16_t temp16, temp16_s;
  uint8_t temp8, temp8_2;

  BC = DE = HL = 0;
  A = 0;
  F_S = F_Z = F_P = F_C = F_A = 0;
  SP = 0;
  INTE = 0;

#ifdef CPM
  PC = 0x100;
  memory[5] = 0xC9;
#else
  PC = 0;
#endif

  static void* ops[] = {
      &&NOP,      &&LXI_BC_d16, &&STAX_B,     &&INX_BC,     &&INR_B,
      &&DCR_B,    &&MVI_B_d8,   &&RLC,        &&NOP,        &&DAD_BC,
      &&LDAX_B,   &&DCX_BC,     &&INR_C,      &&DCR_C,      &&MVI_C_d8,
      &&RRC,      &&NOP,        &&LXI_DE_d16, &&STAX_D,     &&INX_DE,
      &&INR_D,    &&DCR_D,      &&MVI_D_d8,   &&RAL,        &&NOP,
      &&DAD_DE,   &&LDAX_D,     &&DCX_DE,     &&INR_E,      &&DCR_E,
      &&MVI_E_d8, &&RAR,        &&NOP,        &&LXI_HL_d16, &&SHLD_a16,
      &&INX_HL,   &&INR_H,      &&DCR_H,      &&MVI_H_d8,   &&DAA,
      &&NOP,      &&DAD_HL,     &&LHLD_a16,   &&DCX_HL,     &&INR_L,
      &&DCR_L,    &&MVI_L_d8,   &&CMA,        &&NOP,        &&LXI_SP_d16,
      &&STA_a16,  &&INX_SP,     &&INR_M,      &&DCR_M,      &&MVI_M_d8,
      &&STC,      &&NOP,        &&DAD_SP,     &&LDA_a16,    &&DCX_SP,
      &&INR_A,    &&DCR_A,      &&MVI_A_d8,   &&CMC,        &&MOV_B_B,
      &&MOV_B_C,  &&MOV_B_D,    &&MOV_B_E,    &&MOV_B_H,    &&MOV_B_L,
      &&MOV_B_M,  &&MOV_B_A,    &&MOV_C_B,    &&MOV_C_C,    &&MOV_C_D,
      &&MOV_C_E,  &&MOV_C_H,    &&MOV_C_L,    &&MOV_C_M,    &&MOV_C_A,
      &&MOV_D_B,  &&MOV_D_C,    &&MOV_D_D,    &&MOV_D_E,    &&MOV_D_H,
      &&MOV_D_L,  &&MOV_D_M,    &&MOV_D_A,    &&MOV_E_B,    &&MOV_E_C,
      &&MOV_E_D,  &&MOV_E_E,    &&MOV_E_H,    &&MOV_E_L,    &&MOV_E_M,
      &&MOV_E_A,  &&MOV_H_B,    &&MOV_H_C,    &&MOV_H_D,    &&MOV_H_E,
      &&MOV_H_H,  &&MOV_H_L,    &&MOV_H_M,    &&MOV_H_A,    &&MOV_L_B,
      &&MOV_L_C,  &&MOV_L_D,    &&MOV_L_E,    &&MOV_L_H,    &&MOV_L_L,
      &&MOV_L_M,  &&MOV_L_A,    &&MOV_M_B,    &&MOV_M_C,    &&MOV_M_D,
      &&MOV_M_E,  &&MOV_M_H,    &&MOV_M_L,    &&HLT,        &&MOV_M_A,
      &&MOV_A_B,  &&MOV_A_C,    &&MOV_A_D,    &&MOV_A_E,    &&MOV_A_H,
      &&MOV_A_L,  &&MOV_A_M,    &&MOV_A_A,    &&ADD_B,      &&ADD_C,
      &&ADD_D,    &&ADD_E,      &&ADD_H,      &&ADD_L,      &&ADD_M,
      &&ADD_A,    &&ADC_B,      &&ADC_C,      &&ADC_D,      &&ADC_E,
      &&ADC_H,    &&ADC_L,      &&ADC_M,      &&ADC_A,      &&SUB_B,
      &&SUB_C,    &&SUB_D,      &&SUB_E,      &&SUB_H,      &&SUB_L,
      &&SUB_M,    &&SUB_A,      &&SBB_B,      &&SBB_C,      &&SBB_D,
      &&SBB_E,    &&SBB_H,      &&SBB_L,      &&SBB_M,      &&SBB_A,
      &&ANA_B,    &&ANA_C,      &&ANA_D,      &&ANA_E,      &&ANA_H,
      &&ANA_L,    &&ANA_M,      &&ANA_A,      &&XRA_B,      &&XRA_C,
      &&XRA_D,    &&XRA_E,      &&XRA_H,      &&XRA_L,      &&XRA_M,
      &&XRA_A,    &&ORA_B,      &&ORA_C,      &&ORA_D,      &&ORA_E,
      &&ORA_H,    &&ORA_L,      &&ORA_M,      &&ORA_A,      &&CMP_B,
      &&CMP_C,    &&CMP_D,      &&CMP_E,      &&CMP_H,      &&CMP_L,
      &&CMP_M,    &&CMP_A,      &&RNZ,        &&POP_BC,     &&JNZ_a16,
      &&JMP_a16,  &&CNZ_a16,    &&PUSH_BC,    &&ADI_d8,     &&RST_0,
      &&RZ,       &&RET,        &&JZ_a16,     &&JMP_a16,    &&CZ_a16,
      &&CALL_a16, &&ACI_d8,     &&RST_1,      &&RNC,        &&POP_DE,
      &&JNC_a16,  &&OUT_d8,     &&CNC_a16,    &&PUSH_DE,    &&SUI_d8,
      &&RST_2,    &&RC,         &&RET,        &&JC_a16,     &&IN_d8,
      &&CC_a16,   &&CALL_a16,   &&SBI_d8,     &&RST_3,      &&RPO,
      &&POP_HL,   &&JPO_a16,    &&XTHL,       &&CPO_a16,    &&PUSH_HL,
      &&ANI_d8,   &&RST_4,      &&RPE,        &&PCHL,       &&JPE_a16,
      &&XCHG,     &&CPE_a16,    &&CALL_a16,   &&XRI_d8,     &&RST_5,
      &&RP,       &&POP_AF,     &&JP_a16,     &&DI,         &&CP_a16,
      &&PUSH_AF,  &&ORI_d8,     &&RST_6,      &&RM,         &&SPHL,
      &&JM_a16,   &&EI,         &&CM_a16,     &&CALL_a16,   &&CPI_d8,
      &&RST_7};

  int cycle = 0;
  do {
    cycle++;

#ifdef CPM
    if (PC == 0x05) {
      switch (C) {
        case 2:  // C_WRITE
          if (E) putchar((char)E);
          break;
        case 9:  // C_WRITESTR
          for (uint16_t addr = (D << 8) | E; memory[addr] != '$'; addr++) {
            if (memory[addr]) putchar((char)memory[addr]);
          }
          break;
      }
    } else if (PC == 0x00) {
      printf("\nCP/M exit\n");
      return;
    }
#endif

    uint8_t opcode = memory[PC++];
    // printf("PC:%04X opcode:%02X SP:%04X A:%02X B:%02X C:%02X D:%02X E:%02X
    // H:%02X L:%02X flags:%02X INTE:%01X\n", PC, opcode, SP, A, B, C, D, E, H,
    // L, F, INTE);
    goto* ops[opcode];
  done_opcode:;
  } while (1);

  // What follows is machine-generated code.
NOP:
  NOP()
LXI_BC_d16:
  LXI(BC, d16)
STAX_B:
  STAX(BC)
INX_BC:
  INX(BC)
INR_B:
  INR(B)
DCR_B:
  DCR(B)
MVI_B_d8:
  MVI(B, d8)
RLC:
  RLC()
DAD_BC:
  DAD(BC)
LDAX_B:
  LDAX(BC)
DCX_BC:
  DCX(BC)
INR_C:
  INR(C)
DCR_C:
  DCR(C)
MVI_C_d8:
  MVI(C, d8)
RRC:
  RRC()
LXI_DE_d16:
  LXI(DE, d16)
STAX_D:
  STAX(DE)
INX_DE:
  INX(DE)
INR_D:
  INR(D)
DCR_D:
  DCR(D)
MVI_D_d8:
  MVI(D, d8)
RAL:
  RAL()
DAD_DE:
  DAD(DE)
LDAX_D:
  LDAX(DE)
DCX_DE:
  DCX(DE)
INR_E:
  INR(E)
DCR_E:
  DCR(E)
MVI_E_d8:
  MVI(E, d8)
RAR:
  RAR()
LXI_HL_d16:
  LXI(HL, d16)
SHLD_a16:
  SHLD(a16)
INX_HL:
  INX(HL)
INR_H:
  INR(H)
DCR_H:
  DCR(H)
MVI_H_d8:
  MVI(H, d8)
DAA:
  DAA()
DAD_HL:
  DAD(HL)
LHLD_a16:
  LHLD(a16)
DCX_HL:
  DCX(HL)
INR_L:
  INR(L)
DCR_L:
  DCR(L)
MVI_L_d8:
  MVI(L, d8)
CMA:
  CMA()
LXI_SP_d16:
  LXI(SP, d16)
STA_a16:
  STA(a16)
INX_SP:
  INX(SP)
INR_M:
  INR(M)
DCR_M:
  DCR(M)
MVI_M_d8:
  MVI(M, d8)
STC:
  STC()
DAD_SP:
  DAD(SP)
LDA_a16:
  LDA(a16)
DCX_SP:
  DCX(SP)
INR_A:
  INR(A)
DCR_A:
  DCR(A)
MVI_A_d8:
  MVI(A, d8)
CMC:
  CMC()
MOV_B_B:
  MOV(B, B)
MOV_B_C:
  MOV(B, C)
MOV_B_D:
  MOV(B, D)
MOV_B_E:
  MOV(B, E)
MOV_B_H:
  MOV(B, H)
MOV_B_L:
  MOV(B, L)
MOV_B_M:
  MOV(B, M)
MOV_B_A:
  MOV(B, A)
MOV_C_B:
  MOV(C, B)
MOV_C_C:
  MOV(C, C)
MOV_C_D:
  MOV(C, D)
MOV_C_E:
  MOV(C, E)
MOV_C_H:
  MOV(C, H)
MOV_C_L:
  MOV(C, L)
MOV_C_M:
  MOV(C, M)
MOV_C_A:
  MOV(C, A)
MOV_D_B:
  MOV(D, B)
MOV_D_C:
  MOV(D, C)
MOV_D_D:
  MOV(D, D)
MOV_D_E:
  MOV(D, E)
MOV_D_H:
  MOV(D, H)
MOV_D_L:
  MOV(D, L)
MOV_D_M:
  MOV(D, M)
MOV_D_A:
  MOV(D, A)
MOV_E_B:
  MOV(E, B)
MOV_E_C:
  MOV(E, C)
MOV_E_D:
  MOV(E, D)
MOV_E_E:
  MOV(E, E)
MOV_E_H:
  MOV(E, H)
MOV_E_L:
  MOV(E, L)
MOV_E_M:
  MOV(E, M)
MOV_E_A:
  MOV(E, A)
MOV_H_B:
  MOV(H, B)
MOV_H_C:
  MOV(H, C)
MOV_H_D:
  MOV(H, D)
MOV_H_E:
  MOV(H, E)
MOV_H_H:
  MOV(H, H)
MOV_H_L:
  MOV(H, L)
MOV_H_M:
  MOV(H, M)
MOV_H_A:
  MOV(H, A)
MOV_L_B:
  MOV(L, B)
MOV_L_C:
  MOV(L, C)
MOV_L_D:
  MOV(L, D)
MOV_L_E:
  MOV(L, E)
MOV_L_H:
  MOV(L, H)
MOV_L_L:
  MOV(L, L)
MOV_L_M:
  MOV(L, M)
MOV_L_A:
  MOV(L, A)
MOV_M_B:
  MOV(M, B)
MOV_M_C:
  MOV(M, C)
MOV_M_D:
  MOV(M, D)
MOV_M_E:
  MOV(M, E)
MOV_M_H:
  MOV(M, H)
MOV_M_L:
  MOV(M, L)
HLT:
  HLT()
MOV_M_A:
  MOV(M, A)
MOV_A_B:
  MOV(A, B)
MOV_A_C:
  MOV(A, C)
MOV_A_D:
  MOV(A, D)
MOV_A_E:
  MOV(A, E)
MOV_A_H:
  MOV(A, H)
MOV_A_L:
  MOV(A, L)
MOV_A_M:
  MOV(A, M)
MOV_A_A:
  MOV(A, A)
ADD_B:
  ADD(B)
ADD_C:
  ADD(C)
ADD_D:
  ADD(D)
ADD_E:
  ADD(E)
ADD_H:
  ADD(H)
ADD_L:
  ADD(L)
ADD_M:
  ADD(M)
ADD_A:
  ADD(A)
ADC_B:
  ADC(B)
ADC_C:
  ADC(C)
ADC_D:
  ADC(D)
ADC_E:
  ADC(E)
ADC_H:
  ADC(H)
ADC_L:
  ADC(L)
ADC_M:
  ADC(M)
ADC_A:
  ADC(A)
SUB_B:
  SUB(B)
SUB_C:
  SUB(C)
SUB_D:
  SUB(D)
SUB_E:
  SUB(E)
SUB_H:
  SUB(H)
SUB_L:
  SUB(L)
SUB_M:
  SUB(M)
SUB_A:
  SUB(A)
SBB_B:
  SBB(B)
SBB_C:
  SBB(C)
SBB_D:
  SBB(D)
SBB_E:
  SBB(E)
SBB_H:
  SBB(H)
SBB_L:
  SBB(L)
SBB_M:
  SBB(M)
SBB_A:
  SBB(A)
ANA_B:
  ANA(B)
ANA_C:
  ANA(C)
ANA_D:
  ANA(D)
ANA_E:
  ANA(E)
ANA_H:
  ANA(H)
ANA_L:
  ANA(L)
ANA_M:
  ANA(M)
ANA_A:
  ANA(A)
XRA_B:
  XRA(B)
XRA_C:
  XRA(C)
XRA_D:
  XRA(D)
XRA_E:
  XRA(E)
XRA_H:
  XRA(H)
XRA_L:
  XRA(L)
XRA_M:
  XRA(M)
XRA_A:
  XRA(A)
ORA_B:
  ORA(B)
ORA_C:
  ORA(C)
ORA_D:
  ORA(D)
ORA_E:
  ORA(E)
ORA_H:
  ORA(H)
ORA_L:
  ORA(L)
ORA_M:
  ORA(M)
ORA_A:
  ORA(A)
CMP_B:
  CMP(B)
CMP_C:
  CMP(C)
CMP_D:
  CMP(D)
CMP_E:
  CMP(E)
CMP_H:
  CMP(H)
CMP_L:
  CMP(L)
CMP_M:
  CMP(M)
CMP_A:
  CMP(A)
RNZ:
  RNZ()
POP_BC:
  POP(BC)
JNZ_a16:
  JNZ(a16)
JMP_a16:
  JMP(a16)
CNZ_a16:
  CNZ(a16)
PUSH_BC:
  PUSH(BC)
ADI_d8:
  ADI(d8)
RST_0:
  RST(0)
RZ:
  RZ()
RET:
  RET()
JZ_a16:
  JZ(a16)
CZ_a16:
  CZ(a16)
CALL_a16:
  CALL(a16)
ACI_d8:
  ACI(d8)
RST_1:
  RST(1)
RNC:
  RNC()
POP_DE:
  POP(DE)
JNC_a16:
  JNC(a16)
OUT_d8:
  OUT(d8)
CNC_a16:
  CNC(a16)
PUSH_DE:
  PUSH(DE)
SUI_d8:
  SUI(d8)
RST_2:
  RST(2)
RC:
  RC()
JC_a16:
  JC(a16)
IN_d8:
  IN(d8)
CC_a16:
  CC(a16)
SBI_d8:
  SBI(d8)
RST_3:
  RST(3)
RPO:
  RPO()
POP_HL:
  POP(HL)
JPO_a16:
  JPO(a16)
XTHL:
  XTHL()
CPO_a16:
  CPO(a16)
PUSH_HL:
  PUSH(HL)
ANI_d8:
  ANI(d8)
RST_4:
  RST(4)
RPE:
  RPE()
PCHL:
  PCHL()
JPE_a16:
  JPE(a16)
XCHG:
  XCHG()
CPE_a16:
  CPE(a16)
XRI_d8:
  XRI(d8)
RST_5:
  RST(5)
RP:
  RP()
POP_AF:
  POP_PSW()
JP_a16:
  JP(a16)
DI:
  DI()
CP_a16:
  CP(a16)
PUSH_AF:
  PUSH_PSW()
ORI_d8:
  ORI(d8)
RST_6:
  RST(6)
RM:
  RM()
SPHL:
  SPHL()
JM_a16:
  JM(a16)
EI:
  EI()
CM_a16:
  CM(a16)
CPI_d8:
  CPI(d8)
RST_7:
  RST(7)
}

#ifndef __unix__
void initialize_interrupts(void);
#endif

int main(void) {
#ifdef __unix__
  struct termios term;

  if (tcgetattr(fileno(stdin), &term)) {
    return 1;
  }

  term.c_lflag &= ~(ECHO | ICANON);
  if (tcsetattr(fileno(stdin), TCSAFLUSH, &term)) {
    return 1;
  }
#endif

#ifndef __unix__
  initialize_interrupts();
#endif

  terminal_clear();
  run_forever();
  return 0;
}
