#include "../include/pretty_printer.h"

static void
write_token(Nob_String_Builder* sb, Token token)
{
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "[%zu:%zu]", token.line, token.column);
    nob_sb_append_buf(sb, buf, (size_t)n);

    switch (token.kind)
    {
        case Token_Kind_INVALID:
            nob_sb_append_buf(sb, "INVALID", sizeof("INVALID") - 1);
            break;

        case Token_Kind_CHAR:
            nob_sb_append_cstr(sb, "\tCHAR:\t");
            nob_da_append(sb, '\'');
            nob_sb_append_buf(sb, token.payload.data, token.payload.count);
            nob_da_append(sb, '\'');
            break;

        case Token_Kind_INT:
            nob_sb_append_cstr(sb, "\tINT:\t");
            nob_sb_append_buf(sb, token.payload.data, token.payload.count);
            break;
        case Token_Kind_FLOAT:
            nob_sb_append_cstr(sb, "\tFLOAT:\t");
            nob_sb_append_buf(sb, token.payload.data, token.payload.count);
            break;
        case Token_Kind_IDENT:
            nob_sb_append_cstr(sb, "\tIDENT:\t");
            nob_da_append(sb, '\"');
            nob_sb_append_buf(sb, token.payload.data, token.payload.count);
            nob_da_append(sb, '\"');
            break;

        case Token_Kind_STRING_LITERAL:
            nob_sb_append_cstr(sb, "\tSTR_LIT:\t");
            nob_sb_append_buf(sb, "\"", 1);
            nob_sb_append_buf(sb, token.payload.data, token.payload.count);
            nob_sb_append_buf(sb, "\"", 1);
            break;

        case Token_Kind_EOF:
            nob_sb_append_buf(sb, "\t<EOF>", sizeof("\t<EOF>") - 1);
            break;

        case Token_Kind_COUNT:
            assert(false && "UNREACHABLE");
    }
}

void
pretty_print_tokens(Tokens tokens)
{
    static Nob_String_Builder sb = {0};
    for (size_t i = 0; i < tokens.count; i++) {
        sb.count = 0;
        write_token(&sb, tokens.items[i]);
        printf("%.*s\n", (int)sb.count, sb.items);
    }
}

void
print_indent(size_t level)
{
    for(size_t i = 0; i < level; i+=1) {
        printf(" ");
        printf(" ");
    }
}

void
pretty_print_ast(AST_Nodes ast_nodes, size_t ast_root, size_t level)
{
    if (ast_root >= ast_nodes.count) return;

    AST_Node node = ast_nodes.items[ast_root];
    print_indent(level);
    switch (node.kind) {
        case AST_Node_Kind_BLOCKS:
            printf("BLOCKS\n");
            Indices nodes = node.as.blocks.nodes;
            for(size_t i = 0; i < nodes.count; i += 1) {
                pretty_print_ast(ast_nodes, nodes.items[i], level+1);
            }
            break;
        case AST_Node_Kind_SEQUENCE:
            printf("SEQUENCE\n");
            pretty_print_ast(ast_nodes, node.as.sequence.node, level+1);
            break;
        case AST_Node_Kind_BRANCH:
            printf("BRANCH: %.*s\n", (int)node.as.branch.folder.count, node.as.branch.folder.data);
            pretty_print_ast(ast_nodes, node.as.branch.node, level+1);
            break;
        case AST_Node_Kind_FUN_CALL:
            printf("FUN_CALL: %.*s\n", (int)node.as.fun_call.call_path.count, node.as.fun_call.call_path.data);
            break;
        case AST_Node_Kind_UNARY_FUN:
            printf("UNARY_FUN: ");
            switch (node.as.unary_fun.op) {
                case Op_MINUS: printf("-");
                               break;
                case Op_TIMES: printf("*");
                               break;
                case Op_MIN: printf("max");
                               break;
                case Op_MAX: printf("min");
                               break;
                case Op_PLUS: printf("+");
                               break;
                case Op_DIVIDE: printf("/");
                               break;
                case Op_GR: printf(">");
                               break;
                case Op_LEQ: printf("<=");
                               break;
                case Op_POW: printf("pow");
                               break;
            }
            printf("\n");
            pretty_print_ast(ast_nodes, node.as.unary_fun.param, level+1);
            break;
        case AST_Node_Kind_PAREN:
            printf("PAREN\n");
            pretty_print_ast(ast_nodes, node.as.sequence.node, level+1);
            break;
        case AST_Node_Kind_CONSTANT_INT64:
            printf("INT64: %zu\n", node.as.constant_int64.value);
            break;
        case AST_Node_Kind_CONSTANT_FLOAT64:
            printf("FLOAT64: %f\n", node.as.constant_float64.value);
            break;
        case AST_Node_Kind_COUNT:
            assert(false && "unreachable");
            break;
    }
}

void
pretty_print_ir(IR_Nodes ir_nodes, size_t ir_root, size_t level)
{
    (void)level;
    IR_Node node = ir_nodes.items[ir_root];
    print_indent(level);
    switch (node.kind) {
        case IR_Node_Kind_FUN:
            printf("FUN: %.*s\n", (int)node.name.count, node.name.data);
            break;
        case IR_Node_Kind_CONCAT:
            printf("CONCAT\n");
            pretty_print_ir(ir_nodes, node.lhs, level+1);
            pretty_print_ir(ir_nodes, node.rhs, level+1);
            break;
        case IR_Node_Kind_FORK:
            printf("FORK\n");
            pretty_print_ir(ir_nodes, node.lhs, level+1);
            pretty_print_ir(ir_nodes, node.rhs, level+1);
            break;
        case IR_Node_Kind_FOLD:
            printf("FOLD\n");
            pretty_print_ir(ir_nodes, node.lhs, level+1);
            pretty_print_ir(ir_nodes, node.rhs, level+1);
            break;
        case IR_Node_Kind_UNFOLD:
            printf("UNFOLD: %.*s\n", (int)node.name.count, node.name.data);
            pretty_print_ir(ir_nodes, node.lhs, level+1);
            pretty_print_ir(ir_nodes, node.rhs, level+1);
            break;
        case IR_Node_Kind_UNARY:
            printf("UNARY: ");
            switch (node.op) {
                case Op_MINUS: printf("-");
                               break;
                case Op_TIMES: printf("*");
                               break;
                case Op_MIN: printf("max");
                               break;
                case Op_MAX: printf("min");
                               break;
                case Op_PLUS: printf("+");
                               break;
                case Op_DIVIDE: printf("/");
                               break;
                case Op_GR: printf(">");
                               break;
                case Op_LEQ: printf("<=");
                               break;
                case Op_POW: printf("pow");
                               break;
            }
            printf("\n");
            pretty_print_ir(ir_nodes, node.lhs, level+1);
            break;
        case IR_Node_Kind_FLOAT:
            printf("FLOAT: %f\n", node.fv);
            break;
        case IR_Node_Kind_INT:
            printf("INT: %zu\n", node.iv);
            break;
    }
}

void 
pretty_print_type(Struct_Type type)
{
    (void)type;
}
