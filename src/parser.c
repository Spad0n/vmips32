#include "parser.h"

//typedef enum {
//    #define REGISTER(ENUM, NAME, NUMBER) CAT(T_, ENUM),
//    #include "lexer.inl"
//} Register_Type;

typedef enum {
    Unknown_Type = 0,
    R_Type,
    I_Type,
    J_Type,
    S_Type,
    Label_Type,
} Format;

static Format format_type(Token_Type type) {
    switch (type) {
    #define INSTRUCTION(ENUM, NAME, FORMAT, OPCODE) case CAT(T_, ENUM): return FORMAT;
    #include "lexer.inl"
    case T_LABEL: return Label_Type;
    default: return Unknown_Type;
    }
}

//static bool is_type_immediate(Arena *arena, Parser *parser, Token token) {
//    if (token.type == T_INT16) return true;
//    else {
//        str_append_fmt(arena, &parser->error_message, "%l Error: not a immediate `%w`\n", token.loc, token);
//        return false;
//    }
//}

static bool is_type_register(Arena *arena, Parser *parser, Token token) {
    switch (token.type) {
    #define REGISTER(ENUM, NAME, NUMBER) case CAT(T_, ENUM): return true;
    #include "lexer.inl"
    default:
        str_append_fmt(arena, &parser->error_message, "Error: not a register: "SV_FMT"\n", SV_ARG(token.text));
        return false;
    }
}

static bool is_type_label(Arena *arena, Parser *parser, Token token) {
    if (token.type == T_CALL_LABEL) return true;
    else {
        str_append_fmt(arena, &parser->error_message, "Error: not a call label: "SV_FMT"\n", SV_ARG(token.text));
        return false;
    }
}

static int get_opcode(Token_Type type) {
    switch (type) {
    #define INSTRUCTION(ENUM, NAME, TYPE, OPCODE) case CAT(T_, ENUM): return OPCODE;
    #include "lexer.inl"
    default:
        fprintf(stderr, "this keyword have no opcode: %s\n", lexer_token_type_to_cstr(type));
        exit(1);
    }
}

static int get_register_number(Token_Type type) {
    switch (type) {
    #define REGISTER(ENUM, NAME, NUMBER) case CAT(T_, ENUM): return NUMBER;
    #include "lexer.inl"
    default:
        fprintf(stderr, "this register have no number: %s\n", lexer_token_type_to_cstr(type));
        exit(1);
    }
}

//static int16_t get_immediate_value(Token imm) {
//    return sv_to_int16(imm.text);
//}


static uint32_t get_address(Token addr) {
    (void)addr;
    return 0;
}

// TODO: verify all differents instructions parameters
static uint32_t assemble_i_type(Token opcode, Token rt, Token rs, int16_t imm) {
    if (opcode.type == T_BNE) {
        return (get_opcode(opcode.type) << 26) | (get_register_number(rt.type) << 21) | (get_register_number(rs.type) << 16) | (0xFFFF & imm);
    } else {
        return (get_opcode(opcode.type) << 26) | (get_register_number(rs.type) << 21) | (get_register_number(rt.type) << 16) | (0xFFFF & imm);
    }
}

static uint32_t assemble_r_type(Token funct, Token rd, Token rs, Token rt) {
    return (0 << 26) | (get_register_number(rs.type) << 21) | (get_register_number(rt.type) << 16) | (get_register_number(rd.type) << 11) | (0 << 6) | get_opcode(funct.type);
}

static uint32_t assemble_j_type(Token opcode, Token addr) {
    return (get_opcode(opcode.type) << 26) | get_address(addr);
}

Token parser_parse_next_token(Arena *arena, Parser *parser, Lexer *lexer) {
    Token token = lexer_next_token(lexer);
    switch (token.type) {
    case T_UNCLOSED_STRING: {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: unclosed string literal\n");
    } break;
    case T_UNKNOWN: {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: unclosed unknown character: "SV_FMT"\n", SV_ARG(token.text));
    } break;
    default: {}
    }
    return token;
}

