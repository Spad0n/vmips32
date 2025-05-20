#ifndef LEXER_H
#define LEXER_H

#include "utils.h"

typedef enum {
    #define SYMBOL(ENUM, NAME)           CAT(T_, ENUM),
    #define INSTRUCTION(ENUM, NAME, FORMAT, OPCODE) CAT(T_, ENUM),
    #define PSEUDOINSTRUCTION(ENUM, NAME)     CAT(T_, ENUM),
    #define REGISTER(ENUM, NAME, NUMBER) CAT(T_, ENUM),
    #define DIRECTIVE(ENUM, NAME)        CAT(T_, ENUM),
    #include "lexer.inl"
} Token_Type;

typedef struct {
    const char *filepath;
    size_t row;
    size_t col;
} Loc;

typedef struct {
    String_View text;
    Token_Type type;
    Loc loc;
} Token;

typedef struct {
    const char *src_filepath;
    String_View content;
    size_t index;
    size_t line;
    size_t bol;
} Lexer;

const char *lexer_token_type_to_cstr(Token_Type type);
Lexer lexer_from_sv(const char *src_filepath, String_View content);
char lexer_consume_char(Lexer *lexer);
String_View lexer_chop(Lexer *lexer, size_t count);
String_View lexer_chop_until(Lexer *lexer, char ch);
String_View lexer_chop_while(Lexer *lexer, int(*predicate)(int));
bool lexer_starts_with(Lexer *lexer, String_View sv);

Token lexer_next_token(Lexer *lexer);
void lexer_reset_lexer(Lexer *lexer);

#endif // LEXER_H
