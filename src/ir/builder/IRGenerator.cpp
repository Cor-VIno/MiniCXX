#include "ir/builder/IRGenerator.h"

#include "frontend/ast/AST.h"
#include "ir/constants/ConstantFloat.h"
#include "ir/constants/ConstantInt.h"
#include "ir/constants/Constant.h"
#include "ir/constants/ConstantArray.h"
#include "ir/constants/ConstantCString.h"
#include "ir/constants/ConstantZeroInitializer.h"
#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/GlobalVariable.h"
#include "ir/core/LibFunction.h"
#include "ir/types/ArrayType.h"
#include "ir/types/FunctionType.h"
#include "ir/types/PointerType.h"

#include <cstdlib>
#include <stdexcept>

static bool literalIsFloat(const std::string& value)
{
    return value.find('.') != std::string::npos
        || value.find('e') != std::string::npos
        || value.find('E') != std::string::npos
        || value.find('p') != std::string::npos
        || value.find('P') != std::string::npos
        || value.find('f') != std::string::npos
        || value.find('F') != std::string::npos;
}

IRGenerator::IRGenerator()
{
    declareBuiltins();
}

void IRGenerator::enterScope()
{
    scopes.push_back({});
}

void IRGenerator::exitScope()
{
    if (!scopes.empty())
    {
        scopes.pop_back();
    }
}

void IRGenerator::declareBuiltins()
{
    auto add = [this](const std::string& name, const std::string& ret, std::vector<Type*> params,
        bool variadic = false)
    {
        FunctionInfo info{ typeFromName(ret), params, variadic };
        registerFunction(name, info);
        module.addFunction(new LibFunction(FunctionType::get(info.returnType, info.paramTypes), name, variadic));
    };

    Type* intTy = Type::getInt32Ty();
    Type* int8Ty = Type::getInt8Ty();
    Type* floatTy = Type::getFloatTy();
    add("getint", "int", {});
    add("getch", "int", {});
    add("getfloat", "float", {});
    add("getarray", "int", { PointerType::get(intTy) });
    add("getfarray", "int", { PointerType::get(floatTy) });
    add("putint", "void", { intTy });
    add("putch", "void", { intTy });
    add("putfloat", "void", { floatTy });
    add("putarray", "void", { intTy, PointerType::get(intTy) });
    add("putfarray", "void", { intTy, PointerType::get(floatTy) });
    add("putf", "void", { PointerType::get(int8Ty) }, true);
    add("_sysy_starttime", "void", { intTy });
    add("_sysy_stoptime", "void", { intTy });
}

void IRGenerator::collectGlobalConstant(StmtAST* stmt)
{
    if (auto* group = dynamic_cast<DeclGroupAST*>(stmt))
    {
        for (const auto& decl : group->declarations)
        {
            collectGlobalConstant(decl.get());
        }
        return;
    }

    auto* varDecl = dynamic_cast<VarDeclStmtAST*>(stmt);
    if (!varDecl || !varDecl->isConst || !varDecl->initValue)
    {
        return;
    }
    Type* type = typeFromName(varDecl->varType);
    if (Constant* value = evaluateConstantInitializer(varDecl->initValue.get(), type))
    {
        constantValues[varDecl->varName] = value;
    }
}

void IRGenerator::registerFunction(const std::string& name, const FunctionInfo& info)
{
    functions[name] = info;
}

IRGenerator::Symbol* IRGenerator::lookupSymbol(const std::string& name)
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        auto found = it->find(name);
        if (found != it->end())
        {
            return &found->second;
        }
    }
    return nullptr;
}

BasicBlock* IRGenerator::createBlock(const std::string& prefix)
{
    auto* block = new BasicBlock(prefix + std::to_string(blockIndex++), currentFunction);
    if (currentFunction)
    {
        currentFunction->addBasicBlock(block);
    }
    return block;
}

void IRGenerator::ensureBranchTo(BasicBlock* target)
{
    if (builder.getInsertBlock() && !builder.currentBlockHasTerminator())
    {
        builder.createBr(target);
    }
}

Type* IRGenerator::typeFromName(const std::string& name) const
{
    if (name == "void") return Type::getVoidTy();
    if (name == "float") return Type::getFloatTy();
    return Type::getInt32Ty();
}

Type* IRGenerator::arrayTypeFromDimensions(Type* elementType, const std::vector<int>& dimensions) const
{
    Type* result = elementType;
    for (auto it = dimensions.rbegin(); it != dimensions.rend(); ++it)
    {
        result = ArrayType::get(result, static_cast<uint64_t>(*it));
    }
    return result;
}

