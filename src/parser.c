#include "parser.h"

int get_instruction_code(Token *token) {
    switch (token->type) {
    #define INSTRUCTION(ENUM, NAME, TYPE, CODE) case CAT(T_, ENUM): return CODE;
    #include "lexer.inl"
    default: assert(0 && "Unreachable (get_instruction_number)");
    }
}

int get_register_number(Token *token) {
    if (token == NULL) return 0;
    switch (token->type) {
    #define REGISTER(ENUM, NAME, NUMBER) case CAT(T_, ENUM): return NUMBER;
    #include "lexer.inl"
    default: assert(0 && "Unreachable (get_register_number)");
    }
}

static bool is_type_instruction(Token_Type type) {
    switch (type) {
    #define INSTRUCTION(ENUM, NAME, FORMAT, CODE) case CAT(T_, ENUM): return true;
    #include "lexer.inl"
    default: return false;
    }
}

static bool is_type_register(Token_Type type) {
    switch (type) {
    #define REGISTER(ENUM, NAME, CODE) case CAT(T_, ENUM): return true;
    #include "lexer.inl"
    default: return false;
    }
}

static bool is_type_label(Token_Type type) {
    if (type == T_CALL_LABEL) return true;
    else return false;
}

Token parse_next_token(Arena *arena, Parser *parser, Lexer *lexer) {
    Token token = lexer_next_token(lexer);
    if (token.type == T_UNCLOSED_STRING) {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: unclosed string literal\n");
    } else if (token.type == T_UNKNOWN) {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: unclosed unknown character: "SV_FMT"\n", SV_ARG(token.text));
    }
    return token;
}

static bool parse_token_expect(Arena *arena, Lexer *lexer, Parser *parser, Token_Type type) {
    Token token = parse_next_token(arena, parser, lexer);
    if (token.type != type) {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: unexpected token"SV_FMT", expected: `%s`\n", SV_ARG(token.text), lexer_token_type_to_cstr(type));
        return false;
    }
    return true;
}

static bool parse_token_predicate(Arena *arena, Lexer *lexer, Parser *parser, Token *token, bool (*predicate)(Token_Type)) {
    *token = parse_next_token(arena, parser, lexer);
    if (parser->failed) return false;
    else if (!predicate(token->type)) {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: not a register: "SV_FMT"\n", SV_ARG(token->text));
        return false;
    }
    return true;
}

static bool parse_immediate(Arena *arena, Lexer *lexer, Parser *parser, int16_t *imm, uint32_t address) {
    Token token = parse_next_token(arena, parser, lexer);
    if (parser->failed) return false;
    if (token.type == T_CALL_LABEL) {
        for (size_t i = 0; i < parser->tds.count; i++) {
            if (sv_eq(token.text, parser->tds.items[i].name)) {
                *imm = (parser->tds.items[i].address - address) / 4;
                return true;
            }
        }
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: this label never declared: "SV_FMT"\n", SV_ARG(token.text));
        return false;
    } else if (token.type == T_INT16) {
        *imm = sv_to_int16(token.text);
        return true;
    } else {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: this token is not a label nor a int16: "SV_FMT"\n", SV_ARG(token.text));
        return false;
    }
}

static Parser first_pass(Arena *arena, Lexer *lexer) {
    Parser parser = {0};
    uint32_t address = 0x00400000;
    Token token = lexer_next_token(lexer);
    while (token.type != T_ENDOF) {
        if (token.type != T_LABEL) {
            token = lexer_next_token(lexer);
            while (!is_type_instruction(token.type) && token.type != T_ENDOF && token.type != T_LABEL) {
                token = lexer_next_token(lexer);
            }
            address += 4;
        } else {
            Symbol symbol = {
                .name = {
                    .data = token.text.data,
                    .size = token.text.size - 1,
                },
                .address = address,
            };
            arena_da_append(arena, &parser.tds, symbol);
            token = lexer_next_token(lexer);
        }
    }
    lexer_reset_lexer(lexer);
    return parser;
}

static uint32_t get_address(Token addr) {
    (void)addr;
    assert(0 && "TODO");
    return 0;
}

