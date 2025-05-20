/* -*- compile-command: "cc nob.c -o nob" -*- */
#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "gcc", "-ggdb", "-Wall", "-Wextra", "-o", "main", "src/mvm.c", "src/main.c", "src/utils.c", "src/lexer.c", "src/parser.c");
    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    //if (!nob_cmd_run_sync(cmd)) return 1;

    nob_cmd_append(&cmd, "./main", "loop.mips");
    nob_cmd_run_sync(cmd);

    return 0;
}
