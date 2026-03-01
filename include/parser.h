#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "token.h"
#include "ast.h"

typedef struct Parser
{
    Tokens tokens;
    size_t cursor;
} Parser;

size_t parse(Parser *parser, AST_Nodes *ast_nodes, bool *ok);

#endif // PARSER_H
