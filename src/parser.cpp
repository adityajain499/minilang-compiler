#include "parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program = parseProgram();
    if (m_hasErrors) {
        return nullptr;
    }
    return program;
}

const Token& Parser::peek() const {
    if (m_current < m_tokens.size()) {
        return m_tokens[m_current];
    }
    static const Token s_eof(TokenType::EOF_TOKEN, "EOF", 0, 0);
    return s_eof;
}

const Token& Parser::previous() const {
    if (m_current > 0) {
        return m_tokens[m_current - 1];
    }
    return peek();
}

bool Parser::isAtEnd() const {
    return peek().getType() == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return false;
    }
    return peek().getType() == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        m_current++;
    }
    return previous();
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    const Token& tok = peek();
    error(tok, message + " (got '" + tok.getLexeme() + "')");
    throw ParseError(message);
}

void Parser::error(const Token& token, const std::string& message) {
    m_hasErrors = true;
    std::string err = "Syntax error at [" + std::to_string(token.getLine()) + ":" +
                      std::to_string(token.getColumn()) + "]: " + message;
    m_errors.push_back(err);
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (peek().getType() == TokenType::SEMICOLON) {
            advance();
            return;
        }
        if (peek().getType() == TokenType::RBRACE) {
            return;
        }
        if (peek().getType() == TokenType::KW_INT ||
            peek().getType() == TokenType::KW_IF ||
            peek().getType() == TokenType::KW_WHILE) {
            return;
        }
        advance();
    }
}

// Rule (1): Program -> StmtList $
std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto stmts = parseStmtList();
    if (!isAtEnd()) {
        error(peek(), "Unexpected token '" + peek().getLexeme() + "' after program statements");
    }
    return std::make_unique<ProgramNode>(std::move(stmts));
}

// Rules (2), (3): StmtList -> Stmt StmtList | ε
std::vector<std::unique_ptr<StmtNode>> Parser::parseStmtList() {
    std::vector<std::unique_ptr<StmtNode>> stmts;
    while (!isAtEnd() && !check(TokenType::RBRACE)) {
        try {
            auto stmt = parseStmt();
            if (stmt) {
                stmts.push_back(std::move(stmt));
            }
        } catch (const ParseError&) {
            synchronize();
        }
    }
    return stmts;
}

// Rules (4)-(7): Stmt -> VarDecl | AssignStmt | IfStmt | WhileStmt
std::unique_ptr<StmtNode> Parser::parseStmt() {
    if (check(TokenType::KW_INT)) {
        return parseVarDecl();
    }
    if (check(TokenType::ID)) {
        return parseAssignStmt();
    }
    if (check(TokenType::KW_IF)) {
        return parseIfStmt();
    }
    if (check(TokenType::KW_WHILE)) {
        return parseWhileStmt();
    }

    const Token& badToken = peek();
    error(badToken, "Unexpected token '" + badToken.getLexeme() + "', expected statement ('int', identifier, 'if', or 'while')");
    advance();
    throw ParseError("Invalid statement starter");
}

// Rule (8): VarDecl -> int ID InitOpt ;
std::unique_ptr<VarDeclNode> Parser::parseVarDecl() {
    const Token& intTok = consume(TokenType::KW_INT, "Expected 'int'");
    const Token& idTok = consume(TokenType::ID, "Expected identifier after 'int'");
    auto init = parseInitOpt();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<VarDeclNode>("int", idTok.getLexeme(), std::move(init),
                                         intTok.getLine(), intTok.getColumn());
}

// Rules (9), (10): InitOpt -> = Expr | ε
std::unique_ptr<ExprNode> Parser::parseInitOpt() {
    if (match(TokenType::ASSIGN)) {
        return parseExpr();
    }
    return nullptr;
}

// Rule (11): AssignStmt -> ID = Expr ;
std::unique_ptr<AssignNode> Parser::parseAssignStmt() {
    const Token& idTok = consume(TokenType::ID, "Expected identifier");
    consume(TokenType::ASSIGN, "Expected '=' in assignment");
    auto expr = parseExpr();
    consume(TokenType::SEMICOLON, "Expected ';' after assignment");
    return std::make_unique<AssignNode>(idTok.getLexeme(), std::move(expr),
                                        idTok.getLine(), idTok.getColumn());
}

// Rule (12): IfStmt -> if ( Expr ) Block ElseOpt
std::unique_ptr<IfStmtNode> Parser::parseIfStmt() {
    const Token& ifTok = consume(TokenType::KW_IF, "Expected 'if'");
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = parseExpr();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto thenBlock = parseBlock();
    auto elseBlock = parseElseOpt();
    return std::make_unique<IfStmtNode>(std::move(cond), std::move(thenBlock), std::move(elseBlock),
                                        ifTok.getLine(), ifTok.getColumn());
}