Type* IRGenerator::arrayParamStorageType(const FunctionParamAST& param) const
{
    std::vector<int> dimensions;
    for (const auto& dim : param.dimensions)
    {
        dimensions.push_back(evaluateConstantIntDimension(dim.get()));
    }
    return arrayTypeFromDimensions(typeFromName(param.type), dimensions);
}

Type* IRGenerator::elementTypeAfterIndices(Type* aggregateType, size_t indexCount) const
{
    Type* result = aggregateType;
    for (size_t i = 0; i < indexCount; ++i)
    {
        if (auto* arrayTy = dynamic_cast<ArrayType*>(result))
        {
            result = arrayTy->getElementType();
        }
    }
    return result;
}

Value* IRGenerator::emitStringLiteral(const std::string& value)
{
    auto* initializer = new ConstantCString(value);
    Type* stringType = initializer->getType();
    auto* global = new GlobalVariable(stringType, ".str." + std::to_string(stringIndex++), initializer, true);
    module.addGlobal(global);
    return builder.createGEP(stringType, Type::getInt8Ty(), global,
        { ConstantInt::get(0), ConstantInt::get(0) });
}

Constant* IRGenerator::lookupConstantValue(const std::string& name) const
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        auto found = it->find(name);
        if (found != it->end())
        {
            return found->second.constValue;
        }
    }
    auto found = constantValues.find(name);
    return found == constantValues.end() ? nullptr : found->second;
}

int IRGenerator::evaluateConstantIntDimension(ExprAST* expr) const
{
    if (auto* value = dynamic_cast<ConstantInt*>(
        evaluateConstantInitializer(expr, Type::getInt32Ty())))
    {
        return value->getValue();
    }
    return 1;
}

Constant* IRGenerator::evaluateConstantInitializer(ExprAST* expr, Type* targetType) const
{
    if (!expr || !targetType)
    {
        return nullptr;
    }
    if (auto* number = dynamic_cast<NumberExprAST*>(expr))
    {
        if (targetType->isFloatTy())
        {
            return ConstantFloat::get(std::strtof(number->value.c_str(), nullptr));
        }
        return ConstantInt::get(static_cast<int>(std::strtol(number->value.c_str(), nullptr, 0)));
    }
    if (auto* variable = dynamic_cast<VariableExprAST*>(expr))
    {
        return lookupConstantValue(variable->name);
    }
    if (auto* unary = dynamic_cast<UnaryExprAST*>(expr))
    {
        Constant* operand = evaluateConstantInitializer(unary->operand.get(), targetType);
        if (auto* intValue = dynamic_cast<ConstantInt*>(operand))
        {
            int value = intValue->getValue();
            if (unary->op == "-") value = -value;
            else if (unary->op == "!") value = !value;
            return ConstantInt::get(value);
        }
        if (auto* floatValue = dynamic_cast<ConstantFloat*>(operand))
        {
            float value = floatValue->getValue();
            if (unary->op == "-") value = -value;
            return ConstantFloat::get(value);
        }
        return operand;
    }
    if (auto* binary = dynamic_cast<BinaryExprAST*>(expr))
    {
        Constant* lhsConst = evaluateConstantInitializer(binary->LHS.get(), targetType);
        Constant* rhsConst = evaluateConstantInitializer(binary->RHS.get(), targetType);
        auto* lhsInt = dynamic_cast<ConstantInt*>(lhsConst);
        auto* rhsInt = dynamic_cast<ConstantInt*>(rhsConst);
        if (!lhsInt || !rhsInt)
        {
            return nullptr;
        }
        int lhs = lhsInt->getValue();
        int rhs = rhsInt->getValue();
        if (binary->op == "+") return ConstantInt::get(lhs + rhs);
        if (binary->op == "-") return ConstantInt::get(lhs - rhs);
        if (binary->op == "*") return ConstantInt::get(lhs * rhs);
        if (binary->op == "/" && rhs != 0) return ConstantInt::get(lhs / rhs);
        if (binary->op == "%" && rhs != 0) return ConstantInt::get(lhs % rhs);
        if (binary->op == "<") return ConstantInt::get(lhs < rhs);
        if (binary->op == "<=") return ConstantInt::get(lhs <= rhs);
        if (binary->op == ">") return ConstantInt::get(lhs > rhs);
        if (binary->op == ">=") return ConstantInt::get(lhs >= rhs);
        if (binary->op == "==") return ConstantInt::get(lhs == rhs);
        if (binary->op == "!=") return ConstantInt::get(lhs != rhs);
        if (binary->op == "&&") return ConstantInt::get(lhs && rhs);
        if (binary->op == "||") return ConstantInt::get(lhs || rhs);
    }
    return nullptr;
}

