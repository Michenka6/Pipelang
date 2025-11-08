package IR

import "../AST"

Expr :: union {
    Fun,
    Concat,
    Epsillon,
    Fork,
    Fail,
    Fold,
    Unfold,
    Unary_Fun,
}

Fun :: struct {
    name: string
}

Concat :: struct {
    e1: ^Expr,
    e2: ^Expr,
}

Epsillon :: struct {}

Fork :: struct {
    e1: ^Expr,
    e2: ^Expr,
}

Fail :: struct {}

Fold :: struct {
    e: ^Expr,
    g: string,
}

Unfold :: struct {
    e: ^Expr,
    g: string
}

Unary_Fun :: struct {
    op: AST.Unary_Op,
    param:  ^AST.Constant
}
