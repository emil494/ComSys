#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>
#include <stdlib.h>

#define LUI     0x37
#define AUIPC   0x17
#define JAL     0x6f
#define JALR    0x67
#define ECALL   0x73

#define BRANCH  0x63
  #define BEQ     0x0
  #define BNE     0x1
  #define BLT     0x4
  #define BGE     0x5
  #define BLTU    0x6
  #define BGEU    0x7

#define LOAD    0x3
  #define LB      0x0
  #define LH      0x1
  #define LW      0x2
  #define LBU     0x4
  #define LHU     0x5

#define STORE   0x23
  #define SB      0x0
  #define SH      0x1
  #define SW      0x2

#define OP_IMM  0x13
  #define ADDI    0x0
  #define SLLI    0x1
  #define SLTI    0x2
  #define SLTIU   0x3
  #define XORI    0x4
  #define SRI     0x5
    #define SRLI    0x0
    #define SRAI    0x20
  #define ORI     0x6
  #define ANDI    0x7

#define OP      0x33
  #define ADUB    0x0
    #define ADD     0x0
    #define SUB     0x20
  #define SLL     0x1
  #define SLT     0x2
  #define SLTU    0x3
  #define XOR     0x4
  #define SR      0x5
    #define SRL     0x0
    #define SRA     0x20
  #define OR      0x6
  #define AND     0x7

//Check funct7 first, for simpler switch statement, as opcode for RV32M is same as OP
#define RVM     0x1 
  #define MUL     0x0
  #define MULH    0x1
  #define MULHSU  0x2
  #define MULHU   0x3
  #define DIV     0x4
  #define DIVU    0x5
  #define REM     0x6
  #define REMU    0x7

typedef struct register_state {
  u_int32_t regs[32];
  u_int32_t pc;
} register_state_t;

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