Constant* IRGenerator::evaluateConstantArrayInitializer(ExprAST* init, Type* arrayType, Type* scalarType,
    const std::vector<int>& dimensions) const
{
    size_t totalElements = 1;
    for (int dim : dimensions)
    {
        totalElements *= static_cast<size_t>(dim);
    }
    std::vector<Constant*> flatValues(totalElements, dynamic_cast<Constant*>(defaultValue(scalarType)));
    size_t linearIndex = 0;
    collectArrayConstantInitializers(init, scalarType, dimensions, flatValues, 0, linearIndex);
    size_t offset = 0;
    return buildConstantArray(arrayType, scalarType, dimensions, flatValues, 0, offset);
}

void IRGenerator::collectArrayConstantInitializers(ExprAST* init, Type* scalarType,
    const std::vector<int>& dimensions, std::vector<Constant*>& flatValues, size_t depth, size_t& linearIndex) const
{
    if (!init)
    {
        return;
    }
    if (auto* list = dynamic_cast<InitListExprAST*>(init))
    {
        size_t childBlockSize = 1;
        for (size_t i = depth + 1; i < dimensions.size(); ++i)
        {
            childBlockSize *= static_cast<size_t>(dimensions[i]);
        }
        for (const auto& element : list->elements)
        {
            if (linearIndex >= flatValues.size())
            {
                return;
            }
            if (dynamic_cast<InitListExprAST*>(element.get()) && depth + 1 < dimensions.size())
            {
                size_t alignedStart = childBlockSize == 0 ? linearIndex
                    : ((linearIndex + childBlockSize - 1) / childBlockSize) * childBlockSize;
                linearIndex = alignedStart;
                collectArrayConstantInitializers(element.get(), scalarType, dimensions,
                    flatValues, depth + 1, linearIndex);
                linearIndex = alignedStart + childBlockSize;
            }
            else
            {
                collectArrayConstantInitializers(element.get(), scalarType, dimensions,
                    flatValues, dimensions.size(), linearIndex);
            }
        }
        return;
    }

    if (linearIndex < flatValues.size())
    {
        Constant* value = evaluateConstantInitializer(init, scalarType);
        if (value)
        {
            flatValues[linearIndex] = value;
        }
    }
    ++linearIndex;
}

Constant* IRGenerator::buildConstantArray(Type* aggregateType, Type* scalarType,
    const std::vector<int>& dimensions, const std::vector<Constant*>& flatValues,
    size_t depth, size_t& offset) const
{
    if (depth >= dimensions.size())
    {
        return offset < flatValues.size() ? flatValues[offset++] : dynamic_cast<Constant*>(defaultValue(scalarType));
    }
    auto* arrayType = dynamic_cast<ArrayType*>(aggregateType);
    Type* elementType = arrayType ? arrayType->getElementType() : scalarType;
    std::vector<Constant*> elements;
    for (int i = 0; i < dimensions[depth]; ++i)
    {
        elements.push_back(buildConstantArray(elementType, scalarType, dimensions, flatValues, depth + 1, offset));
    }
    return new ConstantArray(aggregateType, std::move(elements));
}

Value* IRGenerator::defaultValue(Type* type) const
{
    if (!type || type->isVoidTy())
    {
        return nullptr;
    }
    if (type->isFloatTy())
    {
        return ConstantFloat::get(0.0f);
    }
    if (type->isArrayTy())
    {
        return ConstantZeroInitializer::get(type);
    }
    return ConstantInt::get(0);
}

Value* IRGenerator::castTo(Value* value, Type* targetType)
{
    if (!value || !targetType || value->getType() == targetType)
    {
        return value;
    }
    const std::string from = value->getTypeName();
    const std::string to = targetType->toString();
    if (from == to)
    {
        return value;
    }
    if (from == "i1" && to == "i32")
    {
        return builder.createCast("zext", value, targetType);
    }
    if (value->getType()->isIntegerTy() && targetType->isFloatTy())
    {
        return builder.createCast("sitofp", value, targetType);
    }
    if (value->getType()->isFloatTy() && targetType->isIntegerTy())
    {
        return builder.createCast("fptosi", value, targetType);
    }
    return value;
}

Value* IRGenerator::castToCondition(Value* value)
{
    if (!value)
    {
        return ConstantInt::getBool(false);
    }
    if (value->getTypeName() == "i1")
    {
        return value;
    }
    if (value->getType()->isFloatTy())
    {
        return builder.createFCmp("one", value, ConstantFloat::get(0.0f));
    }
    return builder.createICmp("ne", value, ConstantInt::get(0));
}