bool parser_parse_next_token_with_predicate(Token *token, Arena *arena, Parser *parser, Lexer *lexer, bool (*predicate)(Arena*, Parser*, Token)) {
    *token = parser_parse_next_token(arena, parser, lexer);
    if (parser->failed) return false;
    return predicate(arena, parser, *token);
}

bool parser_parse_immediate(int16_t *imm, uint32_t address, Arena *arena, Parser *parser, Lexer *lexer) {
    Token token = parser_parse_next_token(arena, parser, lexer);
    if (parser->failed) return false;
    switch (token.type) {
    case T_CALL_LABEL:
        for (size_t i = 0; i < parser->tds.count; i++) {
            if (sv_eq(token.text, parser->tds.items[i].name)) {
                *imm = (parser->tds.items[i].address - address) / 4;
                return true;
            }
        }
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: this label never declared: "SV_FMT"\n", SV_ARG(token.text));
        return false;
    case T_INT16:
        *imm = sv_to_int16(token.text);
        return true;
    default:
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: this token is not a label nor a int16: "SV_FMT"\n", SV_ARG(token.text));
        return false;
    }
}

//Token parser_peek_token(Arena *arena, Parser *parser, Lexer *lexer) {
//    Lexer save_lexer = *lexer;
//    Token peek = parser_parse_next_token(arena, parser, lexer);
//    *lexer = save_lexer;
//    return peek;
//}

//Token parser_parse_expect(Arena *arena, Parser *parser, Lexer *lexer, bool (*predicate)(Token_Type)) {
//    Token token = parser_parse_next_token(arena, parser, lexer);
//    if (predicate(token.type)) {
//        parser->failed = true;
//        str_append_fmt(arena, &parser->error_message, "%l: Error: unexpected token `%w`, expected `%T`\n", token.loc, token.text, type);
//    }
//    return token;
//}

bool parser_parse_expect(Arena *arena, Parser *parser, Lexer *lexer, Token_Type type) {
    Token token = parser_parse_next_token(arena, parser, lexer);
    if (token.type != type) {
        parser->failed = true;
        str_append_fmt(arena, &parser->error_message, "Error: unexpected token "SV_FMT", expected `%s`\n", SV_ARG(token.text), lexer_token_type_to_cstr(type));
        return false;
    }
    return true;
}

Parser parser_first_pass(Arena *arena, Lexer *lexer) {
    Parser parser = {0};
    Token token = {0};
    uint32_t address = 0x00400000;
    while (token.type != T_ENDOF) {
        token = lexer_next_token(lexer);
        switch (format_type(token.type)) {
        case R_Type:
        case I_Type:
            for (size_t i = 0; i < 5; i++) {
                lexer_next_token(lexer);
            }
            address += 4;
            break;
        case J_Type:
            lexer_next_token(lexer);
            address += 4;
            break;
        case S_Type:
            address += 4;
            break;
        case Label_Type: {
            Symbol symbol = {
                .name = {
                    .data = token.text.data,
                    .size = token.text.size - 1,
                },
                .address = address,
            };
            arena_da_append(arena, &parser.tds, symbol);
        } break;
        case Unknown_Type:
        default: {}
        }
    }
    lexer_reset_lexer(lexer);
    return parser;
}

