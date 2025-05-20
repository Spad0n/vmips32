#ifndef SYMBOL
#define SYMBOL(...)
#endif

#ifndef INSTRUCTION
#define INSTRUCTION(...)
#endif

#ifndef PSEUDOINSTRUCTION
#define PSEUDOINSTRUCTION(...)
#endif

#ifndef REGISTER
#define REGISTER(...)
#endif

#ifndef DIRECTIVE
#define DIRECTIVE(...)
#endif

SYMBOL(COMMA,           "<,>")
SYMBOL(LABEL,           "<label>")
SYMBOL(PAREN_OPEN,      "<(>")
SYMBOL(PAREN_CLOSE,     "<)>")
SYMBOL(CHAR,            "<char>")
SYMBOL(STRING,          "<string>")
SYMBOL(UNCLOSED_STRING, "<unclosed>")
SYMBOL(UNKNOWN,         "<unknown>")
SYMBOL(CALL_LABEL,      "<call_label>")
SYMBOL(ENDOF,           "<eof>")
SYMBOL(INT16,           "<int16>")

// Add/substract (R-type)
INSTRUCTION(ADD,     "add",     R_Type, 0x20)
INSTRUCTION(ADDU,    "addu",    R_Type, 0x21)
INSTRUCTION(SUB,     "sub",     R_Type, 0x22)
INSTRUCTION(SUBU,    "subu",    R_Type, 0x23)
INSTRUCTION(SLT,     "slt",     R_Type, 0x2A)

// Add/substract (I-type)
INSTRUCTION(ADDI,    "addi",    I_Type, 0x8)
INSTRUCTION(ADDIU,   "addiu",   I_Type, 0x9)
INSTRUCTION(BNE,     "bne",     I_Type, 0x5)

// Unconditional jumps (J-type)
INSTRUCTION(J,       "j",       J_Type, 0x2)
INSTRUCTION(JAL,     "jal",     J_Type, 0x3)

// Special instructions
INSTRUCTION(SYSCALL, "syscall", S_Type, 0xC)

PSEUDOINSTRUCTION(move,  "move")
PSEUDOINSTRUCTION(clear, "clear")
PSEUDOINSTRUCTION(li,    "li")
PSEUDOINSTRUCTION(la,    "la")

// Registers (standard MIPS names)
REGISTER(ZERO,  "$zero", 0)
REGISTER(AT,    "$at",   1)
REGISTER(V0,    "$v0",   2)
REGISTER(V1,    "$v1",   3)
REGISTER(A0,    "$a0",   4)
REGISTER(A1,    "$a1",   5)
REGISTER(A2,    "$a2",   6)
REGISTER(A3,    "$a3",   7)
REGISTER(T0,    "$t0",   8)
REGISTER(T1,    "$t1",   9)
REGISTER(T2,    "$t2",  10)
REGISTER(T3,    "$t3",  11)
REGISTER(T4,    "$t4",  12)
REGISTER(T5,    "$t5",  13)
REGISTER(T6,    "$t6",  14)
REGISTER(T7,    "$t7",  15)
REGISTER(S0,    "$s0",  16)
REGISTER(S1,    "$s1",  17)
REGISTER(S2,    "$s2",  18)
REGISTER(S3,    "$s3",  19)
REGISTER(S4,    "$s4",  20)
REGISTER(S5,    "$s5",  21)
REGISTER(S6,    "$s6",  22)
REGISTER(S7,    "$s7",  23)
REGISTER(T8,    "$t8",  24)
REGISTER(T9,    "$t9",  25)
REGISTER(K0,    "$k0",  26)
REGISTER(K1,    "$k1",  27)
REGISTER(GP,    "$gp",  28)
REGISTER(SP,    "$sp",  29)
REGISTER(FP,    "$fp",  30)
REGISTER(RA,    "$ra",  31)

// Directives
DIRECTIVE(DATA,    ".data")
DIRECTIVE(TEXT,    ".text")
DIRECTIVE(GLOBL,   ".globl")
DIRECTIVE(WORD,    ".word")
DIRECTIVE(ASCII,   ".ascii")
DIRECTIVE(ASCIIZ,  ".asciiz")
DIRECTIVE(SPACE,   ".space")
DIRECTIVE(BYTE,    ".byte")
DIRECTIVE(HALF,    ".half")
DIRECTIVE(ALIGN,   ".align")
DIRECTIVE(END,     ".end")
DIRECTIVE(ENT,     ".ent")
DIRECTIVE(EXT,     ".ext")
DIRECTIVE(MACRO,   ".macro")
DIRECTIVE(NOMACRO, ".nomacro")

#undef SYMBOL
#undef INSTRUCTION
#undef PSEUDOINSTRUCTION
#undef REGISTER
#undef DIRECTIVE
