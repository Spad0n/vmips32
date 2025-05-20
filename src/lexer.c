#include "lexer.h"
#include <ctype.h>

typedef struct {
    String_View text;
    Token_Type type;
} Literal_Token;

static inline int is_word_head(int ch) {
    return isalpha(ch) || ch == '_';
}

static inline int is_word_body(int ch) {
    return isalnum(ch) || ch == '_' || ch == ':';
}

static inline int is_register_body(int ch) {
    return isalnum(ch) || ch == '$';
}

static inline int is_directive_body(int ch) {
    return isalnum(ch) || ch == '.';
}

static inline int is_int16(int ch) {
    return isdigit(ch) || ch == '-';
}

const char *lexer_token_type_to_cstr(Token_Type type) {
    switch (type) {
    #define SYMBOL(ENUM, NAME)                  case CAT(T_, ENUM): return NAME;
    #define KEYWORD(ENUM, NAME, FORMAT, OPCODE) case CAT(T_, ENUM): return NAME;
    #define EXTRAKEYWORD(ENUM, NAME)            case CAT(T_, ENUM): return NAME;
    #define REGISTER(ENUM, NAME, NUMBER)        case CAT(T_, ENUM): return NAME;
    #define DIRECTIVE(ENUM, NAME)               case CAT(T_, ENUM): return NAME;
    #include "lexer.inl"
    default: assert(0 && "Unreachable");
    }
}

Lexer lexer_from_sv(const char *src_filepath, String_View content) {
    return (Lexer) {
        .src_filepath = src_filepath,
        .content = content,
    };
}

char lexer_consume_char(Lexer *lexer) {
    assert(lexer->index < lexer->content.size);
    if (lexer->content.data[lexer->index] == '\n') {
        lexer->line += 1;
        lexer->bol = lexer->index + 1;
    }
    return lexer->content.data[lexer->index++];
}

String_View lexer_chop(Lexer *lexer, size_t count) {
    String_View text = sv_from_parts(&lexer->content.data[lexer->index], count);
    for (size_t i = 0; i < count; i++) {
        lexer_consume_char(lexer);
    }
    return text;
}

String_View lexer_chop_until(Lexer *lexer, char ch) {
    size_t count = 0;
    while (lexer->index + count < lexer->content.size && ch != lexer->content.data[lexer->index + count]) {
        count += 1;
    }
    return lexer_chop(lexer, count);
}

String_View lexer_chop_while(Lexer *lexer, int(*predicate)(int)) {
    size_t count = 0;
    while (lexer->index + count < lexer->content.size && predicate(lexer->content.data[lexer->index + count])) {
        count += 1;
    }
    return lexer_chop(lexer, count);
}

bool lexer_starts_with(Lexer *lexer, String_View sv) {
    if (lexer->index + sv.size >= lexer->content.size) return false;
    else return memcmp(&lexer->content.data[lexer->index], sv.data, sv.size) == 0;
}

Token lexer_next_token(Lexer *lexer) {
    Literal_Token registers[] = {
        #define REGISTER(ENUM, NAME, NUMBER) { .text = SV_STATIC(NAME), .type = CAT(T_, ENUM) },
        #include "lexer.inl"
    };
    
    Literal_Token keywords[] = {
        #define KEYWORD(ENUM, NAME, FORMAT, OPCODE) { .text = SV_STATIC(NAME), .type = CAT(T_, ENUM) },
        #define EXTRAKEYWORD(ENUM, NAME)            { .text = SV_STATIC(NAME), .type = CAT(T_, ENUM) },
        #include "lexer.inl"
    };

    Literal_Token directives[] = {
        #define DIRECTIVE(ENUM, NAME) { .text = SV_STATIC(NAME), .type = CAT(T_, ENUM) },
        #include "lexer.inl"
    };
    //char peek = 0;
 again:
    lexer_chop_while(lexer, isspace);

    Token token = {0};
    token.loc = (Loc) {
        .filepath = lexer->src_filepath,
        .col = lexer->index - lexer->bol + 1,
        .row = lexer->line + 1,
    };

    // end of file
    if (lexer->index >= lexer->content.size) {
        token.type = T_ENDOF;
        token.text = SV("<eof>");
        return token;
    }

    // ignore comments
    char peek = lexer->content.data[lexer->index];
    if (peek == '#') {
        lexer_chop_until(lexer, '\n');
        goto again;
    }

    // directives
    if (peek == '.') {
        token.text = lexer_chop_while(lexer, is_directive_body);

        for (size_t i = 0; i < STATIC_LEN(directives); i++) {
            if (sv_eq(token.text, directives[i].text)) {
                token.type = directives[i].type;
                return token;
            }
        }
    }

    // registers
    if (peek == '$') {
        token.text = lexer_chop_while(lexer, is_register_body);

        for (size_t i = 0; i < STATIC_LEN(registers); i++) {
            if (sv_eq(token.text, registers[i].text)) {
                token.type = registers[i].type;
                return token;
            }
        }
    }

    if (peek == '\'') {
        lexer_consume_char(lexer);
        token.type = T_CHAR;
        token.text = lexer_chop_until(lexer, '\'');
        lexer_consume_char(lexer);
        return token;
    }

    if (peek == '"') {
        lexer_consume_char(lexer);
        const char *begin = lexer->content.data + lexer->index;
        size_t begin_index = lexer->index;

        bool escaped = false;
        while (true) {
            if (lexer->index >= lexer->content.size || lexer->content.data[lexer->index] == '\n') {
                token.type = T_UNCLOSED_STRING;
                token.text = sv_from_parts(begin, lexer->index - begin_index - 1);
                return token;
            }

            //char ch = lexer->content.data[lexer->index];
            //lexer_consume_char(lexer);
            char ch = lexer_consume_char(lexer);

            if (!escaped && ch == '\"') {
                break;
            }
            escaped = (!escaped && ch == '\\');
        }

        token.type = T_STRING;
        token.text = sv_from_parts(begin, lexer->index - begin_index - 1);
        return token;
    }

    if (peek == '(') {
        lexer_consume_char(lexer);
        token.type = T_PAREN_OPEN;
        token.text = SV("(");
        return token;
    }

    if (peek == ')') {
        lexer_consume_char(lexer);
        token.type = T_PAREN_CLOSE;
        token.text = SV(")");
        return token;
    }

    if (peek == ',') {
        lexer_consume_char(lexer);
        token.type = T_COMMA;
        token.text = SV(",");
        return token;
    }

    if (isdigit(peek) || peek == '-') {
        token.type = T_INT16;
        token.text = lexer_chop_while(lexer, is_int16);
        return token;
    }

    if (is_word_head(peek)) {
        token.text = lexer_chop_while(lexer, is_word_body);

        if (token.text.data[token.text.size - 1] == ':') {
            lexer_consume_char(lexer);
            token.type = T_LABEL;
            return token;
        } else {
            // keyword
            for (size_t i = 0; i < STATIC_LEN(keywords); i++) {
                if (sv_eq(token.text, keywords[i].text)) {
                    token.type = keywords[i].type;
                    return token;
                }
            }

            token.type = T_CALL_LABEL;
            return token;
        }
    }

    token.type = T_UNKNOWN;
    token.text = lexer_chop(lexer, 1);
    return token;
}

void lexer_reset_lexer(Lexer *lexer) {
    lexer->index = 0;
    lexer->bol = 0;
    lexer->line = 0;
}
