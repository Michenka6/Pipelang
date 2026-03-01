#ifndef IR_H
#define IR_H

#include <stdlib.h>
#include "ast.h"

typedef enum IR_Node_Kind {
    IR_Node_Kind_FUN,
    IR_Node_Kind_CONCAT,
    IR_Node_Kind_FORK,
    IR_Node_Kind_FOLD,
    IR_Node_Kind_UNFOLD,
    IR_Node_Kind_UNARY,
    IR_Node_Kind_FLOAT,
    IR_Node_Kind_INT,
} IR_Node_Kind;

typedef struct IR_Node {
    IR_Node_Kind kind;
    Nob_String_View name;
    size_t lhs;
    size_t rhs;
    int64_t iv;
    double fv;
    Op op;
} IR_Node;

typedef struct IR_Nodes {
    IR_Node *items;
    size_t count;
    size_t capacity;
} IR_Nodes;

size_t ast_to_ir(AST_Nodes ast_nodes, size_t ast_root, IR_Nodes *nodes);

#endif // IR_H
