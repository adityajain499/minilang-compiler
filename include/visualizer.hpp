#pragma once

#include "ast.hpp"
#include <iostream>
#include <string>
#include <vector>

// Formats and prints an indented ASCII tree representation of the AST
class AsciiTreePrinter : public ASTVisitor {
public:
    explicit AsciiTreePrinter(std::ostream& out = std::cout);

    void print(ASTNode& root);

    void visit(ProgramNode& node) override;
    void visit(VarDeclNode& node) override;
    void visit(AssignNode& node) override;
    void visit(BlockNode& node) override;
    void visit(IfStmtNode& node) override;
    void visit(WhileStmtNode& node) override;
    void visit(BinaryExprNode& node) override;
    void visit(IdentifierNode& node) override;
    void visit(NumberLiteralNode& node) override;

private:
    struct ChildNode {
        std::string tag;
        ASTNode* node;
    };

    void printLine(const std::string& text, bool isLast);
    void printChildren(const std::vector<ChildNode>& children);

    std::ostream& m_out;
    std::string m_indent;
};

// Generates Graphviz .dot syntax from the AST
class DotExporter : public ASTVisitor {
public:
    explicit DotExporter(std::ostream& out);

    void exportDot(ASTNode& root);
    static bool exportToFile(ASTNode& root, const std::string& filename);

    void visit(ProgramNode& node) override;
    void visit(VarDeclNode& node) override;
    void visit(AssignNode& node) override;
    void visit(BlockNode& node) override;
    void visit(IfStmtNode& node) override;
    void visit(WhileStmtNode& node) override;
    void visit(BinaryExprNode& node) override;
    void visit(IdentifierNode& node) override;
    void visit(NumberLiteralNode& node) override;

private:
    size_t createNode(const std::string& label, const std::string& shape = "box",
                      const std::string& fillColor = "#DAE8FC",
                      const std::string& strokeColor = "#6C8EBF");
    void createEdge(size_t fromId, size_t toId, const std::string& label = "");
    static std::string escapeDot(const std::string& str);

    std::ostream& m_out;
    size_t m_nextId{0};
    size_t m_currentId{0};
};
