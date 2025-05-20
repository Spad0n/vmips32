#ifndef MVM_H
#define MVM_H

#include <stdint.h>

#define NUM_REGS 32
#define MEM_SIZE 1024

typedef struct {
    uint32_t regs[NUM_REGS];
    uint32_t PC;
    uint32_t memory[MEM_SIZE];
} MVM;

enum REG_NAME {
    #define REGISTER(ENUM, NAME, NUMBER) REG_##ENUM = NUMBER,
    #include "lexer.inl"
};

enum OP_CODE {
    #define INSTRUCTION(ENUM, NAME, TYPE, CODE) OP_##ENUM = CODE,
    #include "lexer.inl"
};

const char* mvm_get_opcode_name(enum OP_CODE opcode);

const char* mvm_get_reg_name(enum REG_NAME reg);

void mvm_execute_one(MVM *vm, uint32_t instruction);

#endif // MVM_H
