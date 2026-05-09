#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "frontend/ast/AST.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/parser/Parser.h"
#include "frontend/sema/SemanticAnalyzer.h"
#include "ir/builder/IRGenerator.h"
#include "ir/passes/ConstantFoldingPass.h"
#include "ir/passes/DeadCodeEliminationPass.h"
#include "ir/passes/DeadStoreEliminationPass.h"
#include "ir/passes/IRVerifier.h"
#include "ir/passes/SimplifyCFGPass.h"
#include "ir/passes/StoreLoadForwardingPass.h"

static void printUsage(const char* program)
{
    std::cerr << "Usage: " << program << " [--dump-ast] [--no-opt] [source.sy|-]\n";
}

int main(int argc, char** argv)
{
    bool dumpAst = false;
    bool optimize = true;
    std::string inputPath;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--dump-ast")
        {
            dumpAst = true;
        }
        else if (arg == "--no-opt")
        {
            optimize = false;
        }
        else if (arg == "--help" || arg == "-h")
        {
            printUsage(argv[0]);
            return 0;
        }
        else if (!inputPath.empty())
        {
            printUsage(argv[0]);
            return 1;
        }
        else
        {
            inputPath = arg;
        }
    }

    std::string sourceCode = R"(
int main() {
    int matrix[2][3] = {{1, 2}, {3, 4}};
    int i = 0;
    int j = 1;
    matrix[i][j] = 5;
    int val = matrix[0][j + 1];
    return val;
}
    )";

    if (!inputPath.empty())
    {
        std::ostringstream buffer;
        if (inputPath == "-")
        {
            buffer << std::cin.rdbuf();
        }
        else
        {
            std::ifstream input(inputPath);
            if (!input)
            {
                std::cerr << "Cannot open source file: " << inputPath << "\n";
                return 1;
            }
            buffer << input.rdbuf();
        }
        sourceCode = buffer.str();
    }

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<ProgramAST> program = parser.parseProgram();

    if (!program)
    {
        return 1;
    }

    if (dumpAst)
    {
        program->print(0);
    }

    SemanticAnalyzer sema;
    if (!sema.analyze(program.get()))
    {
        return 1;
    }

    IRGenerator irgen;
    program->accept(irgen);

    if (optimize)
    {
        bool changed = true;
        for (int iteration = 0; changed && iteration < 4; ++iteration)
        {
            changed = false;
            ConstantFoldingPass constantFolding;
            changed |= constantFolding.run(irgen.getModule());
            StoreLoadForwardingPass storeLoadForwarding;
            changed |= storeLoadForwarding.run(irgen.getModule());
            changed |= constantFolding.run(irgen.getModule());
            SimplifyCFGPass simplifyCFG;
            changed |= simplifyCFG.run(irgen.getModule());
            DeadStoreEliminationPass dse;
            changed |= dse.run(irgen.getModule());
            DeadCodeEliminationPass dce;
            changed |= dce.run(irgen.getModule());
        }
    }

    IRVerifier verifier;
    if (!verifier.verify(irgen.getModule()))
    {
        verifier.printErrors();
        return 1;
    }

    std::cout << irgen.printModule();
    return 0;
}
