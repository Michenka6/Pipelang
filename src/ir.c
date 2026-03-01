#include "../include/ir.h"

size_t ast_to_ir(AST_Nodes ast_nodes, size_t ast_root, IR_Nodes *ir_nodes)
{
    (void)ir_nodes;
    AST_Node node = ast_nodes.items[ast_root];
    switch (node.kind) {
        case AST_Node_Kind_BLOCKS: {
            size_t lhs = ast_to_ir(ast_nodes, node.as.blocks.nodes.items[0], ir_nodes);
            for(size_t i = 1; i < node.as.blocks.nodes.count; i += 1) {
                size_t rhs = ast_to_ir(ast_nodes, node.as.blocks.nodes.items[i], ir_nodes);
                IR_Node ir_node = {
                    .kind = IR_Node_Kind_CONCAT,
                    .name = nob_sv_from_cstr(""),
                    .lhs  = lhs,
                    .rhs  = rhs,
                    .iv   = 0,
                    .fv   = 0,
                    .op   = Op_MINUS,
                };
                nob_da_append(ir_nodes, ir_node);
                lhs = ir_nodes->count - 1;
            }
            break;
        }
        case AST_Node_Kind_SEQUENCE:
            ast_to_ir(ast_nodes, node.as.sequence.node, ir_nodes);
            break;
        case AST_Node_Kind_BRANCH: {
            size_t lhs = ast_to_ir(ast_nodes, node.as.branch.node, ir_nodes);
            IR_Node ir_node = {
                .kind = IR_Node_Kind_UNFOLD,
                .name = node.as.branch.folder,
                .lhs  = lhs,
                .rhs  = -1,
                .iv   = 0,
                .fv   = 0,
                .op   = Op_MINUS,
            };
            nob_da_append(ir_nodes, ir_node);
            break;
        }
        case AST_Node_Kind_FUN_CALL: {
            IR_Node ir_node = {
                .kind = IR_Node_Kind_FUN,
                .name = node.as.fun_call.call_path,
                .lhs  = -1,
                .rhs  = -1,
                .iv   = node.as.constant_int64.value,
                .fv   = 0,
                .op   = Op_MINUS,
            };
            nob_da_append(ir_nodes, ir_node);
            break;
        }
        case AST_Node_Kind_UNARY_FUN: {
            size_t lhs = ast_to_ir(ast_nodes, node.as.unary_fun.param, ir_nodes);
            IR_Node ir_node = {
                .kind = IR_Node_Kind_UNARY,
                .name = nob_sv_from_cstr(""),
                .lhs  = lhs,
                .rhs  = -1,
                .iv   = 0,
                .fv   = 0,
                .op   = node.as.unary_fun.op,
            };
            nob_da_append(ir_nodes, ir_node);
            break;
        }
        case AST_Node_Kind_PAREN:
            ast_to_ir(ast_nodes, node.as.paren.node, ir_nodes);
            break;
        case AST_Node_Kind_CONSTANT_INT64: {
            IR_Node ir_node = {
                .kind = IR_Node_Kind_INT,
                .name = nob_sv_from_cstr(""),
                .lhs  = -1,
                .rhs  = -1,
                .iv   = node.as.constant_int64.value,
                .fv   = 0,
                .op   = Op_MINUS,
            };
            nob_da_append(ir_nodes, ir_node);
            break;
        }
        case AST_Node_Kind_CONSTANT_FLOAT64: {
            IR_Node ir_node = {
                .kind = IR_Node_Kind_FLOAT,
                .name = nob_sv_from_cstr(""),
                .lhs  = -1,
                .rhs  = -1,
                .iv   = 0,
                .fv   = node.as.constant_float64.value,
                .op   = Op_MINUS,
            };
            nob_da_append(ir_nodes, ir_node);
            break;
        }
        case AST_Node_Kind_COUNT:
            assert(false && "unreachable");
            break;
    }
    return ir_nodes->count - 1;
}

size_t
ir_optimize_dse(IR_Nodes *in, size_t root, IR_Nodes *out)
{
    assert(in && out);
    assert(root < in->count);

    const size_t n = in->count;

    bool *mark = (bool*)calloc(n, sizeof(bool));
    assert(mark);

    size_t *stack = (size_t*)malloc(n * sizeof(size_t));
    assert(stack);

    size_t sp = 0;
    stack[sp++] = root;

    while (sp) {
        size_t v = stack[--sp];
        if (v == (size_t)-1 || v >= n) continue;
        if (mark[v]) continue;
        mark[v] = true;

        IR_Node node = in->items[v];

        switch (node.kind) {
            case IR_Node_Kind_CONCAT:
            case IR_Node_Kind_FORK:
                if (node.lhs != (size_t)-1) stack[sp++] = node.lhs;
                if (node.rhs != (size_t)-1) stack[sp++] = node.rhs;
                break;

            case IR_Node_Kind_FOLD:
            case IR_Node_Kind_UNFOLD:
            case IR_Node_Kind_UNARY:
                if (node.lhs != (size_t)-1) stack[sp++] = node.lhs;
                break;

            case IR_Node_Kind_FUN:
            case IR_Node_Kind_INT:
            case IR_Node_Kind_FLOAT:
                break;
        }
    }

    size_t *remap = (size_t*)malloc(n * sizeof(size_t));
    assert(remap);

    for (size_t i = 0; i < n; i++) remap[i] = (size_t)-1;

    for (size_t i = 0; i < n; i++) {
        if (!mark[i]) continue;
        remap[i] = out->count;
        nob_da_append(out, in->items[i]);
    }

    for (size_t i = 0; i < out->count; i++) {
        IR_Node *node = &out->items[i];

        switch (node->kind) {
            case IR_Node_Kind_CONCAT:
            case IR_Node_Kind_FORK:
                if (node->lhs != (size_t)-1) node->lhs = remap[node->lhs];
                if (node->rhs != (size_t)-1) node->rhs = remap[node->rhs];
                break;

            case IR_Node_Kind_FOLD:
            case IR_Node_Kind_UNFOLD:
            case IR_Node_Kind_UNARY:
                if (node->lhs != (size_t)-1) node->lhs = remap[node->lhs];
                break;

            case IR_Node_Kind_FUN:
            case IR_Node_Kind_INT:
            case IR_Node_Kind_FLOAT:
                break;
        }
    }

    size_t new_root = remap[root];
    assert(new_root != (size_t)-1);

    free(remap);
    free(stack);
    free(mark);

    return new_root;
}
