#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "arena.h"

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)
#define STATIC_LEN(x) (sizeof(x)/sizeof(x[0]))

#ifdef DEFER_IMPLEMENTATION
#if !(defined(__GNUC__) || defined(__GNUG__) || defined(__clang__) || defined(__APPLE__))
#error "Unsupported compiler"
#endif


#define defer                                                         \
    auto void CAT(DEFER_FUNCTION_, __LINE__)(int *);                                        \
    int CAT(DEFER_VAR_, __LINE__) __attribute__((cleanup(CAT(DEFER_FUNCTION_, __LINE__)))); \
    void CAT(DEFER_FUNCTION_, __LINE__)(int *)

#define defer_line(x) defer { x; }
#endif // DEFER_IMPLEMENTATION

#define SV(cstr) ((String_View) { .data = (cstr), .size = strlen(cstr) })
#define SV_STATIC(cstr) { .data = (cstr), .size = sizeof(cstr) - 1 }

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int)(sv).size, (sv).data

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String;

#define STR_FMT "%.*s"
#define STR_ARG(str) (int)(str)->count, (str)->items

bool str_eq(String *a, String *b);
bool str_eq_cstr(String *a, const char *b);
void str_append_vfmt(Arena *arena, String *str, const char *fmt, va_list args);
void str_append_fmt(Arena *arena, String *str, const char *fmt, ...);

typedef struct {
    const char *data;
    size_t size;
} String_View;

int16_t sv_to_int16(String_View sv);
String_View sv_from_parts(const char *data, size_t size);
bool sv_eq(String_View a, String_View b);

#endif // UTILS_H
