#include <stdio.h>
#include <assert.h>
#include "mvm.h"

#ifndef MVM_PRINTF
#define MVM_PRINTF(format, ...) printf(format, __VA_ARGS__)
#endif

const char* vm_get_opcode_name(enum OP_CODE opcode) {
    switch (opcode) {
    #define KEYWORD(ENUM, NAME, TYPE, CODE) case OP_##ENUM: return NAME;
    #include "lexer.inl"
    default:
        assert(0 && "unreachable");
    }
}

const char* vm_get_reg_name(enum REG_NAME reg) {
    switch (reg) {
        #define REGISTER(ENUM, NAME, CODE) case REG_##ENUM: return NAME;
        #include "lexer.inl"
    default:
        assert(0 && "Unknown register");
    }
}

static void _syscall(MVM *vm) {
    switch(vm->regs[REG_V0]) {
    case 0:
        MVM_PRINTF("%d\n", vm->regs[0]);
    }
}

static void _execute_rtype(MVM *vm, uint32_t instruction) {
    enum OP_CODE funct = instruction & 0x3F;
    int rs = (instruction >> 21) & 0x1F;
    int rt = (instruction >> 16) & 0x1F;
    int rd = (instruction >> 11) & 0x1F;

    switch (funct) {
    case OP_ADD:
    case OP_ADDU:
        vm->regs[rd] = vm->regs[rs] + vm->regs[rt]; return;
    case OP_SUB:
    case OP_SUBU:
        vm->regs[rd] = vm->regs[rs] - vm->regs[rt]; return;
    case OP_SYSCALL:
        _syscall(vm); return;
    default:
        assert(0 && "Uknown instruction for rtype");
    }
}

static void _execute_itype(MVM *vm, uint32_t instruction) {
    enum OP_CODE opcode = instruction >> 26 & 0x3F;
    int rs = (instruction >> 21) & 0x1F;
    int rt = (instruction >> 16) & 0x1F;
    int imm = instruction & 0xFFFF;

    switch (opcode) {
    case OP_ADDI:
    case OP_ADDIU:
        vm->regs[rt] = vm->regs[rs] + imm; return;
    default:
        assert(0 && "Unknown instruction for itype");
    }
}

static void _execute_jtype(MVM *vm, uint32_t instruction) {
    (void)vm;
    (void)instruction;
    assert(0 && "todo");
}

void mvm_execute_many(MVM *vm, uint32_t *instructions, size_t num_instructions) {
    while (vm->PC < num_instructions) {
        uint32_t instruction = instructions[vm->PC];
        int opcode = (instruction >> 26) & 0x3F;

        switch (opcode) {
        case 0b000000:
            _execute_rtype(vm, instruction); break;
        case OP_ADDI:
        case OP_ADDIU:
            _execute_itype(vm, instruction); break;
        case OP_J:
        case OP_JAL:
            _execute_jtype(vm, instruction); break;
        default:
            assert(0 && "Unknown instruction");
        }

        vm->PC += 1;
    }
}

void mvm_execute_one(MVM *vm, uint32_t instruction) {
    int opcode = (instruction >> 26) & 0x3F;

    switch (opcode) {
    case 0b000000:
        _execute_rtype(vm, instruction); return;
    case OP_ADDI:
    case OP_ADDIU:
        _execute_itype(vm, instruction); return;
    case OP_J:
    case OP_JAL:
        _execute_jtype(vm, instruction); return;
    default:
        assert(0 && "Unknown instruction");
    }

    vm->PC += 1;
}
