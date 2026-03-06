#include "CodeGenerator.h"
#include <iostream>

// --- 作用域管理 ---
void CodeGenerator::enterScope()
{
	stackOffsetMap.push_back({});
}

void CodeGenerator::exitScope()
{
	stackOffsetMap.pop_back();
}

int CodeGenerator::getVarOffset(const std::string& varName)
{
	for (auto it = stackOffsetMap.rbegin(); it != stackOffsetMap.rend(); ++it)
	{
		//[]会返回0，如果变量不存在，所以先用count检查一下
		//当然以前的find也行,甚至以前的性能更好,因为只用找一次
		if (it->count(varName)) return it->at(varName);
	}
	std::cerr << "CodeGen Error: Variable " << varName << " not found!\n";
	return 0;
}

std::string CodeGenerator::createLabel(const std::string& prefix)
{
	return prefix + std::to_string(++labelCount);
}

void CodeGenerator::generate(ProgramAST* program)
{
	if (!program) return;
	assemblyCode += ".intel_syntax noprefix\n";

	assemblyCode += ".data\n";
	for (const auto& decl : program->declarations)
	{
		if (auto* varDecl = dynamic_cast<VarDeclStmtAST*>(decl.get()))
		{
			// 在汇编中，全局变量本质上也是一个标签
			assemblyCode += varDecl->varName + ":\n";
			if (varDecl->initValue)
			{
				// 对于全局变量，C/C++规定它的初始值必须是编译期常量
				if (auto* numExpr = dynamic_cast<NumberExprAST*>(varDecl->initValue.get()))
				{
					// .long 表示分配 4 个字节的空间并填入初始数字
					assemblyCode += "  .long " + numExpr->value + "\n";
				}
			}
			else
			{
				// 如果没有初始化，默认填充 4 字节的 0
				assemblyCode += "  .zero 4\n";
			}
		}
	}

	assemblyCode += "\n.text\n";
	assemblyCode += ".global main\n\n";
	for (const auto& decl : program->declarations)
	{
		if (auto* funcDecl = dynamic_cast<FunctionDeclAST*>(decl.get()))
		{
			generateFunctionDecl(funcDecl);
		}
	}

	if (!stringPool.empty())
	{
		assemblyCode += "\n.data\n";
		for (size_t i = 0; i < stringPool.size(); ++i)
		{
			assemblyCode += ".L_str_" + std::to_string(i) + ":\n";
			assemblyCode += "  .ascii \"" + stringPool[i] + "\\0\"\n"; // 字符串必须以 \0 结尾
		}
	}
}

// --- 函数定义生成 ---
void CodeGenerator::generateFunctionDecl(FunctionDeclAST* funcDecl)
{
	currentFunctionName = funcDecl->functionName;

	assemblyCode += funcDecl->functionName + ":\n";

	// Prologue (序言)
	assemblyCode += "  push rbp\n";
	assemblyCode += "  mov rbp, rsp\n";
	assemblyCode += "  sub rsp, 128\n"; // 预留栈空间

	enterScope();
	currentOffset = 0;

	if (funcDecl->body)
	{
		for (const auto& stmt : funcDecl->body->statements)
		{
			generateStmt(stmt.get());
		}
	}

	exitScope();

	// Epilogue (结语)
	assemblyCode += ".L_" + funcDecl->functionName + "_end:\n";
	if (funcDecl->functionName == "main")
	{
		assemblyCode += "  call getchar\n"; // 强行让程序等待键盘输入
		assemblyCode += "  mov rsp, rbp\n";
		assemblyCode += "  pop rbp\n";
		assemblyCode += "  ret\n\n";
	}
	/*assemblyCode += "  mov rsp, rbp\n";
	assemblyCode += "  pop rbp\n";
	assemblyCode += "  ret\n\n";*/
}