Value* IRGenerator::emitArrayElementAddress(const Symbol& symbol,
    const std::vector<std::unique_ptr<ExprAST>>& indices, Type** elementType)
{
    std::vector<Value*> gepIndices;
    Type* sourceType = symbol.valueType;
    if (!symbol.isArrayParam)
    {
        gepIndices.push_back(ConstantInt::get(0));
    }
    for (const auto& index : indices)
    {
        gepIndices.push_back(castTo(index->accept(*this), Type::getInt32Ty()));
    }

    size_t unwrapCount = symbol.isArrayParam
        ? (indices.empty() ? 0 : indices.size() - 1)
        : indices.size();
    Type* resultType = elementTypeAfterIndices(sourceType, unwrapCount);
    if (elementType)
    {
        *elementType = resultType;
    }
    return builder.createGEP(sourceType, resultType, symbol.address, gepIndices);
}

Value* IRGenerator::emitLValueAddress(LValueAST* lvalue, Type** elementType)
{
    if (auto* var = dynamic_cast<VariableExprAST*>(lvalue))
    {
        Symbol* symbol = lookupSymbol(var->name);
        if (!symbol)
        {
            return nullptr;
        }
        if (elementType)
        {
            *elementType = symbol->valueType;
        }
        return symbol->address;
    }
    if (auto* access = dynamic_cast<ArrayAccessExprAST*>(lvalue))
    {
        Symbol* symbol = lookupSymbol(access->arrayName);
        if (!symbol)
        {
            return nullptr;
        }
        return emitArrayElementAddress(*symbol, access->indices, elementType);
    }
    return nullptr;
}

void IRGenerator::emitArrayInitStores(const Symbol& symbol, ExprAST* init, Type* scalarType)
{
    if (!init)
    {
        return;
    }
    size_t linearIndex = 0;
    emitArrayInitStoresRecursive(symbol, init, scalarType, 0, linearIndex);
}

void IRGenerator::emitArrayInitStoresRecursive(const Symbol& symbol, ExprAST* init, Type* scalarType,
    size_t depth, size_t& linearIndex)
{
    if (!init)
    {
        return;
    }
    if (auto* list = dynamic_cast<InitListExprAST*>(init))
    {
        size_t totalElements = 1;
        for (int dim : symbol.dimensions)
        {
            totalElements *= static_cast<size_t>(dim);
        }
        size_t childBlockSize = 1;
        for (size_t i = depth + 1; i < symbol.dimensions.size(); ++i)
        {
            childBlockSize *= static_cast<size_t>(symbol.dimensions[i]);
        }

        for (const auto& element : list->elements)
        {
            if (linearIndex >= totalElements)
            {
                return;
            }
            if (dynamic_cast<InitListExprAST*>(element.get()) && depth + 1 < symbol.dimensions.size())
            {
                size_t alignedStart = childBlockSize == 0 ? linearIndex
                    : ((linearIndex + childBlockSize - 1) / childBlockSize) * childBlockSize;
                linearIndex = alignedStart;
                emitArrayInitStoresRecursive(symbol, element.get(), scalarType, depth + 1, linearIndex);
                linearIndex = alignedStart + childBlockSize;
            }
            else
            {
                emitArrayInitStoresRecursive(symbol, element.get(), scalarType, symbol.dimensions.size(), linearIndex);
            }
        }
        return;
    }
    emitArrayScalarStore(symbol, init, scalarType, linearIndex);
    ++linearIndex;
}

void IRGenerator::emitArrayScalarStore(const Symbol& symbol, ExprAST* init, Type* scalarType, size_t linearIndex)
{
    if (symbol.dimensions.empty())
    {
        return;
    }

    size_t totalElements = 1;
    for (int dim : symbol.dimensions)
    {
        totalElements *= static_cast<size_t>(dim);
    }
    if (linearIndex >= totalElements)
    {
        return;
    }

    std::vector<Value*> indices;
    size_t remainder = linearIndex;
    for (size_t i = 0; i < symbol.dimensions.size(); ++i)
    {
        size_t stride = 1;
        for (size_t j = i + 1; j < symbol.dimensions.size(); ++j)
        {
            stride *= static_cast<size_t>(symbol.dimensions[j]);
        }
        size_t index = stride == 0 ? 0 : remainder / stride;
        remainder = stride == 0 ? 0 : remainder % stride;
        indices.push_back(ConstantInt::get(static_cast<int>(index)));
    }

    std::vector<Value*> gepIndices;
    gepIndices.push_back(ConstantInt::get(0));
    gepIndices.insert(gepIndices.end(), indices.begin(), indices.end());
    Value* address = builder.createGEP(symbol.valueType, scalarType, symbol.address, gepIndices);
    Value* value = castTo(init->accept(*this), scalarType);
    builder.createStore(value, address);
}

IRGenerator::FunctionInfo IRGenerator::makeFunctionInfo(const std::string& returnType,
    const std::vector<FunctionParamAST>& params) const
{
    FunctionInfo info;
    info.returnType = typeFromName(returnType);
    for (const auto& param : params)
    {
        Type* paramType = param.isArray ? arrayParamStorageType(param) : typeFromName(param.type);
        info.paramTypes.push_back(param.isArray ? PointerType::get(paramType) : paramType);
    }
    return info;
}

