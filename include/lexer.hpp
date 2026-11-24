#pragma once

#include "token.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(std::string source);

    // Scans and returns the next token from the source stream
    Token nextToken();

    // Convenience method to tokenize the entire input until EOF_TOKEN
    std::vector<Token> tokenizeAll();

    bool hasErrors() const { return m_hasErrors; }
    const std::vector<std::string>& getErrors() const { return m_errors; }

private:
    enum class DFAState {
        START,
        IN_ID,
        IN_NUM,
        SAW_SLASH,
        IN_COMMENT,
        SAW_ASSIGN,
        SAW_EXCLAMATION,
        SAW_LT,
        SAW_GT
    };

    char peek() const;
    char advance();
    bool isWhitespace(char ch) const;
    TokenType lookupKeywordOrId(const std::string& lexeme) const;
    void reportError(size_t line, size_t column, const std::string& message);

    std::string m_source;
    size_t m_cursor{0};
    size_t m_line{1};
    size_t m_column{1};

    bool m_hasErrors{false};
    std::vector<std::string> m_errors;

    static const std::unordered_map<std::string, TokenType> s_keywords;
};
