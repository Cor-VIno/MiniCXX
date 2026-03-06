# Minicxx Compiler

<div align="center">
  A lightweight, hand-written compiler currently implementing a subset of C, with a roadmap to evolve into a fully-featured C++ compiler targeting x86-64 assembly.
</div>

## 📖 Introduction

**Minicxx** is a toy compiler built from scratch in C++. Currently, its syntax and capabilities closely resemble a subset of the C language (supporting functions, basic control flows, and integer operations) . However, the core architecture is designed with extensibility in mind, paving the way for future C++ features such as classes, objects, and advanced type systems. 

It implements a complete compilation pipeline: Lexical Analysis, Syntax Analysis (AST generation), Semantic Analysis, AST Optimization (Pass Manager), and x86-64 Assembly Code Generation.

## 📦 Releases (Play Instantly!)

You don't need to build the compiler from source to try it out! 
Head over to the **[Releases]** page of this repository to download the latest pre-built `minicxx.exe`. You can run it directly on your Windows machine out of the box.

*(Note: Ensure you have a 64-bit GCC (MinGW-w64) installed and added to your system's `PATH` so Minicxx can assemble and link the final executable).*

## 🚀 How to Use (Interactive CLI)

Running the compiler will start an interactive command-line session. Here is the step-by-step operation guide:

1. **Select Optimization Level**: The compiler will first prompt you to select an optimization level. Enter `0` (no optimization), `1` (basic optimization, like constant folding), or `2` (advanced optimization).
2. **Set Output Directory**: Enter the absolute or relative folder path where you want the generated `.s` (assembly) and `.exe` (executable) files to be saved. If you simply press `Enter` (leave it blank), the files will be saved in the current directory.
3. **Write Your Code**: Enter your C/C++ code line by line. 
4. **Compile**: When you are finished typing your code, press `Enter` twice (i.e., submit an empty line). The compilation process will begin immediately, outputting the AST and generating the files.
5. **Exit**: To exit the compiler safely, simply type `ccc` on a new line and press Enter.

## 💻 Supported Syntax (Current C Subset)

*   **Primitive Types**: `int`, `void`.
*   **Variable Declarations**: `int a = 10;`.
*   **Binary Operations**: `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, `<=`, `>=`.
*   **Control Flow**: `if-else`, `while`, `do-while`.
*   **Functions**: Function declarations, definitions, and `return` statements.
*   **Built-in Functions**: Native support for `printf`.

*Note: Minicxx currently does not support floating-point numeric literals, boolean values, comments, or `#` macro directives.*

## 🔮 Future Roadmap (Towards C++)

As the name **Minicxx** implies, the ultimate goal is to evolve this project from a C subset into a C++ compiler. Planned features include:
- Object-Oriented Programming (Classes, inheritance, virtual functions).
- Support for `float`, `double`, and `bool`.
- Namespaces and C++ standard library subset integration.
- Pointers and reference types.

## 📄 License
This project is licensed under the MIT License.

---
---

# Minicxx 编译器

<div align="center">
  一个轻量级的、纯手写的编译器。目前实现了 C 语言的基础子集，未来计划逐步演进为全面支持面向对象的 C++ 编译器，目标代码为 x86-64 汇编。
</div>

## 📖 简介

**Minicxx** 是一个纯 C++ 从零手写的微型编译器。目前，它的语法和功能更贴近 C 语言的子集（支持函数、基础控制流和整数运算）。然而，其底层架构在设计之初就考虑到了极高的扩展性，为未来引入类、对象、多态等 C++ 高级特性奠定了基础。

它实现了一个完整的编译流水线：词法分析、语法分析（AST构建）、语义分析 、抽象语法树优化（Pass趟机制）以及 x86-64 汇编代码生成。

## 📦 Releases (开箱即用)

您不需要从源码去编译这个编译器！
只需前往本仓库的 **[Releases]** 页面，下载最新打包好的 `minicxx.exe`，即可在 Windows 系统上直接双击运行体验。

*(注意：为了让 Minicxx 能成功把汇编代码转换为最终的可执行程序，请确保您的电脑上已安装 64 位的 GCC (MinGW-w64) 并已配置好 `PATH` 环境变量)。*

## 🚀 使用指南 (交互式命令行)

运行 `minicxx.exe` 后，您将进入一个交互式的命令行终端。具体操作步骤如下：

1. **选择优化力度**：程序会首先要求您选择代码的优化级别。您可以输入 `0`（不优化）、`1`（基础优化，如常量折叠）或 `2`（进阶优化）。
2. **设置输出目录**：输入您希望将生成的 `.s` (汇编文件) 和 `.exe` (目标程序) 保存到的文件夹路径。如果您直接按下回车（留空），文件将默认保存在当前目录下。
3. **编写代码**：您可以像在普通编辑器里一样，逐行输入您的 C/C++ 代码。
4. **开始编译**：当您的代码编写完成后，连续按下两次回车（即输入一个空行）即可触发编译。编译器会打印出 AST 并自动在您指定的目录下生成汇编与可执行文件。
5. **退出程序**：如果想要安全退出编译器，只需在新的一行输入 `ccc` 并回车即可。

## 💻 目前支持的语法 (C 语言子集)

*   **基础类型**: `int`, `void`。
*   **变量声明**: `int a = 10;`。
*   **二元运算**: `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, `<=`, `>=`。
*   **控制流**: `if-else`, `while`, `do-while`。
*   **函数**: 函数声明、定义以及 `return` 返回语句。
*   **内置函数**: 原生开特权支持 `printf`。

*注意：Minicxx 目前暂不支持浮点数字面量、布尔值、注释或 `#` 宏指令等操作。*

## 🔮 未来规划 (向 C++ 演进)

正如其名 **Minicxx** 所暗示的，本项目的终极目标是从 C 子集平滑过渡到完整的 C++ 编译器。未来的开发路线图包括：
- 面向对象编程支持（类、继承、虚函数等）。
- 扩展数据类型支持：`float`、`double`、`bool`。
- 命名空间（Namespaces）与基础 C++ 标准库子集。
- 指针与引用类型的实现。

## 📄 开源协议
本项目采用 MIT 开源许可协议。
