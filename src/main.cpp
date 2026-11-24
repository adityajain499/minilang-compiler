#include "lexer.hpp"
#include "parser.hpp"
#include "visualizer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.mini> [output.dot]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string dotOutputFile = (argc >= 3) ? argv[2] : "ast.dot";

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file '" << inputFile << "'\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 1. Lexical Analysis
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenizeAll();

    for (const auto& token : tokens) {
        std::cout << token.toString() << "\n";
    }

    if (lexer.hasErrors()) {
        std::cerr << "Lexical Analysis: FAILED (" << lexer.getErrors().size() << " errors found)\n";
        return 1;
    }

    // 2. Syntax Analysis & AST Construction
    Parser parser(tokens);
    auto ast = parser.parse();

    if (parser.hasErrors() || !ast) {
        std::cerr << "Syntax Analysis: FAILED (" << parser.getErrors().size() << " errors found):\n";
        for (const auto& err : parser.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    std::cout << "Syntax Analysis: PASSED\n\n";

    // 3. ASCII Tree Visualization
    std::cout << "=== Abstract Syntax Tree (ASCII) ===\n";
    AsciiTreePrinter printer(std::cout);
    printer.print(*ast);
    std::cout << "\n";

    // 4. Graphviz .dot Export
    if (DotExporter::exportToFile(*ast, dotOutputFile)) {
        std::cout << "AST Graphviz written to: " << dotOutputFile << "\n";
    } else {
        std::cerr << "Error: Failed to write AST to " << dotOutputFile << "\n";
        return 1;
    }

    return 0;
}
