#include "../include/types.h"

static Struct_Type
from_base_type(Base_Type t)
{
    Struct_Type type = {0};
    type.is_base_type = true;
    type.type = t;
    return type;
}

static Struct_Type
judge(IR_Nodes ir_nodes, size_t ir_root, Type_Env *env, Struct_Type input)
{
    IR_Node node = ir_nodes.items[ir_root];
    
    switch (node.kind) {
        case IR_Node_Kind_FUN: {
            for(size_t i = 0; i < env->funs.count; i+=1) {
                if (nob_sv_eq(node.name, env->funs.items[i].name)) {
                    return env->funs.items[i].to;
                }
            }
            break;
        }
        case IR_Node_Kind_CONCAT: {
            Struct_Type t1 = judge(ir_nodes, node.lhs, env, input);
            Struct_Type t2 = judge(ir_nodes, node.rhs, env, t1);
            return t2;
        }
        case IR_Node_Kind_FORK: {
            Struct_Type t1 = judge(ir_nodes, node.lhs, env, input);
            Struct_Type t2 = judge(ir_nodes, node.rhs, env, t1);
            return t2;
        }
        case IR_Node_Kind_FOLD: {
            Struct_Type t1 = judge(ir_nodes, node.lhs, env, input);
            return t1;
        }
        case IR_Node_Kind_UNFOLD: {
            Struct_Type t1 = judge(ir_nodes, node.lhs, env, input);
            for(size_t i = 0; i < env->funs.count; i+=1) {
                if (nob_sv_eq(node.name, env->funs.items[i].name)) {
                    return env->funs.items[i].to;
                }
            }
            return t1;
        }
        case IR_Node_Kind_UNARY: {
            Struct_Type t1 = judge(ir_nodes, node.lhs, env, input);
            return t1;
        }
        case IR_Node_Kind_FLOAT:
            return from_base_type(Base_Type_FLOAT32);
        case IR_Node_Kind_INT:
            return from_base_type(Base_Type_INT8);
    }
    return input;
}

void
augment_env(Type_Env *env)
{
    Fun_Signature fs = {0};

    Base_Types bs = {0};
    nob_da_append(&bs, Base_Type_INT8);
    nob_da_append(&bs, Base_Type_INT8);
    nob_da_append(&bs, Base_Type_INT8);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("rgb"),
        .from = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
        .to   = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("rgb/r"),
        .from   = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
        .to = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("rgb/b"),
        .from   = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
        .to = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("rgb/g"),
        .from   = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
        .to = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("u8/to_f32"),
        .to = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
        .from = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_FLOAT32,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("f32/to_u8"),
        .to = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_FLOAT32,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
        .from = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("sum"),
        .from   = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
        .to = (Struct_Type) {
            .is_base_type  = true,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 1,
            .intern_types  = (Base_Types) {0},
        },
    };
    nob_da_append(&env->funs, fs);

    fs = (Fun_Signature) {
        .name = nob_sv_from_cstr("concat"),
        .from   = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
        .to = (Struct_Type) {
            .is_base_type  = false,
            .type          = Base_Type_INT8,
            .name          = (Nob_String_View) {0},
            .size_in_bytes = 3,
            .intern_types  = bs,
        },
    };
    nob_da_append(&env->funs, fs);

}

bool
typecheck(IR_Nodes ir_nodes, size_t ir_root, Type_Env *env)
{
    augment_env(env);
    Struct_Type t = judge(ir_nodes, ir_root, env, from_base_type(Base_Type_INT8));
    return !t.is_base_type || t.type != Base_Type_INT8 || t.type != Base_Type_FLOAT32;
}
