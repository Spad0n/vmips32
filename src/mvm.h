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

typedef enum {
    #define REGISTER(ENUM, NAME, NUMBER) REG_##ENUM = NUMBER,
    #include "lexer.inl"
} REG_NAME;

typedef enum {
    #define INSTRUCTION(ENUM, NAME, TYPE, CODE) OP_##ENUM = CODE,
    #include "lexer.inl"
} OP_CODE;

const char* mvm_get_opcode_name(OP_CODE opcode);

const char* mvm_get_reg_name(REG_NAME reg);

void mvm_execute_many(MVM *vm, uint32_t *instructions, size_t num_instructions);

void mvm_execute_one(MVM *vm, uint32_t instruction);

MVM mvm_init();

#endif // MVM_H
