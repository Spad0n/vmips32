#include "utils.h"

#if defined(_WIN32)
#  define utils_alloca(size) _alloca(size)
#elif defined(__unix__)
#  include <alloca.h>
#  define utils_alloca(size) alloca(size)
#else
#  error "Platform does not support `alloca`"
#endif

bool str_eq(String *a, String *b) {
    if (a->count != b->count) return false;
    else return memcmp(a->items, b->items, a->count) == 0;
}

bool str_eq_cstr(String *a, const char *b) {
    size_t b_count = strlen(b);
    if (a->count != b_count) return false;
    else return memcmp(a->items, b, a->count) == 0;
}

void str_append_vfmt(Arena *arena, String *str, const char *fmt, va_list args) {
    va_list copy;
    va_copy(copy, args);
    int str_size = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    char *temp = utils_alloca((str_size + 1) * sizeof(char));
    vsnprintf(temp, str_size + 1, fmt, args);
    arena_da_append_many(arena, str, temp, str_size);
}

void str_append_fmt(Arena *arena, String *str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    str_append_vfmt(arena, str, fmt, args);
    va_end(args);
}

int16_t sv_to_int16(String_View sv) {
    char *int16_string = utils_alloca(sizeof(char) * sv.size + 1);
    memcpy(int16_string, sv.data, sv.size);
    int16_string[sv.size] = '\0';
    char *endptr;
    long val = strtol(int16_string, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Erreur de convertion\n");
        exit(1);
    } else if (val < INT16_MIN || val > INT16_MAX) {
        fprintf(stderr, "Erreur de convertion\n");
        exit(1);
    }
    return (int16_t)val;
}

bool sv_eq(String_View a, String_View b) {
    if (a.size != b.size) return false;
    else return memcmp(a.data, b.data, a.size) == 0;
}

String_View sv_from_parts(const char *data, size_t size) {
    return (String_View) {data, size};
}