// Rules (13), (14): ElseOpt -> else Block | ε
std::unique_ptr<BlockNode> Parser::parseElseOpt() {
    if (match(TokenType::KW_ELSE)) {
        return parseBlock();
    }
    return nullptr;
}

// Rule (15): WhileStmt -> while ( Expr ) Block
std::unique_ptr<WhileStmtNode> Parser::parseWhileStmt() {
    const Token& whileTok = consume(TokenType::KW_WHILE, "Expected 'while'");
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto cond = parseExpr();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto body = parseBlock();
    return std::make_unique<WhileStmtNode>(std::move(cond), std::move(body),
                                          whileTok.getLine(), whileTok.getColumn());
}

// Rule (16): Block -> { StmtList }
std::unique_ptr<BlockNode> Parser::parseBlock() {
    const Token& lbrace = consume(TokenType::LBRACE, "Expected '{'");
    auto stmts = parseStmtList();
    consume(TokenType::RBRACE, "Expected '}'");
    return std::make_unique<BlockNode>(std::move(stmts), lbrace.getLine(), lbrace.getColumn());
}

// Rule (17): Expr -> ArithExpr ExprTail
std::unique_ptr<ExprNode> Parser::parseExpr() {
    auto left = parseArithExpr();
    return parseExprTail(std::move(left));
}

// Rules (18), (19): ExprTail -> CompOp ArithExpr | ε
std::unique_ptr<ExprNode> Parser::parseExprTail(std::unique_ptr<ExprNode> left) {
    if (isCompOp(peek().getType())) {
        const Token& opTok = advance();
        auto right = parseArithExpr();
        return std::make_unique<BinaryExprNode>(opTok.getLexeme(), std::move(left), std::move(right),
                                                opTok.getLine(), opTok.getColumn());
    }
    return left;
}

// Rules (20)-(25): CompOp -> == | != | <= | >= | < | >
bool Parser::isCompOp(TokenType type) const {
    return type == TokenType::EQ ||
           type == TokenType::NEQ ||
           type == TokenType::LE ||
           type == TokenType::GE ||
           type == TokenType::LT ||
           type == TokenType::GT;
}

// Rule (26): ArithExpr -> Term ArithExprTail
std::unique_ptr<ExprNode> Parser::parseArithExpr() {
    auto left = parseTerm();
    return parseArithExprTail(std::move(left));
}

// Rules (27)-(29): ArithExprTail -> + Term ArithExprTail | - Term ArithExprTail | ε
std::unique_ptr<ExprNode> Parser::parseArithExprTail(std::unique_ptr<ExprNode> left) {
    if (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        const Token& opTok = previous();
        auto right = parseTerm();
        auto binary = std::make_unique<BinaryExprNode>(opTok.getLexeme(), std::move(left), std::move(right),
                                                       opTok.getLine(), opTok.getColumn());
        return parseArithExprTail(std::move(binary));
    }
    return left;
}

// Rule (30): Term -> Factor TermTail
std::unique_ptr<ExprNode> Parser::parseTerm() {
    auto left = parseFactor();
    return parseTermTail(std::move(left));
}

// Rules (31)-(33): TermTail -> * Factor TermTail | / Factor TermTail | ε
std::unique_ptr<ExprNode> Parser::parseTermTail(std::unique_ptr<ExprNode> left) {
    if (match(TokenType::STAR) || match(TokenType::SLASH)) {
        const Token& opTok = previous();
        auto right = parseFactor();
        auto binary = std::make_unique<BinaryExprNode>(opTok.getLexeme(), std::move(left), std::move(right),
                                                       opTok.getLine(), opTok.getColumn());
        return parseTermTail(std::move(binary));
    }
    return left;
}

// Rules (34)-(36): Factor -> ID | NUM | ( Expr )
std::unique_ptr<ExprNode> Parser::parseFactor() {
    if (match(TokenType::ID)) {
        const Token& tok = previous();
        return std::make_unique<IdentifierNode>(tok.getLexeme(), tok.getLine(), tok.getColumn());
    }
    if (match(TokenType::NUM)) {
        const Token& tok = previous();
        long long val = 0;
        try {
            val = std::stoll(tok.getLexeme());
        } catch (...) {
            val = 0;
        }
        return std::make_unique<NumberLiteralNode>(val, tok.getLine(), tok.getColumn());
    }
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpr();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    const Token& badTok = peek();
    error(badTok, "Expected expression (identifier, number, or '('), got '" + badTok.getLexeme() + "'");
    advance();
    throw ParseError("Invalid factor");
}
