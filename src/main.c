#include  "utils.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"
#include "lexer.h"
#include "parser.h"
#include "mvm.h"
#include <errno.h>

String read_file(Arena *arena, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Error: could not open the file '%s': %s\n", filepath, strerror(errno));
        exit(1);
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: could not read '%s': %s\n", filepath, strerror(errno));
        exit(1);
    }

    size_t count = ftell(file);
    rewind(file);

    char *data = malloc(count * sizeof(char));
    fread(data, sizeof(char), count, file);
    if (ferror(file)) {
        fprintf(stderr, "Error: could not read '%s': %s\n", filepath, strerror(errno));
        exit(1);
    }

    String string = {0};
    arena_da_append_many(arena, &string, data, count);
    free(data);
    fclose(file);
    return string;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.mips>\n", argv[0]);
        return 1;
    }
    Arena arena = {0};
    const char *src_filepath = argv[1];

    String file = read_file(&arena, src_filepath);
    String_View content = sv_from_parts(file.items, file.count);

    Lexer lexer = lexer_from_sv(src_filepath, content);

    //Token token = {0};
    //while (token.type != T_ENDOF) {
    //    token = lexer_next_token(&lexer);
    //    printf("token: %s, string: "SV_FMT"\n", lexer_token_type_to_cstr(token.type), SV_ARG(token.text));
    //}
    //lexer_reset_lexer(&lexer);

    Parser parser = parser_second_pass(&arena, &lexer);
    if (parser.failed) {
        fprintf(stderr, STR_FMT, STR_ARG(&parser.error_message));
        exit(1);
    }
    for (size_t i = 0; i < parser.segments.count; i++) {
        //printf("name: "SV_FMT", line: %d\n", SV_ARG(parser.tds.items[i].name), parser.tds.items[i].address);
        printf("inst: 0x%08X\n", parser.segments.items[i]);
    }

    //Parser parser = parser_parse(&arena, &lexer);
    //if (parser.failed) {
    //    fprintf(stderr, STR_FMT"\n", STR_ARG(&parser.error_message));
    //    exit(1);
    //}

    ////for (size_t i = 0; i < parser.segments.count; i++) {
    ////    printf("inst: 0x%08X\n", parser.segments.items[i]);
    ////}
    //FILE *fw = fopen("bytecode.bin", "wb");
    //if (!fw) {
    //    fprintf(stderr, "Error: couldn't create the file \"bytecode.bin\": %s\n", strerror(errno));
    //    exit(1);
    //}

    ////for (size_t i = 0; i < parser.segments.count; i++) {
    ////    uint32_t hexa = parser.segments.items[i];
    ////    if (fwrite(&hexa, sizeof(), 1, fw) != 1) {
    ////        fprintf(stderr, "Error: couldn't write a hexadecimal byte 0x%08X: %s\n", hexa, strerror(errno));
    ////        exit(1);
    ////    }
    ////}
    //if (fwrite(parser.segments.items, sizeof(uint32_t), parser.segments.count, fw) != parser.segments.count) {
    //    fprintf(stderr, "Error: couldn't write a hexadecimal byte: %s\n", strerror(errno));
    //    exit(1);
    //}

    //fclose(fw);

    //FILE *test = fopen("bytecode.bin", "rb");
    //if (!test) {
    //    fprintf(stderr, "Error: couldn't read the file \"bytecode.bin\": %s\n", strerror(errno));
    //    exit(1);
    //}
    //uint8_t buffer[4];
    //MVM mips = {0};
    //while (fread(buffer, sizeof(uint8_t), 4, test) == 4) {
    //    uint32_t instruction = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    //    mvm_execute_one(&mips, instruction);
    //}
    //if (!feof(test)) {
    //    fprintf(stderr, "Error: %s\n", strerror(errno));
    //    exit(1);
    //}

    arena_free(&arena);
    return 0;
}
