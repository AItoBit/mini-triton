#include "MiniTriton/Frontend/Lexer.h"
#include "MiniTriton/Frontend/Parser.h"
#include "MiniTriton/AST/ASTPrinter.h"
#include "MiniTriton/Semantic/TypeChecker.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

using namespace minitriton;

static void printUsage(const char* progName) {
    std::cerr << "MiniTriton Compiler v0.1.0\n\n"
              << "Usage: " << progName << " <command> <file.mt>\n\n"
              << "Commands:\n"
              << "  dump-tokens   Print the token stream\n"
              << "  dump-ast      Print the abstract syntax tree\n"
              << "  check         Run semantic analysis / type checking\n"
              << "  compile       Compile to MLIR/LLVM/PTX (future)\n"
              << "  run           Compile and execute on GPU (future)\n";
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << path << "'\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// -----------------------------------------------------------------------
// dump-tokens
// -----------------------------------------------------------------------

static int cmdDumpTokens(const std::string& filename, const std::string& source) {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();

    std::cout << "=== Token Stream: " << filename << " ===\n\n";
    for (const auto& tok : tokens) {
        std::cout << "  " << tok << "\n";
    }
    std::cout << "\nTotal: " << tokens.size() << " tokens\n";
    return 0;
}

// -----------------------------------------------------------------------
// dump-ast
// -----------------------------------------------------------------------

static int cmdDumpAST(const std::string& filename, const std::string& source) {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();

    Parser parser(tokens, filename);
    auto module = parser.parseModule();

    if (parser.hasErrors()) {
        std::cerr << "Parse errors:\n";
        for (const auto& err : parser.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    ASTPrinter printer;
    std::cout << "=== AST: " << filename << " ===\n\n";
    std::cout << printer.print(*module);
    return 0;
}

// -----------------------------------------------------------------------
// check (type checking)
// -----------------------------------------------------------------------

static int cmdCheck(const std::string& filename, const std::string& source) {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();

    Parser parser(tokens, filename);
    auto module = parser.parseModule();

    if (parser.hasErrors()) {
        std::cerr << "Parse errors:\n";
        for (const auto& err : parser.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    TypeChecker checker;
    bool ok = checker.check(*module);

    if (!ok) {
        std::cerr << "Type errors:\n";
        for (const auto& err : checker.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    std::cout << filename << ": all checks passed ✓\n";
    return 0;
}

// -----------------------------------------------------------------------
// compile (placeholder)
// -----------------------------------------------------------------------

static int cmdCompile(const std::string& filename, const std::string& source) {
    // Phase 1: Frontend
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();

    Parser parser(tokens, filename);
    auto module = parser.parseModule();

    if (parser.hasErrors()) {
        std::cerr << "Parse errors:\n";
        for (const auto& err : parser.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    TypeChecker checker;
    if (!checker.check(*module)) {
        std::cerr << "Type errors:\n";
        for (const auto& err : checker.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    std::cout << "Frontend OK ✓\n";
    std::cout << "\n[TODO] MLIR generation not yet implemented\n"
              << "[TODO] GPU lowering not yet implemented\n"
              << "[TODO] PTX generation not yet implemented\n";
    return 0;
}

// -----------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    std::string filename = argv[2];

    std::string source = readFile(filename);
    if (source.empty() && filename != "-") {
        return 1;
    }

    if (command == "dump-tokens") return cmdDumpTokens(filename, source);
    if (command == "dump-ast")    return cmdDumpAST(filename, source);
    if (command == "check")       return cmdCheck(filename, source);
    if (command == "compile")     return cmdCompile(filename, source);
    if (command == "run") {
        std::cerr << "[TODO] 'run' not yet implemented\n";
        return 1;
    }

    std::cerr << "Unknown command: " << command << "\n";
    printUsage(argv[0]);
    return 1;
}