Value* IRGenerator::visitProgram(ProgramAST* node)
{
    enterScope();
    for (const auto& decl : node->declarations)
    {
        collectGlobalConstant(decl.get());
    }
    for (const auto& decl : node->declarations)
    {
        if (auto* func = dynamic_cast<FunctionDeclAST*>(decl.get()))
        {
            registerFunction(func->functionName, makeFunctionInfo(func->returnType, func->parameters));
        }
        else if (auto* funcDecl = dynamic_cast<FunctionDeclStmtAST*>(decl.get()))
        {
            registerFunction(funcDecl->functionName, makeFunctionInfo(funcDecl->returnType, funcDecl->parameters));
        }
    }
    for (const auto& decl : node->declarations)
    {
        decl->accept(*this);
    }
    exitScope();
    return nullptr;
}

Value* IRGenerator::visitNumberExpr(NumberExprAST* node)
{
    if (literalIsFloat(node->value))
    {
        return ConstantFloat::get(std::strtof(node->value.c_str(), nullptr));
    }
    return ConstantInt::get(static_cast<int>(std::strtol(node->value.c_str(), nullptr, 0)));
}

Value* IRGenerator::visitVariableExpr(VariableExprAST* node)
{
    Symbol* symbol = lookupSymbol(node->name);
    if (!symbol)
    {
        return nullptr;
    }
    if (symbol->isArray)
    {
        return symbol->address;
    }
    return builder.createLoad(symbol->valueType, symbol->address);
}

Value* IRGenerator::visitUnaryExpr(UnaryExprAST* node)
{
    Value* operand = node->operand ? node->operand->accept(*this) : nullptr;
    if (!operand)
    {
        return nullptr;
    }
    if (node->op == "+")
    {
        return operand;
    }
    if (node->op == "!")
    {
        return castTo(castToCondition(operand), Type::getInt32Ty());
    }
    if (operand->getType()->isFloatTy())
    {
        return builder.createBinary(Opcode::FSub, ConstantFloat::get(0.0f), operand);
    }
    return builder.createBinary(Opcode::Sub, ConstantInt::get(0), operand);
}

Value* IRGenerator::visitBinaryExpr(BinaryExprAST* node)
{
    Value* lhs = node->LHS ? node->LHS->accept(*this) : nullptr;
    if (!lhs)
    {
        return nullptr;
    }

    if ((node->op == "&&" || node->op == "||") && currentFunction)
    {
        auto* resultSlot = builder.createAlloca(Type::getInt32Ty());
        builder.createStore(ConstantInt::get(node->op == "||" ? 1 : 0), resultSlot);

        BasicBlock* rhsBlock = createBlock(node->op == "&&" ? "land.rhs" : "lor.rhs");
        BasicBlock* endBlock = createBlock(node->op == "&&" ? "land.end" : "lor.end");
        Value* lhsCond = castToCondition(lhs);
        if (node->op == "&&")
        {
            builder.createCondBr(lhsCond, rhsBlock, endBlock);
        }
        else
        {
            builder.createCondBr(lhsCond, endBlock, rhsBlock);
        }

        builder.setInsertPoint(rhsBlock);
        Value* rhs = node->RHS ? node->RHS->accept(*this) : nullptr;
        if (rhs)
        {
            builder.createStore(castTo(castToCondition(rhs), Type::getInt32Ty()), resultSlot);
        }
        ensureBranchTo(endBlock);

        builder.setInsertPoint(endBlock);
        return builder.createLoad(Type::getInt32Ty(), resultSlot);
    }

    Value* rhs = node->RHS ? node->RHS->accept(*this) : nullptr;
    if (!rhs)
    {
        return nullptr;
    }

    bool useFloat = lhs->getType()->isFloatTy() || rhs->getType()->isFloatTy();
    Type* opType = useFloat ? Type::getFloatTy() : Type::getInt32Ty();
    lhs = castTo(lhs, opType);
    rhs = castTo(rhs, opType);

    if (node->op == "+") return builder.createBinary(useFloat ? Opcode::FAdd : Opcode::Add, lhs, rhs);
    if (node->op == "-") return builder.createBinary(useFloat ? Opcode::FSub : Opcode::Sub, lhs, rhs);
    if (node->op == "*") return builder.createBinary(useFloat ? Opcode::FMul : Opcode::Mul, lhs, rhs);
    if (node->op == "/") return builder.createBinary(useFloat ? Opcode::FDiv : Opcode::SDiv, lhs, rhs);
    if (node->op == "%") return builder.createBinary(Opcode::SRem, lhs, rhs);

    std::string pred;
    if (node->op == "==") pred = useFloat ? "oeq" : "eq";
    else if (node->op == "!=") pred = useFloat ? "one" : "ne";
    else if (node->op == "<") pred = useFloat ? "olt" : "slt";
    else if (node->op == "<=") pred = useFloat ? "ole" : "sle";
    else if (node->op == ">") pred = useFloat ? "ogt" : "sgt";
    else if (node->op == ">=") pred = useFloat ? "oge" : "sge";

    if (!pred.empty())
    {
        Value* cmp = useFloat ? static_cast<Value*>(builder.createFCmp(pred, lhs, rhs))
            : static_cast<Value*>(builder.createICmp(pred, lhs, rhs));
        return castTo(cmp, Type::getInt32Ty());
    }
    return nullptr;
}

