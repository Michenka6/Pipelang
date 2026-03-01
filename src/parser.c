#include "../include/parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/nob.h"

static inline bool
parser_peek(Parser* parser, Token *tk)
{
    if (parser->cursor >= parser->tokens.count) return false;
    *tk = parser->tokens.items[parser->cursor];
    return true;
}

static bool
eat_tk(Parser* p, Token_Kind kind, Nob_String_View payload)
{
    if (p->cursor >= p->tokens.count) return false;

    Token tk = p->tokens.items[p->cursor];
    bool is_empty_payload = (payload.count == 0);
    bool same_kind = (tk.kind == kind);
    bool same_payload = nob_sv_eq(tk.payload, payload);

    if (same_kind && (is_empty_payload || same_payload)) {
        p->cursor += 1;
        return true;
    }
    return false;
}

static size_t parse_blocks(Parser* parser, AST_Nodes *ast_nodes, bool* ok);
static size_t parse_block (Parser* parser, AST_Nodes *ast_nodes, bool* ok);

static bool
parse_op(Parser *parser, Op *op)
{
    Token tk;
    if (!parser_peek(parser, &tk)) return false;
    if (tk.kind != Token_Kind_CHAR && tk.kind != Token_Kind_IDENT) return false;

    switch (tk.payload.data[0]) {
        case '-':
            *op = Op_MINUS;
            parser->cursor += 1;
            return true;
        case '*':
            *op = Op_TIMES;
            parser->cursor += 1;
            return true;
        case '+':
            *op = Op_PLUS;
            parser->cursor += 1;
            return true;
        case '/':
            *op = Op_DIVIDE;
            parser->cursor += 1;
            return true;
        case '>':
            *op = Op_GR;
            parser->cursor += 1;
            return true;
        case '<':
            *op = Op_LEQ;
            parser->cursor += 2;
            return true;
        default:
            if (nob_sv_eq(tk.payload, nob_sv_from_cstr("min"))) {
                *op = Op_MIN;
                parser->cursor += 1;
                return true;
            }

            if (nob_sv_eq(tk.payload, nob_sv_from_cstr("max"))) {
                *op = Op_MAX;
                parser->cursor += 1;
                return true;
            }

            if (nob_sv_eq(tk.payload, nob_sv_from_cstr("pow"))) {
                *op = Op_POW;
                parser->cursor += 1;
                return true;
            }
            return false;
    }
    return false;
}

