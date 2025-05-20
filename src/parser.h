#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "utils.h"

//typedef struct {
//    String_View name;
//    uint32_t address;
//} TDS;

typedef struct {
    String_View name;
    uint32_t address;
} Symbol;

typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} TDS;

typedef struct {
    uint32_t instruction;
    uint32_t address;
} Instruction;

typedef struct {
    Instruction *items;
    size_t count;
    size_t capacity;
} Segments;

typedef struct {
    TDS tds;
    Segments segments;
    bool failed;
    String error_message;
} Parser;

Parser parser_parse(Arena *arena, Lexer *lexer);
Parser parser_first_pass(Arena *arena, Lexer *lexer);
Parser parser_second_pass(Arena *arena, Lexer *lexer);

#endif // PARSER_H