Value* IRGenerator::visitStringExpr(StringExprAST*)
{
    return nullptr;
}

Value* IRGenerator::visitCallExpr(CallExprAST* node)
{
    std::vector<Value*> args;
    std::string calleeName = node->name;
    if (node->name == "starttime" || node->name == "stoptime")
    {
        calleeName = node->name == "starttime" ? "_sysy_starttime" : "_sysy_stoptime";
        args.push_back(ConstantInt::get(node->sourceLine > 0 ? node->sourceLine : 0));
        return builder.createCall(Type::getVoidTy(), calleeName, args);
    }

    auto found = functions.find(node->name);
    for (size_t i = 0; i < node->args.size(); ++i)
    {
        const auto& arg = node->args[i];
        if (auto* str = dynamic_cast<StringExprAST*>(arg.get()))
        {
            args.push_back(emitStringLiteral(str->value));
            continue;
        }
        bool expectsPointer = found != functions.end()
            && i < found->second.paramTypes.size()
            && found->second.paramTypes[i]->isPointerTy();
        if (expectsPointer)
        {
            if (auto* var = dynamic_cast<VariableExprAST*>(arg.get()))
            {
                Symbol* symbol = lookupSymbol(var->name);
                if (symbol && symbol->isArray && !symbol->isArrayParam)
                {
                    Type* firstElementType = elementTypeAfterIndices(symbol->valueType, 1);
                    args.push_back(builder.createGEP(symbol->valueType, firstElementType, symbol->address,
                        { ConstantInt::get(0), ConstantInt::get(0) }));
                    continue;
                }
            }
        }
        args.push_back(arg->accept(*this));
    }
    Type* retTy = found == functions.end() ? Type::getInt32Ty() : found->second.returnType;
    if (found != functions.end())
    {
        for (size_t i = 0; i < args.size() && i < found->second.paramTypes.size(); ++i)
        {
            args[i] = castTo(args[i], found->second.paramTypes[i]);
        }
    }
    return builder.createCall(retTy, calleeName, args);
}

Value* IRGenerator::visitArrayAccessExpr(ArrayAccessExprAST* node)
{
    Symbol* symbol = lookupSymbol(node->arrayName);
    if (!symbol)
    {
        return nullptr;
    }
    Type* elementType = nullptr;
    Value* address = emitArrayElementAddress(*symbol, node->indices, &elementType);
    if (!elementType || elementType->isArrayTy())
    {
        return address;
    }
    return builder.createLoad(elementType, address);
}

Value* IRGenerator::visitMemberAccessExpr(MemberAccessExprAST*)
{
    return nullptr;
}

Value* IRGenerator::visitInitListExpr(InitListExprAST*)
{
    return nullptr;
}

Value* IRGenerator::visitExprStmt(ExprStmtAST* node)
{
    return node->expr ? node->expr->accept(*this) : nullptr;
}

Value* IRGenerator::visitEmptyStmt(EmptyStmtAST*)
{
    return nullptr;
}

Value* IRGenerator::visitAssignStmt(AssignStmtAST* node)
{
    Type* targetType = nullptr;
    Value* address = emitLValueAddress(node->target.get(), &targetType);
    Value* value = node->value ? node->value->accept(*this) : nullptr;
    if (address && value)
    {
        builder.createStore(castTo(value, targetType), address);
    }
    return nullptr;
}

Value* IRGenerator::visitBlockStmt(BlockStmtAST* node)
{
    enterScope();
    for (const auto& stmt : node->statements)
    {
        if (builder.currentBlockHasTerminator())
        {
            break;
        }
        stmt->accept(*this);
    }
    exitScope();
    return nullptr;
}

