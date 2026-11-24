#include "lexer.hpp"
#include <cctype>
#include <iostream>

std::string_view tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_INT:     return "KW_INT";
        case TokenType::KW_IF:      return "KW_IF";
        case TokenType::KW_ELSE:    return "KW_ELSE";
        case TokenType::KW_WHILE:   return "KW_WHILE";
        case TokenType::ID:         return "ID";
        case TokenType::NUM:        return "NUM";
        case TokenType::PLUS:       return "PLUS";
        case TokenType::MINUS:      return "MINUS";
        case TokenType::STAR:       return "STAR";
        case TokenType::SLASH:      return "SLASH";
        case TokenType::ASSIGN:     return "ASSIGN";
        case TokenType::EQ:         return "EQ";
        case TokenType::NEQ:        return "NEQ";
        case TokenType::LT:         return "LT";
        case TokenType::LE:         return "LE";
        case TokenType::GT:         return "GT";
        case TokenType::GE:         return "GE";
        case TokenType::LPAREN:     return "LPAREN";
        case TokenType::RPAREN:     return "RPAREN";
        case TokenType::LBRACE:     return "LBRACE";
        case TokenType::RBRACE:     return "RBRACE";
        case TokenType::SEMICOLON:  return "SEMICOLON";
        case TokenType::EOF_TOKEN:  return "EOF_TOKEN";
        case TokenType::ERROR:      return "ERROR";
    }
    return "UNKNOWN";
}

std::string Token::toString() const {
    return "[" + std::to_string(m_line) + ":" + std::to_string(m_column) + "] (" +
           std::string(tokenTypeToString(m_type)) + ", \"" + m_lexeme + "\")";
}

const std::unordered_map<std::string, TokenType> Lexer::s_keywords = {
    {"int", TokenType::KW_INT},
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE}
};

Lexer::Lexer(std::string source) : m_source(std::move(source)) {}

char Lexer::peek() const {
    if (m_cursor >= m_source.size()) {
        return '\0';
    }
    char ch = m_source[m_cursor];
    if (ch == '\r') {
        return '\n';
    }
    return ch;
}

char Lexer::advance() {
    if (m_cursor >= m_source.size()) {
        return '\0';
    }
    char ch = m_source[m_cursor++];
    if (ch == '\r') {
        // Handle Windows CRLF or bare CR
        if (m_cursor < m_source.size() && m_source[m_cursor] == '\n') {
            m_cursor++;
        }
        m_line++;
        m_column = 1;
        return '\n';
    }
    if (ch == '\n') {
        m_line++;
        m_column = 1;
        return '\n';
    }
    m_column++;
    return ch;
}