void assemble_i_type(Arena *arena, Parser *parser, Token *token_inst, Token *token1, Token *token2, uint16_t imm, uint32_t address) {
    Instruction result = {
        .instruction = (get_instruction_code(token_inst) << 26) | (get_register_number(token1) << 21) | (get_register_number(token2) << 16) | (0xFFFF & imm),
        .address = address,
    };
    arena_da_append(arena, &parser->segments, result);
}

void assemble_r_type(Arena *arena, Parser *parser, Token *token_inst, Token *token1, Token *token2, Token *token3, uint32_t address) {
    Instruction result = {
        .instruction = (0 << 26) | (get_register_number(token1) << 21) | (get_register_number(token2) << 16) | (get_register_number(token3) << 11) | (0 << 6) | get_instruction_code(token_inst),
        .address = address,
    };
    arena_da_append(arena, &parser->segments, result);
}

void assemble_j_type(Arena *arena, Parser *parser, Token opcode, Token addr, uint32_t address) {
    Instruction result = {
        .instruction = (get_instruction_code(&opcode) << 26) | get_address(addr),
        .address = address,
    };
    arena_da_append(arena, &parser->segments, result);
}

Parser parser_parse(Arena *arena, Lexer *lexer) {
    Parser parser = first_pass(arena, lexer);
    if (parser.failed) return parser;

    Token token = {0};
    uint32_t address = 0x00400000;
    while (token.type != T_ENDOF) {
        token = lexer_next_token(lexer);
        Token rs = {0}, rt = {0}, rd = {0};
        int16_t imm = 0;
        switch (token.type) {
        case T_ADD:
        case T_ADDU:
        case T_SUB:
        case T_SUBU:
        case T_SLT:
            if (!parse_token_predicate(arena, lexer, &parser, &rd, is_type_register)) return parser;
            if (!parse_token_expect(arena, lexer, &parser, T_COMMA)) return parser;
            if (!parse_token_predicate(arena, lexer, &parser, &rs, is_type_register)) return parser;
            if (!parse_token_expect(arena, lexer, &parser, T_COMMA)) return parser;
            if (!parse_token_predicate(arena, lexer, &parser, &rt, is_type_register)) return parser;
            assemble_r_type(arena, &parser, &token, &rs, &rt, &rd, address);
            address += 4;
            break;
        case T_ADDI:
        case T_ADDIU:
            if (!parse_token_predicate(arena, lexer, &parser, &rt, is_type_register)) return parser;
            if (!parse_token_expect(arena, lexer, &parser, T_COMMA)) return parser;
            if (!parse_token_predicate(arena, lexer, &parser, &rs, is_type_register)) return parser;
            if (!parse_token_expect(arena, lexer, &parser, T_COMMA)) return parser;
            if (!parse_immediate(arena, lexer, &parser, &imm, address + 4)) return parser;
            assemble_i_type(arena, &parser, &token, &rs, &rt, imm, address);
            address += 4;
            break;
        case T_BNE:
        case T_ORI:
            if (!parse_token_predicate(arena, lexer, &parser, &rs, is_type_register)) return parser;
            if (!parse_token_expect(arena, lexer, &parser, T_COMMA)) return parser;
            if (!parse_token_predicate(arena, lexer, &parser, &rt, is_type_register)) return parser;
            if (!parse_token_expect(arena, lexer, &parser, T_COMMA)) return parser;
            if (!parse_immediate(arena, lexer, &parser, &imm, address + 4)) return parser;
            assemble_i_type(arena, &parser, &token, &rs, &rt, imm, address);
            address += 4;
            break;
        case T_J: {
            Token addr = {0};
            if (!parse_token_predicate(arena, lexer, &parser, &addr, is_type_label)) return parser;
            assemble_j_type(arena, &parser, token, addr, address);
            address += 4;
        } break;
        case T_SYSCALL: {
            Instruction instr = {
                .instruction = get_instruction_code(&token),
                .address = address,
            };
            arena_da_append(arena, &parser.segments, instr);
            address += 4;
        } break;
        case T_LABEL: break;
        default:
            if (token.type == T_ENDOF) break;
            fprintf(stderr, "not implemented: %s", lexer_token_type_to_cstr(token.type));
            exit(1);
        }
    }
    return parser;
}
