# Minicxx Compiler

<div align="center">
  A lightweight, hand-written C++ subset compiler targeting x86-64 assembly.<br>
  一个轻量级的、纯手写的 C++ 子集编译器，目标代码为 x86-64 汇编。
</div>

## Introduction (简介)

**Minicxx** is a toy compiler built from scratch in C++. It implements a complete compilation pipeline, including Lexical Analysis, Syntax Analysis (AST generation), Semantic Analysis, AST Optimization (Pass Manager), and x86-64 Assembly Code Generation. It relies on GCC for the final assembly and linking stages.

**Minicxx** 是一个纯 C++ 从零手写的微型编译器。它实现了一个完整的编译流水线：词法分析、语法分析（AST构建）、语义分析、抽象语法树优化（Pass趟机制）以及 x86-64 汇编代码生成。它依赖 GCC 完成最后的汇编与链接步骤。

## Features (核心特性)

- **Lexical Analysis (词法分析)**: Hand-written lexer to tokenize input strings [2, 3].
- **Recursive Descent Parser (递归下降语法分析)**: Builds a structured Abstract Syntax Tree (AST) supporting block statements, control flows, and functions [1, 6].
- **Semantic Analysis (语义分析)**: Implements block-scoped symbol tables to check variable declarations, type matching, and function parameter resolution [8, 9].
- **AST Optimization (抽象语法树优化)**: Pluggable optimization architecture supporting Constant Folding.
- **x86-64 Code Generation (x86-64 汇编生成)**: Emits `.intel_syntax` assembly, utilizing 64-bit registers (`rax`, `rbx`) and precise stack pointer (`rbp`, `rsp`) management.

## Architecture (架构)

The compiler pipeline strictly follows modern compiler design principles:
本编译器的流水线严格遵循现代编译器设计原则：

1. **Lexer**: Converts raw source code into a stream of `Token`s, parsing keywords, identifiers, numbers, and operators [2, 4].
2. **Parser**: Generates a `ProgramAST` root node containing multiple statements [1, 7]. Supports definitions for `IfStmtAST`, `WhileStmtAST`, `DoWhileStmtAST`, `VarDeclStmtAST`, and `FunctionDeclAST` [1].
3. **Semantic Analyzer**: Uses a `std::vector<std::unordered_map>` as a stack to manage scopes [9]. Checks for semantic errors like variable redefinition or undeclared usage [8].
4. **PassManager & Optimizer**: Traverses the AST to perform modifications (e.g., Constant Folding).
5. **CodeGenerator**: Traverses the optimized AST to emit x86-64 assembly instructions.
6. **GCC Linking**: Automatically invokes GCC to compile the `.s` file into a native `.exe`.

## Supported Syntax (支持的语法)

Currently, Minicxx supports a basic subset of C++:
目前，Minicxx 支持基础的 C++ 子集：

*   **Primitive Types / 基础类型**: `int`, `void` [8].
*   **Variable Declarations / 变量声明**: `int a = 10;` [1].
*   **Binary Operations / 二元运算**: `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, `<=`, `>=` [4].
*   **Control Flow / 控制流**: `if-else`, `while`, `do-while` [1].
*   **Functions / 函数**: Function declarations, definitions, and `return` statements [1, 6].
*   **Built-in Functions / 内置函数**: Includes special support for `printf`.

*Note: Minicxx currently does not support floating-point numeric literals, boolean values, comments, or `#` macro directives [10].*
*注意：Minicxx 目前不支持浮点数字面量、布尔值、注释或 `#` 宏指令 [10]。*

## Getting Started (快速开始)

### Prerequisites (依赖项)
- A modern C++ compiler (C++17 or higher recommended).
- **MinGW-w64 (64-bit GCC)** installed and added to your system's `PATH`.

### Build and Run (编译与运行)
1. Clone the repository / 克隆仓库:
   ```bash
   git clone https://github.com/YourUsername/Minicxx.git
   cd Minicxx/src
   ```
2. Compile the Minicxx compiler itself / 编译 Minicxx 编译器:
   ```bash
   g++ *.cpp -o minicxx.exe -std=c++17
   ```
3. Run Minicxx / 运行编译器:
   ```bash
   ./minicxx.exe
   ```

### Usage (使用方法)
1. Run the compiler executable.
2. Select the optimization level (0, 1, or 2).
3. Specify the output directory (or press Enter for the current directory).
4. Enter your C++ code. Terminate the input with an empty line.
5. The compiler will generate the `output.s` assembly file and automatically invoke GCC to generate `program.exe`.

## Example (代码示例)

**Input Code (输入代码)**:
```c
int main() {
    int a = 10;
    int b = 20;
    int c = a + b * 2;
    printf("Result is %d\n", c);
    return 0;
}
```

**Output (输出)**:
The compiler will print the parsed AST [1] and generate a native Windows 64-bit executable `program.exe` that prints `Result is 50`.

## Limitations & Future Work (局限性与未来计划)
- Add robust error recovery in the Parser.
- Introduce advanced types like `float`, `double`, `char`, and pointers.
- Implement more optimization passes (e.g., Dead Code Elimination).
- Support arrays and structs.

## License (开源协议)
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