bool Lexer::isWhitespace(char ch) const {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

TokenType Lexer::lookupKeywordOrId(const std::string& lexeme) const {
    auto it = s_keywords.find(lexeme);
    if (it != s_keywords.end()) {
        return it->second;
    }
    return TokenType::ID;
}

void Lexer::reportError(size_t line, size_t column, const std::string& message) {
    m_hasErrors = true;
    m_errors.push_back("[" + std::to_string(line) + ":" + std::to_string(column) + "] " + message);
    std::cerr << "Lexical error at [" << line << ":" << column << "]: " << message << "\n";
}

Token Lexer::nextToken() {
    DFAState state = DFAState::START;
    std::string lexeme;
    size_t startLine = m_line;
    size_t startCol = m_column;

    while (true) {
        char ch = peek();

        switch (state) {
            case DFAState::START: {
                if (ch == '\0') {
                    return Token(TokenType::EOF_TOKEN, "EOF", m_line, m_column);
                }

                // Skip whitespace characters
                if (isWhitespace(ch)) {
                    advance();
                    continue;
                }

                startLine = m_line;
                startCol = m_column;

                if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
                    lexeme += advance();
                    state = DFAState::IN_ID;
                } else if (std::isdigit(static_cast<unsigned char>(ch))) {
                    lexeme += advance();
                    state = DFAState::IN_NUM;
                } else if (ch == '/') {
                    advance();
                    state = DFAState::SAW_SLASH;
                } else if (ch == '=') {
                    advance();
                    state = DFAState::SAW_ASSIGN;
                } else if (ch == '!') {
                    advance();
                    state = DFAState::SAW_EXCLAMATION;
                } else if (ch == '<') {
                    advance();
                    state = DFAState::SAW_LT;
                } else if (ch == '>') {
                    advance();
                    state = DFAState::SAW_GT;
                } else if (ch == '+') {
                    advance();
                    return Token(TokenType::PLUS, "+", startLine, startCol);
                } else if (ch == '-') {
                    advance();
                    return Token(TokenType::MINUS, "-", startLine, startCol);
                } else if (ch == '*') {
                    advance();
                    return Token(TokenType::STAR, "*", startLine, startCol);
                } else if (ch == ';') {
                    advance();
                    return Token(TokenType::SEMICOLON, ";", startLine, startCol);
                } else if (ch == '(') {
                    advance();
                    return Token(TokenType::LPAREN, "(", startLine, startCol);
                } else if (ch == ')') {
                    advance();
                    return Token(TokenType::RPAREN, ")", startLine, startCol);
                } else if (ch == '{') {
                    advance();
                    return Token(TokenType::LBRACE, "{", startLine, startCol);
                } else if (ch == '}') {
                    advance();
                    return Token(TokenType::RBRACE, "}", startLine, startCol);
                } else {
                    char badChar = advance();
                    std::string badStr(1, badChar);
                    reportError(startLine, startCol, "Unexpected character '" + badStr + "'");
                    return Token(TokenType::ERROR, badStr, startLine, startCol);
                }
                break;
            }

            case DFAState::IN_ID: {
                if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
                    lexeme += advance();
                } else {
                    TokenType type = lookupKeywordOrId(lexeme);
                    return Token(type, lexeme, startLine, startCol);
                }
                break;
            }

            case DFAState::IN_NUM: {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    lexeme += advance();
                } else {
                    return Token(TokenType::NUM, lexeme, startLine, startCol);
                }
                break;
            }

            case DFAState::SAW_SLASH: {
                if (ch == '/') {
                    advance();
                    state = DFAState::IN_COMMENT;
                } else {
                    return Token(TokenType::SLASH, "/", startLine, startCol);
                }
                break;
            }

            case DFAState::IN_COMMENT: {
                if (ch == '\n') {
                    advance();
                    state = DFAState::START;
                } else if (ch == '\0') {
                    state = DFAState::START;
                } else {
                    advance();
                }
                break;
            }

            case DFAState::SAW_ASSIGN: {
                if (ch == '=') {
                    advance();
                    return Token(TokenType::EQ, "==", startLine, startCol);
                }
                return Token(TokenType::ASSIGN, "=", startLine, startCol);
            }

            case DFAState::SAW_EXCLAMATION: {
                if (ch == '=') {
                    advance();
                    return Token(TokenType::NEQ, "!=", startLine, startCol);
                }
                reportError(startLine, startCol, "Unexpected character '!' (did you mean '!=')");
                return Token(TokenType::ERROR, "!", startLine, startCol);
            }

            case DFAState::SAW_LT: {
                if (ch == '=') {
                    advance();
                    return Token(TokenType::LE, "<=", startLine, startCol);
                }
                return Token(TokenType::LT, "<", startLine, startCol);
            }

            case DFAState::SAW_GT: {
                if (ch == '=') {
                    advance();
                    return Token(TokenType::GE, ">=", startLine, startCol);
                }
                return Token(TokenType::GT, ">", startLine, startCol);
            }
        }
    }
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = nextToken();
        tokens.push_back(tok);
        if (tok.getType() == TokenType::EOF_TOKEN) {
            break;
        }
    }
    return tokens;
}
