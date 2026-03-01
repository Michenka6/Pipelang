#ifndef TYPES_H
#define TYPES_H

#include "ir.h"

typedef enum Base_Type {
    Base_Type_UINT8,
    Base_Type_INT8,
    Base_Type_UINT64,
    Base_Type_INT64,
    Base_Type_FLOAT32,
    Base_Type_FLOAT64,
    Base_Type_BOOL,
} Base_Type;

typedef struct Base_Types {
    Base_Type *items;
    size_t count;
    size_t capacity;
} Base_Types;

typedef struct Struct_Type {
    bool is_base_type;
    Base_Type type;
    Nob_String_View name;
    size_t      size_in_bytes;
    Base_Types  intern_types;
} Struct_Type;

typedef struct Struct_Types {
    Struct_Type *items;
    size_t count;
    size_t capacity;
} Struct_Types;

typedef struct Fun_Signature {
    Nob_String_View name;
    Struct_Type from;
    Struct_Type to;
} Fun_Signature;

typedef struct Fun_Signatures {
    Fun_Signature *items;
    size_t count;
    size_t capacity;
} Fun_Signatures;

typedef struct Type_Env {
    Struct_Types types;
    Fun_Signatures funs;
} Type_Env;

bool typecheck(IR_Nodes ir_nodes, size_t ir_root, Type_Env *env);

#endif // TYPES_H

