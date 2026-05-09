#pragma once

#include "ir/core/Instruction.h"

#include <string>
#include <vector>

class BasicBlock;
class Function;

class IRBuilder
{
private:
    BasicBlock* insertBlock = nullptr;
    unsigned tempIndex = 0;

public:
    void setInsertPoint(BasicBlock* block)
    {
        insertBlock = block;
    }

    BasicBlock* getInsertBlock() const
    {
        return insertBlock;
    }

    bool currentBlockHasTerminator() const;
    std::string nextTempName();

    AllocaInst* createAlloca(Type* allocatedType, const std::string& name = "");
    LoadInst* createLoad(Type* loadedType, Value* ptr, const std::string& name = "");
    StoreInst* createStore(Value* value, Value* ptr);
    BinaryOperator* createBinary(Opcode op, Value* lhs, Value* rhs, const std::string& name = "");
    ICmpInst* createICmp(const std::string& pred, Value* lhs, Value* rhs, const std::string& name = "");
    FCmpInst* createFCmp(const std::string& pred, Value* lhs, Value* rhs, const std::string& name = "");
    BranchInst* createBr(BasicBlock* target);
    BranchInst* createCondBr(Value* condition, BasicBlock* trueBlock, BasicBlock* falseBlock);
    ReturnInst* createRet(Value* value);
    ReturnInst* createRetVoid();
    CallInst* createCall(Type* retTy, const std::string& calleeName, const std::vector<Value*>& args,
        const std::string& name = "");
    GEPInst* createGEP(Type* sourceElementType, Type* resultElementType, Value* basePtr,
        const std::vector<Value*>& indices, const std::string& name = "");
    CastInst* createCast(const std::string& castOp, Value* value, Type* destTy, const std::string& name = "");
};