Value* IRGenerator::visitIfStmt(IfStmtAST* node)
{
    BasicBlock* thenBlock = createBlock("if.then");
    BasicBlock* elseBlock = node->elseBranch ? createBlock("if.else") : nullptr;
    BasicBlock* endBlock = createBlock("if.end");
    builder.createCondBr(castToCondition(node->condition->accept(*this)), thenBlock,
        elseBlock ? elseBlock : endBlock);

    builder.setInsertPoint(thenBlock);
    if (node->thenBranch)
    {
        node->thenBranch->accept(*this);
    }
    ensureBranchTo(endBlock);

    if (elseBlock)
    {
        builder.setInsertPoint(elseBlock);
        node->elseBranch->accept(*this);
        ensureBranchTo(endBlock);
    }

    builder.setInsertPoint(endBlock);
    return nullptr;
}

Value* IRGenerator::visitWhileStmt(WhileStmtAST* node)
{
    BasicBlock* condBlock = createBlock("while.cond");
    BasicBlock* bodyBlock = createBlock("while.body");
    BasicBlock* endBlock = createBlock("while.end");
    ensureBranchTo(condBlock);

    builder.setInsertPoint(condBlock);
    builder.createCondBr(castToCondition(node->condition->accept(*this)), bodyBlock, endBlock);

    builder.setInsertPoint(bodyBlock);
    breakTargets.push_back(endBlock);
    continueTargets.push_back(condBlock);
    if (node->body)
    {
        node->body->accept(*this);
    }
    breakTargets.pop_back();
    continueTargets.pop_back();
    ensureBranchTo(condBlock);

    builder.setInsertPoint(endBlock);
    return nullptr;
}

Value* IRGenerator::visitDoWhileStmt(DoWhileStmtAST* node)
{
    BasicBlock* bodyBlock = createBlock("do.body");
    BasicBlock* condBlock = createBlock("do.cond");
    BasicBlock* endBlock = createBlock("do.end");
    ensureBranchTo(bodyBlock);

    builder.setInsertPoint(bodyBlock);
    breakTargets.push_back(endBlock);
    continueTargets.push_back(condBlock);
    if (node->body)
    {
        node->body->accept(*this);
    }
    breakTargets.pop_back();
    continueTargets.pop_back();
    ensureBranchTo(condBlock);

    builder.setInsertPoint(condBlock);
    builder.createCondBr(castToCondition(node->condition->accept(*this)), bodyBlock, endBlock);
    builder.setInsertPoint(endBlock);
    return nullptr;
}

Value* IRGenerator::visitForStmt(ForStmtAST* node)
{
    enterScope();
    if (node->init)
    {
        node->init->accept(*this);
    }
    BasicBlock* condBlock = createBlock("for.cond");
    BasicBlock* bodyBlock = createBlock("for.body");
    BasicBlock* incBlock = createBlock("for.inc");
    BasicBlock* endBlock = createBlock("for.end");
    ensureBranchTo(condBlock);

    builder.setInsertPoint(condBlock);
    if (node->condition)
    {
        builder.createCondBr(castToCondition(node->condition->accept(*this)), bodyBlock, endBlock);
    }
    else
    {
        builder.createBr(bodyBlock);
    }

    builder.setInsertPoint(bodyBlock);
    breakTargets.push_back(endBlock);
    continueTargets.push_back(incBlock);
    if (node->body)
    {
        node->body->accept(*this);
    }
    breakTargets.pop_back();
    continueTargets.pop_back();
    ensureBranchTo(incBlock);

    builder.setInsertPoint(incBlock);
    if (node->increment)
    {
        node->increment->accept(*this);
    }
    ensureBranchTo(condBlock);

    builder.setInsertPoint(endBlock);
    exitScope();
    return nullptr;
}

Value* IRGenerator::visitReturnStmt(ReturnStmtAST* node)
{
    Type* retTy = currentFunction ? currentFunction->getReturnType() : Type::getVoidTy();
    if (retTy->isVoidTy())
    {
        builder.createRetVoid();
        return nullptr;
    }
    Value* value = node->returnValue ? node->returnValue->accept(*this) : defaultValue(retTy);
    builder.createRet(castTo(value, retTy));
    return nullptr;
}

Value* IRGenerator::visitBreakStmt(BreakStmtAST*)
{
    if (!breakTargets.empty())
    {
        builder.createBr(breakTargets.back());
    }
    return nullptr;
}

Value* IRGenerator::visitContinueStmt(ContinueStmtAST*)
{
    if (!continueTargets.empty())
    {
        builder.createBr(continueTargets.back());
    }
    return nullptr;
}

