#include "../include/cli_flags.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

const Config DEFAULT_CONFIG = {
    .filepath     = "",
    .print_usage  = false,
    .print_tokens = false,
    .print_ast    = false,
    .print_ir     = false,
    .typecheck    = true,
    .generate_c   = true,
};

void
print_usage(void)
{
    static const char *HELP =
    "pipe -- PipeLang compiler\n"
    "\n"
    "Usage:\n"
    "  pipe [OPTIONS] <file>\n"
    "\n"
    "Options:\n"
    "  --help           Show this help message and exit\n"
    "  --print_tokens   Print tokens after lexing\n"
    "  --print_ast      Pretty print AST after parsing\n"
    "  --print_ir       Pretty print IR after lowering\n"
    "  --typecheck      Enable typechecking (default: on)\n"
    "  --no-typecheck   Disable typechecking\n"
    "  --generate_c     Generate C output (default: on)\n"
    "  --no-generate_c  Disable C generation\n";

    printf("%s", HELP);
}


Config
parse_flags(int argc, char **argv)
{
    Config config = DEFAULT_CONFIG;

    bool file_provided = false;

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        if (strcmp(arg, "--help") == 0)
        {
            config.print_usage = true;
            return config;
        }

        else if (strcmp(arg, "--print_tokens") == 0)
        {
            config.print_tokens = true;
        }
        else if (strcmp(arg, "--print_ast") == 0)
        {
            config.print_ast = true;
        }
        else if (strcmp(arg, "--print_ir") == 0)
        {
            config.print_ir = true;
        }

        else if (strcmp(arg, "--typecheck") == 0)
        {
            config.typecheck = true;
        }
        else if (strcmp(arg, "--no-typecheck") == 0)
        {
            config.typecheck = false;
        }

        else if (strcmp(arg, "--generate_c") == 0)
        {
            config.generate_c = true;
        }
        else if (strcmp(arg, "--no-generate_c") == 0)
        {
            config.generate_c = false;
        }

        else if (arg[0] != '-')
        {
            if (file_provided)
            {
                fprintf(stderr, "pipe: multiple input files provided\n");
                exit(1);
            }

            config.filepath = arg;
            file_provided = true;
        }

        else
        {
            fprintf(stderr, "pipe: unknown option '%s'\n", arg);
            fprintf(stderr, "Try 'pipe --help' for more information.\n");
            exit(1);
        }
    }

    return config;
}
