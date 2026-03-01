#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdint.h>
#include "nob.h"

typedef struct Indices
{
    size_t* items;
    size_t count;
    size_t capacity;
} Indices;

typedef enum Op
{
    Op_MINUS,
    Op_TIMES,
    Op_PLUS,
    Op_MIN,
    Op_MAX,
    Op_DIVIDE,
    Op_GR,
    Op_LEQ,
    Op_POW,
} Op;

typedef enum AST_Node_Kind
{
    AST_Node_Kind_BLOCKS,
    AST_Node_Kind_SEQUENCE,
    AST_Node_Kind_BRANCH,
    AST_Node_Kind_FUN_CALL,
    AST_Node_Kind_UNARY_FUN,
    AST_Node_Kind_PAREN,
    AST_Node_Kind_CONSTANT_INT64,
    AST_Node_Kind_CONSTANT_FLOAT64,
    AST_Node_Kind_COUNT,
} AST_Node_Kind;

typedef struct AST_Node
{
    AST_Node_Kind kind;
    union
    {
        struct { Indices nodes; } blocks;
        struct { size_t node; } sequence;
        struct { size_t node; Nob_String_View folder; } branch;
        struct { Nob_String_View call_path; } fun_call;
        struct { Op op; size_t param; } unary_fun;
        struct { size_t node; } paren;
        struct { int64_t value; } constant_int64;
        struct { double value; } constant_float64;
    } as;
} AST_Node;

typedef struct AST_Nodes
{
    AST_Node* items;
    size_t count;
    size_t capacity;
} AST_Nodes;

#endif // AST_H
