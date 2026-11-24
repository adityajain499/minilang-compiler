#pragma once

#include <string>
#include <string_view>

enum class TokenType {
    // Keywords
    KW_INT,
    KW_IF,
    KW_ELSE,
    KW_WHILE,

    // Identifiers and Literals
    ID,
    NUM,

    // Arithmetic Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,

    // Assignment Operator
    ASSIGN,

    // Comparison Operators
    EQ,
    NEQ,
    LT,
    LE,
    GT,
    GE,

    // Delimiters
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,

    // Special
    EOF_TOKEN,
    ERROR
};

std::string_view tokenTypeToString(TokenType type);

class Token {
public:
    Token(TokenType type, std::string lexeme, size_t line, size_t column)
        : m_type(type), m_lexeme(std::move(lexeme)), m_line(line), m_column(column) {}

    TokenType getType() const { return m_type; }
    const std::string& getLexeme() const { return m_lexeme; }
    size_t getLine() const { return m_line; }
    size_t getColumn() const { return m_column; }

    std::string toString() const;

private:
    TokenType m_type;
    std::string m_lexeme;
    size_t m_line;
    size_t m_column;
};
