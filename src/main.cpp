#include<iostream>
#include<vector>
#include"Lexer.h"
#include"LexerMap.h"
#include"Parser.h"
#include"SemanticAnalyzer.h"
#include"PassManager.h"
#include"CodeGenerator.h"
#include <fstream>
#include <cstdlib>
#include <string>

void printToken(const Token& tok)
{
    std::string typeName = "UnknownType";
    auto it = TokenNameMap.find(tok.type);
    if (it != TokenNameMap.end())
    {
        typeName = it->second;
    }
    std::cout << "[" << typeName << "] \t'" << tok.value << "'\n";
}

int main()
{
    std::cout << "Minicxx currently does not support floating-point numeric literals, boolean values, comments, #, and other advanced operations.\n\n\n";
	std::cout << "But!!!   Supported!!!!  printf  !!!!!\n\n\n";
    std::cout << "Please enter your code. To exit, type 'ccc' on a new line.\n\n";
    while (true)
    {
        std::string optLevel = "0";
        int optLevelInt = 0;
        std::cout << "Select optimization level (0: no optimization, 1: basic optimization, 2: advanced optimization): ";
        std::getline(std::cin, optLevel);
        if (optLevel == "ccc")
        {
            std::cout << "Exiting...\n";
            std::cin.get();
            return 0;
        }
        else if (optLevel == "0")
        {
            optLevelInt = 0;
        }
        else if (optLevel == "1")
        {
            optLevelInt = 1;
        }
        else if (optLevel == "2")
        {
            optLevelInt = 2;
        }
        else
        {
            std::cout << "Invalid optimization level. Defaulting to 0 (no optimization).\n";
            optLevelInt = 0;
        }

        std::string outputDir;
        std::cout << "Enter output directory path (leave empty for current directory): ";
        std::getline(std::cin, outputDir);
        if (outputDir == "ccc")
        {
            std::cout << "Exiting...\n";
            std::cin.get();
            return 0;
        }

        // 智能处理路径拼接：如果用户输入了路径，检查末尾有没有斜杠，没有就补上
        std::string asmFilePath = "output.s";
        std::string exeFilePath = "program.exe";
        if (!outputDir.empty())
        {
            char lastChar = outputDir.back();
            if (lastChar != '\\' && lastChar != '/')
            {
                outputDir += "\\"; // 默认按 Windows 的反斜杠补充
            }
            asmFilePath = outputDir + "output.s";
            exeFilePath = outputDir + "program.exe";
        }
        // ==========================================================

        std::cout << "Enter code (end with an empty line):\n";
        std::string code;
        while (true)
        {
            std::string line;
            std::getline(std::cin, line);
            if (line.empty()) break;
            if (line == "ccc")
            {
                std::cout << "Exiting...\n";
                std::cin.get();
                return 0;
            }
            code += line + "\n";
        }
        std::cout << "===========================\n";
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        /*for (const auto& tok : tokens)
        {
        printToken(tok);
        }*/
        // 词法给语法
        Parser parser(tokens);
        auto ast = parser.parseProgram();
        //if (ast) ast->print();

        // 语法给语义
        SemanticAnalyzer analyzer;
     

        if (analyzer.analyze(ast.get()))
        {
            continue;
        }

        // 语义给优化
        PassManager passManager(optLevelInt); // 传入优化级别，0表示不优化，1表示基础优化，2表示进阶优化
        auto optimizedAST = passManager.runAll(std::move(ast));

        // 语义给代码生成
        CodeGenerator codeGen;
        codeGen.generate(optimizedAST.get());
        std::string assemblyCode = codeGen.getAssembly();
        std::cout << "Generated Assembly:\n" << assemblyCode;

        //continue;

        // 将生成的汇编代码写入指定目录下的 output.s 文件
        std::ofstream outFile(asmFilePath);
        if (outFile.is_open())
        {
            outFile << assemblyCode;
            outFile.close();
            std::cout << "\nAssembly code has been written to " << asmFilePath << "\n";
        }
        else
        {
            std::cerr << "Failed to open " << asmFilePath << " for writing.\n";
            continue; // 文件创建失败就重来
        }

        // 使用组装好的绝对路径调用 GCC，并在路径两侧加上双引号以防路径中有空格
        std::string compileCmd = "gcc \"" + asmFilePath + "\" -o \"" + exeFilePath + "\" -m64";
        std::cout << "Running command: " << compileCmd << "\n";

        int compileResult = std::system(compileCmd.c_str());
        if (compileResult == 0)
        {
            std::cout << "Compilation successful! Executable generated: " << exeFilePath << "\n";
        }
        else
        {
            std::cerr << "Compilation failed. \n";
        }
        std::cout << "\n";
    }
    return 0;
}