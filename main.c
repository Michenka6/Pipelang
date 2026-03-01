#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "include/ast.h"
#include "include/cli_flags.h"
#include "include/ir.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/pretty_printer.h"
#include "include/types.h"
#include "include/util.h"
#include "include/codegen.h"

#define NOB_IMPLEMENTATION
#include "include/nob.h"

int
main(int argc, char **argv)
{
    Config config = parse_flags(argc, argv);
    if (config.print_usage || strcmp(config.filepath, "") == 0) {
        print_usage();
        return -1;
    }

    Nob_String_Builder sb = {0};

    const char* filepath = config.filepath;

    bool read_file = nob_read_entire_file(filepath, &sb);
    if (!read_file) return 1;

    Lexer lexer = {
        .source = nob_sv_from_parts(sb.items, sb.count),
        .cursor = 0,
        .line   = 1,
        .column = 1,
    };

    Tokens tokens = {0};
    lexer_tokenize(&lexer, &tokens);

    if (config.print_tokens) pretty_print_tokens(tokens);

    Parser parser = {
        .tokens = tokens,
        .cursor = 0,
    };

    AST_Nodes ast_nodes = {0};
    bool parsed_ok = false;
    size_t ast_root = parse(&parser, &ast_nodes, &parsed_ok);
    if (!parsed_ok) {
        printf("ERROR: Failed to parse!\n");
        return 1;
    }

    if (config.print_ast) {
        printf("\n");
        printf("AST PRETTY PRINT:\n");
        printf("#####\n");
        printf("#####\n");
        pretty_print_ast(ast_nodes, ast_root, 0);
        printf("\n");
    }

    IR_Nodes ir_nodes = {0};
    size_t ir_root = ast_to_ir(ast_nodes, ast_root, &ir_nodes);

    if (config.print_ir) {
        printf("\n");
        printf("IR PRETTY PRINT:\n");
        printf("#####\n");
        printf("#####\n");
        pretty_print_ir(ir_nodes, ir_root, 0);
        printf("\n");
    }

    Type_Env env = {0};
    bool typechecked = typecheck(ir_nodes, ir_root, &env);
    if (!typechecked) {
        printf("ERROR: Failed to typecheck the program!\n");
        return -1;
    }

    generate_c_from_ir(&ir_nodes, ir_root);

    return 0;
}
