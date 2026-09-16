#pragma once

#include "MiniTriton/Frontend/Token.h"
#include <string>
#include <vector>
#include <stack>

namespace minitriton {

class Lexer {
public:
    explicit Lexer(const std::string& source, const std::string& filename = "<input>");

    /// Tokenize the entire source and return all tokens.
    std::vector<Token> tokenize();

    /// Get the next token from the source.
    Token nextToken();

    /// Check if there are more tokens.
    bool hasMore() const;

    /// Get filename for error reporting.
    const std::string& getFilename() const { return filename_; }

private:
    // Source management
    std::string source_;
    std::string filename_;
    size_t pos_;
    int line_;
    int col_;

    // Indentation tracking (Python-style)
    std::stack<int> indentStack_;
    int pendingDedents_;
    bool atLineStart_;
    bool emittedNewline_;

    // Character operations
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    bool match(char expected);

    // Token production
    Token makeToken(TokenKind kind, const std::string& value);
    Token makeError(const std::string& message);

    // Scanning routines
    Token scanToken();
    Token scanNumber();
    Token scanIdentifierOrKeyword();
    Token scanString();
    void skipComment();
    Token handleNewline();
    Token handleIndentation();

    // Keyword lookup
    TokenKind identifierKind(const std::string& text) const;
};

} // namespace minitriton