// --- 语句分发 ---
void CodeGenerator::generateStmt(StmtAST* stmt)
{
	if (!stmt) return;
	if (auto* varDecl = dynamic_cast<VarDeclStmtAST*>(stmt)) generateVarDecl(varDecl);
	else if (auto* exprStmt = dynamic_cast<ExprStmtAST*>(stmt)) generateExpr(exprStmt->expr.get());
	else if (auto* retStmt = dynamic_cast<ReturnStmtAST*>(stmt)) generateReturnStmt(retStmt);
	else if (auto* ifStmt = dynamic_cast<IfStmtAST*>(stmt)) generateIfStmt(ifStmt);
	else if (auto* whileStmt = dynamic_cast<WhileStmtAST*>(stmt)) generateWhileStmt(whileStmt);
	else if (auto* doWhileStmt = dynamic_cast<DoWhileStmtAST*>(stmt)) generateDoWhileStmt(doWhileStmt);
	else if (auto* blockStmt = dynamic_cast<BlockStmtAST*>(stmt)) generateBlockStmt(blockStmt);
}

void CodeGenerator::generateVarDecl(VarDeclStmtAST* varDecl)
{
	if (varDecl->varType == "int")
	{
		// 严格按照 int 分配 4 个字节 [1, 8]
		currentOffset += 4;
		stackOffsetMap.back()[varDecl->varName] = currentOffset;

		if (varDecl->initValue)
		{
			// 计算表达式，结果在 64位的 rax 中
			generateExpr(varDecl->initValue.get());
			// 将 rax 的低 32 位 (eax) 存入 4 字节的内存中
			assemblyCode += "  mov dword ptr [rbp - " + std::to_string(currentOffset) + "], eax\n";
		}

	}

}

//void CodeGenerator::generateReturnStmt(ReturnStmtAST* retStmt)
//{
//	if (retStmt->returnValue)
//	{
//		generateExpr(retStmt->returnValue.get());
//	}
//	assemblyCode += "  mov rsp, rbp\n";
//	assemblyCode += "  pop rbp\n";
//	assemblyCode += "  ret\n";
//}

void CodeGenerator::generateReturnStmt(ReturnStmtAST* retStmt)
{
	if (retStmt->returnValue)
	{
		generateExpr(retStmt->returnValue.get());
	}

	assemblyCode += "  jmp .L_" + currentFunctionName + "_end\n";
}

void CodeGenerator::generateExpr(ExprAST* expr)
{
	if (!expr) return;

	if (auto* numExpr = dynamic_cast<NumberExprAST*>(expr))
	{
		// 直接将数字放入 64 位寄存器
		assemblyCode += "  mov rax, " + numExpr->value + "\n";
	}
	else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr))
	{
		int offset = getVarOffset(varExpr->name);
		// 读取 4 字节内存，并符号扩展为 8 字节存入 rax
		assemblyCode += "  movsxd rax, dword ptr [rbp - " + std::to_string(offset) + "]\n";
	}
	else if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr))
	{
		generateExpr(binExpr->LHS.get());
		assemblyCode += "  push rax\n"; // 左操作数入栈保护

		generateExpr(binExpr->RHS.get());
		assemblyCode += "  mov rbx, rax\n"; // 右操作数放入 rbx
		assemblyCode += "  pop rax\n";      // 左操作数弹出到 rax

		// 执行纯 64 位的计算
		if (binExpr->op == "+")
		{
			assemblyCode += "  add rax, rbx\n";
		}
		else if (binExpr->op == "-")
		{
			assemblyCode += "  sub rax, rbx\n";
		}
		else if (binExpr->op == "*")
		{
			assemblyCode += "  imul rax, rbx\n";
		}
		else if (binExpr->op == "/")
		{
			assemblyCode += "  cqo\n";      // 64 位除法准备指令
			assemblyCode += "  idiv rbx\n"; // 商存在 rax
		}
	}
	else if (auto* strExpr = dynamic_cast<StringExprAST*>(expr))
	{
		// 将字符串放入池子，生成一个标签名（如 .L_str_0）
		int strIndex = stringPool.size();
		stringPool.push_back(strExpr->value);
		// 把字符串标签的地址（指针）加载到 rax 中
		assemblyCode += "  lea rax, [rip + .L_str_" + std::to_string(strIndex) + "]\n";
	}
	else if (auto* callExpr = dynamic_cast<CallExprAST*>(expr))
	{
		if (callExpr->name == "printf")
		{
			// Windows x64 传参规则：第一个参数放 rcx，第二个放 rdx
			if (callExpr->args.size() > 0)
			{
				generateExpr(callExpr->args[0].get());
				assemblyCode += "  mov rcx, rax\n"; // 格式化字符串地址放入 rcx
			}
			if (callExpr->args.size() > 1)
			{
				// 注意：要先 push rcx 保护一下，算完第二参数再 pop 出来
				assemblyCode += "  push rcx\n";
				generateExpr(callExpr->args[1].get());
				assemblyCode += "  mov rdx, rax\n"; // 要打印的数字放入 rdx
				assemblyCode += "  pop rcx\n";
			}

			assemblyCode += "  sub rsp, 32\n";    // Windows x64 ABI 必须的 Shadow Space (预留空间)
			assemblyCode += "  call printf\n";
			assemblyCode += "  add rsp, 32\n";    // 恢复栈
		}
	}
}

