#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include "nob.h"

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "token.h"
#include "ir.h"
#include "types.h"
    

void pretty_print_tokens(Tokens tokens);
void pretty_print_ast(AST_Nodes ast_nodes, size_t ast_root, size_t level);
void pretty_print_ir(IR_Nodes ir_nodes, size_t ir_root, size_t level);
void pretty_print_type(Struct_Type type);

#endif // PRETTY_PRINTER_H
