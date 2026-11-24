#pragma once

#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

// Exception type used internally for panic-mode error recovery unwinding
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Parses the token stream and builds an AST. Returns nullptr if syntax errors occur.
    std::unique_ptr<ProgramNode> parse();

    bool hasErrors() const { return m_hasErrors; }
    const std::vector<std::string>& getErrors() const { return m_errors; }

private:
    // Recursive descent methods mapping 1:1 to grammar rules in docs/grammar_spec.md
    std::unique_ptr<ProgramNode> parseProgram();                                    // Rule (1)
    std::vector<std::unique_ptr<StmtNode>> parseStmtList();                         // Rules (2), (3)
    std::unique_ptr<StmtNode> parseStmt();                                          // Rules (4)-(7)
    std::unique_ptr<VarDeclNode> parseVarDecl();                                    // Rule (8)
    std::unique_ptr<ExprNode> parseInitOpt();                                       // Rules (9), (10)
    std::unique_ptr<AssignNode> parseAssignStmt();                                  // Rule (11)
    std::unique_ptr<IfStmtNode> parseIfStmt();                                      // Rule (12)
    std::unique_ptr<BlockNode> parseElseOpt();                                      // Rules (13), (14)
    std::unique_ptr<WhileStmtNode> parseWhileStmt();                                // Rule (15)
    std::unique_ptr<BlockNode> parseBlock();                                        // Rule (16)
    std::unique_ptr<ExprNode> parseExpr();                                          // Rule (17)
    std::unique_ptr<ExprNode> parseExprTail(std::unique_ptr<ExprNode> left);        // Rules (18), (19)
    bool isCompOp(TokenType type) const;                                            // Rules (20)-(25)
    std::unique_ptr<ExprNode> parseArithExpr();                                     // Rule (26)
    std::unique_ptr<ExprNode> parseArithExprTail(std::unique_ptr<ExprNode> left);   // Rules (27)-(29)
    std::unique_ptr<ExprNode> parseTerm();                                          // Rule (30)
    std::unique_ptr<ExprNode> parseTermTail(std::unique_ptr<ExprNode> left);        // Rules (31)-(33)
    std::unique_ptr<ExprNode> parseFactor();                                        // Rules (34)-(36)

    // Token stream inspection helpers
    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& advance();
    const Token& consume(TokenType type, const std::string& message);

    // Diagnostics and Panic-Mode recovery
    void error(const Token& token, const std::string& message);
    void synchronize();

    std::vector<Token> m_tokens;
    size_t m_current{0};
    bool m_hasErrors{false};
    std::vector<std::string> m_errors;
};