void CodeGenerator::generateBlockStmt(BlockStmtAST* blockStmt)
{
	if (!blockStmt) return;

	enterScope(); // 进入新的作用域

	// 遍历生成大括号内的所有语句
	for (const auto& stmt : blockStmt->statements)
	{
		generateStmt(stmt.get());
	}

	exitScope();  // 离开作用域，变量偏移量映射表将自动弹出
}

void CodeGenerator::generateIfStmt(IfStmtAST* ifStmt)
{
	// 创建唯一的跳转标签
	std::string elseLabel = createLabel(".L_if_else_");
	std::string endLabel = createLabel(".L_if_end_");

	// 计算 if 条件表达式，结果存入 rax
	generateExpr(ifStmt->condition.get());

	// 判断条件是否为假 (0)
	assemblyCode += "  cmp rax, 0\n";

	// 如果为假，跳转到 else 分支（如果有）或者直接跳到末尾
	if (ifStmt->elseBranch)
	{
		assemblyCode += "  je " + elseLabel + "\n";
	}
	else
	{
		assemblyCode += "  je " + endLabel + "\n";
	}

	// 生成 thenBranch (条件为真时执行的代码)
	if (ifStmt->thenBranch)
	{
		generateStmt(ifStmt->thenBranch.get());
	}

	// 如果有 else 分支，then 分支执行完后必须无条件跳过 else 分支
	if (ifStmt->elseBranch)
	{
		assemblyCode += "  jmp " + endLabel + "\n"; // then执行完，跳到结束

		assemblyCode += elseLabel + ":\n";         // Else 标签所在位置
		generateStmt(ifStmt->elseBranch.get());    // 生成 else 内部的语句
	}

	// 写入结束标签
	assemblyCode += endLabel + ":\n";
}

void CodeGenerator::generateWhileStmt(WhileStmtAST* whileStmt)
{
	std::string condLabel = createLabel(".L_while_cond_");
	std::string endLabel = createLabel(".L_while_end_");

	// 写入条件判断的起始标签
	assemblyCode += condLabel + ":\n";

	// 计算 condition，结果在 rax
	generateExpr(whileStmt->condition.get());

	// 判断是否为假，如果为假 (0)，立刻跳出循环，飞到 endLabel
	assemblyCode += "  cmp rax, 0\n";
	assemblyCode += "  je " + endLabel + "\n";

	// 生成循环体 body 内的代码
	if (whileStmt->body)
	{
		generateStmt(whileStmt->body.get());
	}

	// 循环体执行完，无条件跳回条件判断处
	assemblyCode += "  jmp " + condLabel + "\n";

	// 写入循环结束的标签
	assemblyCode += endLabel + ":\n";
}

void CodeGenerator::generateDoWhileStmt(DoWhileStmtAST* doWhileStmt)
{
	std::string loopLabel = createLabel(".L_dowhile_loop_");

	// 写入循环开始标签
	assemblyCode += loopLabel + ":\n";

	// 无条件先执行一次 body
	if (doWhileStmt->body)
	{
		generateStmt(doWhileStmt->body.get());
	}

	// 执行完后，计算 condition 表达式
	generateExpr(doWhileStmt->condition.get());

	// 比较条件是否为真
	assemblyCode += "  cmp rax, 0\n";

	// jne = Jump if Not Equal。如果 rax 不等于 0（条件为真），则跳回 loopLabel 继续循环
	assemblyCode += "  jne " + loopLabel + "\n";
}

