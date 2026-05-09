#include "ir/builder/IRBuilder.h"

#include "ir/core/BasicBlock.h"

bool IRBuilder::currentBlockHasTerminator() const
{
    return insertBlock && insertBlock->getTerminator();
}

std::string IRBuilder::nextTempName()
{
    return "t" + std::to_string(tempIndex++);
}

AllocaInst* IRBuilder::createAlloca(Type* allocatedType, const std::string& name)
{
    return new AllocaInst(allocatedType, insertBlock, name.empty() ? nextTempName() : name);
}

LoadInst* IRBuilder::createLoad(Type* loadedType, Value* ptr, const std::string& name)
{
    return new LoadInst(loadedType, ptr, insertBlock, name.empty() ? nextTempName() : name);
}

StoreInst* IRBuilder::createStore(Value* value, Value* ptr)
{
    return new StoreInst(value, ptr, insertBlock);
}

BinaryOperator* IRBuilder::createBinary(Opcode op, Value* lhs, Value* rhs, const std::string& name)
{
    return new BinaryOperator(op, lhs, rhs, insertBlock, name.empty() ? nextTempName() : name);
}

ICmpInst* IRBuilder::createICmp(const std::string& pred, Value* lhs, Value* rhs, const std::string& name)
{
    return new ICmpInst(pred, lhs, rhs, insertBlock, name.empty() ? nextTempName() : name);
}

FCmpInst* IRBuilder::createFCmp(const std::string& pred, Value* lhs, Value* rhs, const std::string& name)
{
    return new FCmpInst(pred, lhs, rhs, insertBlock, name.empty() ? nextTempName() : name);
}

BranchInst* IRBuilder::createBr(BasicBlock* target)
{
    return currentBlockHasTerminator() ? nullptr : new BranchInst(target, insertBlock);
}

BranchInst* IRBuilder::createCondBr(Value* condition, BasicBlock* trueBlock, BasicBlock* falseBlock)
{
    return currentBlockHasTerminator() ? nullptr : new BranchInst(condition, trueBlock, falseBlock, insertBlock);
}

ReturnInst* IRBuilder::createRet(Value* value)
{
    return currentBlockHasTerminator() ? nullptr : new ReturnInst(value, insertBlock);
}

ReturnInst* IRBuilder::createRetVoid()
{
    return currentBlockHasTerminator() ? nullptr : new ReturnInst(nullptr, insertBlock);
}

CallInst* IRBuilder::createCall(Type* retTy, const std::string& calleeName, const std::vector<Value*>& args,
    const std::string& name)
{
    std::string resultName = retTy && retTy->isVoidTy() ? "" : (name.empty() ? nextTempName() : name);
    return new CallInst(retTy, calleeName, args, insertBlock, resultName);
}

GEPInst* IRBuilder::createGEP(Type* sourceElementType, Type* resultElementType, Value* basePtr,
    const std::vector<Value*>& indices, const std::string& name)
{
    return new GEPInst(sourceElementType, resultElementType, basePtr, indices, insertBlock,
        name.empty() ? nextTempName() : name);
}

CastInst* IRBuilder::createCast(const std::string& castOp, Value* value, Type* destTy, const std::string& name)
{
    return new CastInst(castOp, value, destTy, insertBlock, name.empty() ? nextTempName() : name);
}
