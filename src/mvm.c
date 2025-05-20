#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mvm.h"

const char* vm_get_opcode_name(OP_CODE opcode) {
    switch (opcode) {
    #define INSTRUCTION(ENUM, NAME, TYPE, CODE) case OP_##ENUM: return NAME;
    #include "lexer.inl"
    default:
        assert(0 && "unreachable");
    }
}

const char* vm_get_reg_name(REG_NAME reg) {
    switch (reg) {
        #define REGISTER(ENUM, NAME, CODE) case REG_##ENUM: return NAME;
        #include "lexer.inl"
    default:
        assert(0 && "Unknown register");
    }
}

static void _syscall(MVM *vm) {
    switch(vm->regs[REG_V0]) {
    case 1:  printf("%d", vm->regs[REG_A0]); return;
    case 10: exit(vm->regs[REG_A0]);
    case 11: printf("%c", vm->regs[REG_A0]); return;
    default:
        assert(0 && "TODO");
    }
}

static void _execute_rtype(MVM *vm, uint32_t instruction) {
    OP_CODE funct = instruction & 0x3F;
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
    case OP_SLT:
        vm->regs[rd] = vm->regs[rs] < vm->regs[rt] ? 1 : 0; return;
    case OP_SYSCALL:
        _syscall(vm); return;
    default:
        assert(0 && "Uknown instruction for rtype");
    }
}

static void _execute_itype(MVM *vm, uint32_t instruction) {
    OP_CODE opcode = instruction >> 26 & 0x3F;
    int rs = (instruction >> 21) & 0x1F;
    int rt = (instruction >> 16) & 0x1F;
    int16_t imm = instruction & 0xFFFF;

    switch (opcode) {
    case OP_ADDI:
    case OP_ADDIU:
        vm->regs[rt] = vm->regs[rs] + imm; return;
    case OP_BNE:
        if (vm->regs[rs] != vm->regs[rt]) {
            vm->PC += (imm << 2);
        } return;
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
        case OP_BNE:
            _execute_itype(vm, instruction); break;
        case OP_J:
        case OP_JAL:
            _execute_jtype(vm, instruction); break;
        default:
            assert(0 && "Unknown instruction");
        }

        vm->PC += 4;
    }
}

void mvm_execute_one(MVM *vm, uint32_t instruction) {
    int opcode = (instruction >> 26) & 0x3F;

    switch (opcode) {
    case 0b000000:
        _execute_rtype(vm, instruction); break;
    case OP_ADDI:
    case OP_ADDIU:
    case OP_BNE:
        _execute_itype(vm, instruction); break;
    case OP_J:
    case OP_JAL:
        _execute_jtype(vm, instruction); break;
    default:
        fprintf(stderr, "unknown instruction: %s\n", vm_get_opcode_name(opcode));
        exit(1);
        //assert(0 && "Unknown instruction");
    }

    vm->PC += 4;
}

MVM mvm_init() {
    MVM mvm = {0};
    mvm.PC = 0x00400000;
    return mvm;
}