Parser parser_second_pass(Arena *arena, Lexer *lexer) {
    Parser parser = parser_first_pass(arena, lexer);
    if (parser.failed) return parser;

    Token token = {0};
    uint32_t address = 0x00400000;
    while (token.type != T_ENDOF) {
        token = lexer_next_token(lexer);

        switch (format_type(token.type)) {
        case R_Type: {
            Token rd, rs, rt;
            if (!parser_parse_next_token_with_predicate(&rd, arena, &parser, lexer, is_type_register)) return parser;
            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
            if (!parser_parse_next_token_with_predicate(&rs, arena, &parser, lexer, is_type_register)) return parser;
            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
            if (!parser_parse_next_token_with_predicate(&rt, arena, &parser, lexer, is_type_register)) return parser;
            Instruction instr = {
                .instruction = assemble_r_type(token, rd, rs, rt),
                .address     = address,
            };
            //arena_da_append(arena, &parser.segments, assemble_r_type(token, rd, rs, rt));
            arena_da_append(arena, &parser.segments, instr);
            address += 4;
        } break;
        case I_Type: {
            Token rt, rs;
            int16_t imm;
            if (!parser_parse_next_token_with_predicate(&rt, arena, &parser, lexer, is_type_register)) return parser;
            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
            if (!parser_parse_next_token_with_predicate(&rs, arena, &parser, lexer, is_type_register)) return parser;
            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
            if (!parser_parse_immediate(&imm, address + 4, arena, &parser, lexer)) return parser;
            // TODO: maybe do some condition
            Instruction instr = {
                .instruction = assemble_i_type(token, rt, rs, imm),
                .address     = address,
            };
            //arena_da_append(arena, &parser.segments, assemble_i_type(token, rt, rs, imm));
            arena_da_append(arena, &parser.segments, instr);
            address += 4;
        } break;
        case J_Type: {
            Token addr;
            if (!parser_parse_next_token_with_predicate(&addr, arena, &parser, lexer, is_type_label)) return parser;
            Instruction instr = {
                .instruction = assemble_j_type(token, addr),
                .address     = address,
            };
            //arena_da_append(arena, &parser.segments, assemble_j_type(token, addr));
            arena_da_append(arena, &parser.segments, instr);
            address += 4;
        } break;
        case S_Type: {
            Instruction instr = {
                .instruction = get_opcode(token.type),
                .address     = address,
            };
            arena_da_append(arena, &parser.segments, instr);
            address += 4;
        } break;
        case Label_Type: break;
        case Unknown_Type:
        default:
            if (token.type == T_ENDOF) break;
            assert(0 && "TODO");
        }
    }

    return parser;
}

//Parser parser_parse(Arena *arena, Lexer *lexer) {
//    Parser parser = {0};
//
//    for (;;) {
//        Token opcode = lexer_next_token(lexer);
//        if (opcode.type == T_ENDOF) break;
//
//        switch (format_type(opcode.type)) {
//        case R_Type: {
//            Token rd, rs, rt;
//            if (!parser_parse_next_token_with_predicate(&rd, arena, &parser, lexer, is_type_register)) return parser;
//            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
//            if (!parser_parse_next_token_with_predicate(&rs, arena, &parser, lexer, is_type_register)) return parser;
//            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
//            if (!parser_parse_next_token_with_predicate(&rt, arena, &parser, lexer, is_type_register)) return parser;
//            arena_da_append(arena, &parser.segments, assemble_r_type(opcode, rd, rs, rt));
//        } break;
//        case I_Type: {
//            Token rt, rs, imm;
//            if (!parser_parse_next_token_with_predicate(&rt, arena, &parser, lexer, is_type_register)) return parser;
//            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
//            if (!parser_parse_next_token_with_predicate(&rs, arena, &parser, lexer, is_type_register)) return parser;
//            if (!parser_parse_expect(arena, &parser, lexer, T_COMMA)) return parser;
//            if (!parser_parse_next_token_with_predicate(&imm, arena, &parser, lexer, is_type_immediate)) return parser;
//            arena_da_append(arena, &parser.segments, assemble_i_type(opcode, rt, rs, imm));
//        } break;
//        case J_Type: {
//            Token addr;
//            if (!parser_parse_next_token_with_predicate(&addr, arena, &parser, lexer, is_type_label)) return parser;
//            arena_da_append(arena, &parser.segments, assemble_j_type(opcode, addr));
//        } break;
//        case S_Type: {
//            arena_da_append(arena, &parser.segments, get_opcode(opcode.type));
//        } break;
//        default: assert(0 && "TODO");
//        }
//    }
//
//    return parser;
//}
