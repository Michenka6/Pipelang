#ifndef CLI_FLAGS_H
#define CLI_FLAGS_H

#include "stdbool.h"

typedef struct Config {
    const char *filepath;
    bool print_usage;
    bool print_tokens;
    bool print_ast;
    bool print_ir;
    bool typecheck;
    bool generate_c;
} Config;

void print_usage(void);
Config parse_flags(int argc, char **argv);

#endif // CLI_FLAGS_H