Value* IRGenerator::visitVarDeclStmt(VarDeclStmtAST* node)
{
    Type* varType = typeFromName(node->varType);
    if (!currentFunction)
    {
        auto* init = node->initValue ? evaluateConstantInitializer(node->initValue.get(), varType)
            : dynamic_cast<Constant*>(defaultValue(varType));
        auto* global = new GlobalVariable(varType, node->varName, init, node->isConst);
        module.addGlobal(global);
        scopes.back()[node->varName] = { global, varType, false, false, {}, node->isConst ? init : nullptr };
        if (node->isConst && init)
        {
            constantValues[node->varName] = init;
        }
        return global;
    }

    auto* alloca = builder.createAlloca(varType, node->varName);
    Value* init = node->initValue ? node->initValue->accept(*this) : defaultValue(varType);
    Constant* constValue = node->isConst && node->initValue
        ? evaluateConstantInitializer(node->initValue.get(), varType)
        : nullptr;
    scopes.back()[node->varName] = { alloca, varType, false, false, {}, constValue };
    if (init)
    {
        builder.createStore(castTo(init, varType), alloca);
    }
    return alloca;
}

Value* IRGenerator::visitDeclGroup(DeclGroupAST* node)
{
    for (const auto& decl : node->declarations)
    {
        decl->accept(*this);
    }
    return nullptr;
}

Value* IRGenerator::visitArrayDecl(ArrayDeclAST* node)
{
    std::vector<int> dimensions;
    for (const auto& dim : node->dimensions)
    {
        dimensions.push_back(evaluateConstantIntDimension(dim.get()));
    }
    Type* elementType = typeFromName(node->elementType);
    Type* arrayType = arrayTypeFromDimensions(elementType, dimensions);

    if (!currentFunction)
    {
        Constant* initializer = node->initVal
            ? evaluateConstantArrayInitializer(node->initVal.get(), arrayType, elementType, dimensions)
            : ConstantZeroInitializer::get(arrayType);
        auto* global = new GlobalVariable(arrayType, node->arrayName,
            initializer, node->isConst);
        module.addGlobal(global);
        scopes.back()[node->arrayName] = { global, arrayType, true, false, dimensions, nullptr };
        return global;
    }

    auto* alloca = builder.createAlloca(arrayType, node->arrayName);
    Symbol symbol{ alloca, arrayType, true, false, dimensions, nullptr };
    scopes.back()[node->arrayName] = symbol;
    builder.createStore(ConstantZeroInitializer::get(arrayType), alloca);
    if (node->initVal)
    {
        emitArrayInitStores(symbol, node->initVal.get(), elementType);
    }
    return alloca;
}

Value* IRGenerator::visitStructDecl(StructDeclAST*)
{
    return nullptr;
}

Value* IRGenerator::visitFunctionDecl(FunctionDeclAST* node)
{
    FunctionInfo info = makeFunctionInfo(node->returnType, node->parameters);
    registerFunction(node->functionName, info);
    auto* function = new Function(FunctionType::get(info.returnType, info.paramTypes), node->functionName);
    module.addFunction(function);
    currentFunction = function;
    blockIndex = 0;
    enterScope();

    for (size_t i = 0; i < node->parameters.size(); ++i)
    {
        const auto& param = node->parameters[i];
        auto* arg = function->addArgument(info.paramTypes[i], param.name);
        if (param.isArray)
        {
            std::vector<int> dimensions;
            for (const auto& dim : param.dimensions)
            {
                dimensions.push_back(evaluateConstantIntDimension(dim.get()));
            }
            scopes.back()[param.name] = { arg, arrayParamStorageType(param), true, true, dimensions, nullptr };
        }
        else
        {
            scopes.back()[param.name] = { nullptr, typeFromName(param.type), false, false, {}, nullptr };
        }
    }

    BasicBlock* entry = createBlock("entry");
    builder.setInsertPoint(entry);
    for (size_t i = 0; i < node->parameters.size(); ++i)
    {
        const auto& param = node->parameters[i];
        if (param.isArray)
        {
            continue;
        }
        Symbol& symbol = scopes.back()[param.name];
        symbol.address = builder.createAlloca(symbol.valueType, param.name + ".addr");
        builder.createStore(function->getArguments()[i], symbol.address);
    }

    if (node->body)
    {
        for (const auto& stmt : node->body->statements)
        {
            if (builder.currentBlockHasTerminator())
            {
                break;
            }
            stmt->accept(*this);
        }
    }
    if (!builder.currentBlockHasTerminator())
    {
        if (info.returnType->isVoidTy())
        {
            builder.createRetVoid();
        }
        else
        {
            builder.createRet(defaultValue(info.returnType));
        }
    }

    exitScope();
    currentFunction = nullptr;
    return function;
}

Value* IRGenerator::visitFunctionDeclStmt(FunctionDeclStmtAST* node)
{
    FunctionInfo info = makeFunctionInfo(node->returnType, node->parameters);
    registerFunction(node->functionName, info);
    if (!module.getFunction(node->functionName))
    {
        module.addFunction(new LibFunction(FunctionType::get(info.returnType, info.paramTypes), node->functionName));
    }
    return nullptr;
}
