#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstddef>

class ASTVisitor;

// Base class for all Abstract Syntax Tree nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual size_t getLine() const = 0;
    virtual size_t getColumn() const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Base class for Statement nodes
class StmtNode : public ASTNode {
public:
    ~StmtNode() override = default;
};

// Base class for Expression nodes
class ExprNode : public ASTNode {
public:
    ~ExprNode() override = default;
};

// Forward declarations of AST concrete node classes
class ProgramNode;
class VarDeclNode;
class AssignNode;
class BlockNode;
class IfStmtNode;
class WhileStmtNode;
class BinaryExprNode;
class IdentifierNode;
class NumberLiteralNode;

// Visitor interface for traversal, pretty-printing, and graph generation
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(ProgramNode& node) = 0;
    virtual void visit(VarDeclNode& node) = 0;
    virtual void visit(AssignNode& node) = 0;
    virtual void visit(BlockNode& node) = 0;
    virtual void visit(IfStmtNode& node) = 0;
    virtual void visit(WhileStmtNode& node) = 0;
    virtual void visit(BinaryExprNode& node) = 0;
    virtual void visit(IdentifierNode& node) = 0;
    virtual void visit(NumberLiteralNode& node) = 0;
};

// Root node representing a complete MiniLang program
class ProgramNode : public ASTNode {
public:
    explicit ProgramNode(std::vector<std::unique_ptr<StmtNode>> statements)
        : m_statements(std::move(statements)) {}

    const std::vector<std::unique_ptr<StmtNode>>& getStatements() const {
        return m_statements;
    }

    size_t getLine() const override { return 1; }
    size_t getColumn() const override { return 1; }
    void accept(ASTVisitor& visitor) override;

private:
    std::vector<std::unique_ptr<StmtNode>> m_statements;
};

// Variable declaration: int <name> [= <initExpr>];
class VarDeclNode : public StmtNode {
public:
    VarDeclNode(std::string type, std::string name, std::unique_ptr<ExprNode> initExpr,
                size_t line, size_t column)
        : m_type(std::move(type)), m_name(std::move(name)), m_initExpr(std::move(initExpr)),
          m_line(line), m_column(column) {}

    const std::string& getType() const { return m_type; }
    const std::string& getName() const { return m_name; }
    bool hasInit() const { return m_initExpr != nullptr; }
    const ExprNode* getInitExpr() const { return m_initExpr.get(); }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::string m_type;
    std::string m_name;
    std::unique_ptr<ExprNode> m_initExpr;
    size_t m_line;
    size_t m_column;
};

// Assignment statement: <name> = <value>;
class AssignNode : public StmtNode {
public:
    AssignNode(std::string name, std::unique_ptr<ExprNode> value, size_t line, size_t column)
        : m_name(std::move(name)), m_value(std::move(value)), m_line(line), m_column(column) {}

    const std::string& getName() const { return m_name; }
    const ExprNode* getValue() const { return m_value.get(); }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::string m_name;
    std::unique_ptr<ExprNode> m_value;
    size_t m_line;
    size_t m_column;
};

// Block statement: { <statements> }
class BlockNode : public StmtNode {
public:
    BlockNode(std::vector<std::unique_ptr<StmtNode>> statements, size_t line, size_t column)
        : m_statements(std::move(statements)), m_line(line), m_column(column) {}

    const std::vector<std::unique_ptr<StmtNode>>& getStatements() const {
        return m_statements;
    }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::vector<std::unique_ptr<StmtNode>> m_statements;
    size_t m_line;
    size_t m_column;
};

// Conditional branching: if (<condition>) <thenBlock> [else <elseBlock>]
class IfStmtNode : public StmtNode {
public:
    IfStmtNode(std::unique_ptr<ExprNode> condition,
               std::unique_ptr<BlockNode> thenBlock,
               std::unique_ptr<BlockNode> elseBlock,
               size_t line, size_t column)
        : m_condition(std::move(condition)),
          m_thenBlock(std::move(thenBlock)),
          m_elseBlock(std::move(elseBlock)),
          m_line(line), m_column(column) {}

    const ExprNode* getCondition() const { return m_condition.get(); }
    const BlockNode* getThenBlock() const { return m_thenBlock.get(); }
    bool hasElse() const { return m_elseBlock != nullptr; }
    const BlockNode* getElseBlock() const { return m_elseBlock.get(); }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::unique_ptr<ExprNode> m_condition;
    std::unique_ptr<BlockNode> m_thenBlock;
    std::unique_ptr<BlockNode> m_elseBlock;
    size_t m_line;
    size_t m_column;
};

// While loop construct: while (<condition>) <body>
class WhileStmtNode : public StmtNode {
public:
    WhileStmtNode(std::unique_ptr<ExprNode> condition,
                  std::unique_ptr<BlockNode> body,
                  size_t line, size_t column)
        : m_condition(std::move(condition)),
          m_body(std::move(body)),
          m_line(line), m_column(column) {}

    const ExprNode* getCondition() const { return m_condition.get(); }
    const BlockNode* getBody() const { return m_body.get(); }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::unique_ptr<ExprNode> m_condition;
    std::unique_ptr<BlockNode> m_body;
    size_t m_line;
    size_t m_column;
};

// Binary expression: <left> <op> <right>
class BinaryExprNode : public ExprNode {
public:
    BinaryExprNode(std::string op,
                   std::unique_ptr<ExprNode> left,
                   std::unique_ptr<ExprNode> right,
                   size_t line, size_t column)
        : m_op(std::move(op)),
          m_left(std::move(left)),
          m_right(std::move(right)),
          m_line(line), m_column(column) {}

    const std::string& getOp() const { return m_op; }
    const ExprNode* getLeft() const { return m_left.get(); }
    const ExprNode* getRight() const { return m_right.get(); }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::string m_op;
    std::unique_ptr<ExprNode> m_left;
    std::unique_ptr<ExprNode> m_right;
    size_t m_line;
    size_t m_column;
};

// Variable reference: <name>
class IdentifierNode : public ExprNode {
public:
    IdentifierNode(std::string name, size_t line, size_t column)
        : m_name(std::move(name)), m_line(line), m_column(column) {}

    const std::string& getName() const { return m_name; }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    std::string m_name;
    size_t m_line;
    size_t m_column;
};

// Integer literal value: <value>
class NumberLiteralNode : public ExprNode {
public:
    NumberLiteralNode(long long value, size_t line, size_t column)
        : m_value(value), m_line(line), m_column(column) {}

    long long getValue() const { return m_value; }

    size_t getLine() const override { return m_line; }
    size_t getColumn() const override { return m_column; }
    void accept(ASTVisitor& visitor) override;

private:
    long long m_value;
    size_t m_line;
    size_t m_column;
};