static size_t
parse_sequence(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    AST_Node node = {0};

    bool is_sequence = eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("["));
    if (!is_sequence) { *ok = false; return (size_t)-1; }


    size_t nodes = parse_blocks(parser, ast_nodes, ok);
    if (!*ok) return (size_t)-1;

    if (!eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("]"))) {
        *ok = false;
        return (size_t)-1;
    }

    node = (AST_Node){
        .kind = AST_Node_Kind_SEQUENCE,
        .as.sequence.node = nodes,
    };
    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_branch(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    AST_Node node = {0};

    bool is_branch = eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("{"));
    if (!is_branch) { *ok = false; return (size_t)-1; }
    size_t nodes = parse_blocks(parser, ast_nodes, ok);
    if (!*ok) return (size_t)-1;

    if (!eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("|"))) {
        *ok = false;
        return (size_t)-1;
    }

    Token tk;
    if (!parser_peek(parser, &tk) || tk.kind != Token_Kind_IDENT) {
        *ok = false;
        return (size_t)-1;
    }
    parser->cursor += 1; // consume folder ident

    if (!eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("}"))) {
        *ok = false;
        return (size_t)-1;
    }

    node = (AST_Node){
        .kind = AST_Node_Kind_BRANCH,
        .as.branch.node = nodes,
        .as.branch.folder = tk.payload,
    };
    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_paren(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    AST_Node node = {0};

    bool is_paren = eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("("));
    if (!is_paren) { *ok = false; return (size_t)-1; }

    size_t nodes = parse_blocks(parser, ast_nodes, ok);
    if (!*ok) return (size_t)-1;

    if (!eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr(")"))) {
        *ok = false;
        return (size_t)-1;
    }

    node = (AST_Node){
        .kind = AST_Node_Kind_PAREN,
        .as.paren.node = nodes,
    };
    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_int(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    Token tk = parser->tokens.items[parser->cursor];
    parser->cursor += 1;

    char *value_str = nob_temp_strndup(tk.payload.data, tk.payload.count);
    int value = atoi(value_str);

    AST_Node node = (AST_Node){
        .kind = AST_Node_Kind_CONSTANT_INT64,
        .as.constant_int64.value = value,
    };
    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_float(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    Token tk = parser->tokens.items[parser->cursor];
    parser->cursor += 1;

    char *value_str = nob_temp_strndup(tk.payload.data, tk.payload.count);
    double value = atof(value_str);

    AST_Node node = (AST_Node){
        .kind = AST_Node_Kind_CONSTANT_FLOAT64,
        .as.constant_float64.value = value,
    };
    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_call_path(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    Token first = parser->tokens.items[parser->cursor];
    const char *start = first.payload.data;

    parser->cursor += 1;

    while (eat_tk(parser, Token_Kind_CHAR, nob_sv_from_cstr("/"))) {
        Token next;
        if (!parser_peek(parser, &next) || next.kind != Token_Kind_IDENT) {
            *ok = false;
            return (size_t)-1;
        }
        parser->cursor += 1;
    }

    Token last = parser->tokens.items[parser->cursor - 1];

    const char *end = last.payload.data + last.payload.count;
    size_t count = (size_t)(end - start);

    AST_Node node = (AST_Node){
        .kind = AST_Node_Kind_FUN_CALL,
        .as.fun_call.call_path = (Nob_String_View){ .count = count, .data = start },
    };

    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_unary(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    Op op;
    if (!parse_op(parser, &op)) {
        *ok = false;
        return (size_t)-1;
    }

    size_t block = parse_block(parser, ast_nodes, ok);
    if (!*ok) return (size_t)-1;

    AST_Node node = (AST_Node){
        .kind = AST_Node_Kind_UNARY_FUN,
        .as.unary_fun.op = op,
        .as.unary_fun.param = block,
    };
    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

static size_t
parse_block(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    // order matters (matches your original)
    size_t idx;

    idx = parse_sequence(parser, ast_nodes, ok);
    if (*ok) return idx;

    idx = parse_branch(parser, ast_nodes, ok);
    if (*ok) return idx;

    idx = parse_paren(parser, ast_nodes, ok);
    if (*ok) return idx;

    idx = parse_unary(parser, ast_nodes, ok);
    if (*ok) return idx;

    Token tk;
    if (!parser_peek(parser, &tk)) {
        *ok = false;
        return (size_t)-1;
    }

    if (tk.kind == Token_Kind_INT)   return parse_int(parser, ast_nodes, ok);
    if (tk.kind == Token_Kind_FLOAT) return parse_float(parser, ast_nodes, ok);
    if (tk.kind == Token_Kind_IDENT) return parse_call_path(parser, ast_nodes, ok);

    *ok = false;
    return (size_t)-1;
}

static size_t
parse_blocks(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    AST_Node node = {
        .kind = AST_Node_Kind_BLOCKS,
        .as.blocks = { (Indices){0} },
    };

    // block+
    while (true) {
        size_t block_index = parse_block(parser, ast_nodes, ok);
        if (!*ok) break;
        nob_da_append(&node.as.blocks.nodes, block_index);
    }

    nob_da_append(ast_nodes, node);
    *ok = true;
    return ast_nodes->count - 1;
}

size_t
parse(Parser* parser, AST_Nodes *ast_nodes, bool* ok)
{
    parser->cursor = 0;
    return parse_blocks(parser, ast_nodes, ok);
}
