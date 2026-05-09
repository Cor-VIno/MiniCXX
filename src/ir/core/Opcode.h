#pragma once

enum class Opcode
{
    Add,
    Sub,
    Mul,
    SDiv,
    SRem,
    FAdd,
    FSub,
    FMul,
    FDiv,
    Alloca,
    Load,
    Store,
    ICmp,
    FCmp,
    Br,
    Ret,
    Call,
    GEP,
    Cast,
    Phi,
    Select,
    InsertElement,
    ExtractElement
};
