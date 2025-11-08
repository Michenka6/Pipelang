package AST

Program :: struct {
    blocks: []Block
}

Block :: union #no_nil {
    Sequence,
    Branch,
    Function,
}

Sequence :: struct {
    blocks: []Block
}

Branch :: struct {
    folder: string,
    blocks: []Block,
}

Function :: union #no_nil {
    Call_Fun,
    Unary_Fun,
    Paren,
}

Paren :: struct {
    program: ^Program
}

Call_Fun :: struct {
    name: Function_Path,
}

Function_Path :: string

Unary_Fun :: struct {
    op:    Unary_Op,
    param: ^Constant,
}

Unary_Op :: enum {
    MINUS,
    TIMES,
}

Constant :: union {
    int,
    f32,
}
