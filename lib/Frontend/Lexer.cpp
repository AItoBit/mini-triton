#include "MiniTriton/Frontend/Lexer.h"
#include <cctype>
#include <unordered_map>
#include <algorithm>

namespace minitriton {

// -----------------------------------------------------------------------
// tokenKindToString
// -----------------------------------------------------------------------
const char* tokenKindToString(TokenKind kind) {
    switch (kind) {
        case TokenKind::Integer:       return "Integer";
        case TokenKind::Float:         return "Float";
        case TokenKind::String:        return "String";
        case TokenKind::Identifier:    return "Identifier";
        case TokenKind::KW_def:        return "def";
        case TokenKind::KW_return:     return "return";
        case TokenKind::KW_if:         return "if";
        case TokenKind::KW_else:       return "else";
        case TokenKind::KW_kernel:     return "kernel";
        case TokenKind::KW_load:       return "load";
        case TokenKind::KW_store:      return "store";
        case TokenKind::KW_program_id: return "program_id";
        case TokenKind::KW_arange:     return "arange";
        case TokenKind::KW_reduce_sum: return "reduce_sum";
        case TokenKind::KW_constant:   return "constant";
        case TokenKind::KW_max:        return "max";
        case TokenKind::KW_i32:        return "i32";
        case TokenKind::KW_f32:        return "f32";
        case TokenKind::KW_ptr:        return "ptr";
        case TokenKind::Plus:          return "Plus";
        case TokenKind::Minus:         return "Minus";
        case TokenKind::Star:          return "Star";
        case TokenKind::Slash:         return "Slash";
        case TokenKind::Less:          return "Less";
        case TokenKind::Greater:       return "Greater";
        case TokenKind::LessEqual:     return "LessEqual";
        case TokenKind::GreaterEqual:  return "GreaterEqual";
        case TokenKind::EqualEqual:    return "EqualEqual";
        case TokenKind::NotEqual:      return "NotEqual";
        case TokenKind::Equal:         return "Equal";
        case TokenKind::At:            return "At";
        case TokenKind::Colon:         return "Colon";
        case TokenKind::Comma:         return "Comma";
        case TokenKind::Dot:           return "Dot";
        case TokenKind::Arrow:         return "Arrow";
        case TokenKind::LParen:        return "LParen";
        case TokenKind::RParen:        return "RParen";
        case TokenKind::LAngle:        return "LAngle";
        case TokenKind::RAngle:        return "RAngle";
        case TokenKind::Newline:       return "Newline";
        case TokenKind::Indent:        return "Indent";
        case TokenKind::Dedent:        return "Dedent";
        case TokenKind::EndOfFile:     return "EOF";
        case TokenKind::Error:         return "Error";
    }
    return "Unknown";
}

std::ostream& operator<<(std::ostream& os, const Token& tok) {
    os << tokenKindToString(tok.kind);
    if (!tok.value.empty()) {
        os << "('" << tok.value << "')";
    }
    os << " [" << tok.line << ":" << tok.col << "]";
    return os;
}

// -----------------------------------------------------------------------
// Lexer Construction
// -----------------------------------------------------------------------

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename), pos_(0), line_(1), col_(1),
      pendingDedents_(0), atLineStart_(true), emittedNewline_(false) {
    indentStack_.push(0);  // Base indentation level
}

// -----------------------------------------------------------------------
// Character operations
// -----------------------------------------------------------------------

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[pos_];
}

char Lexer::peekNext() const {
    if (pos_ + 1 >= source_.size()) return '\0';
    return source_[pos_ + 1];
}

char Lexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[pos_] != expected) return false;
    advance();
    return true;
}

// -----------------------------------------------------------------------
// Token production
// -----------------------------------------------------------------------

Token Lexer::makeToken(TokenKind kind, const std::string& value) {
    return Token(kind, value, line_, col_);
}

Token Lexer::makeError(const std::string& message) {
    return Token(TokenKind::Error, message, line_, col_);
}

// -----------------------------------------------------------------------
// Keyword lookup
// -----------------------------------------------------------------------

TokenKind Lexer::identifierKind(const std::string& text) const {
    static const std::unordered_map<std::string, TokenKind> keywords = {
        {"def",        TokenKind::KW_def},
        {"return",     TokenKind::KW_return},
        {"if",         TokenKind::KW_if},
        {"else",       TokenKind::KW_else},
        {"kernel",     TokenKind::KW_kernel},
        {"load",       TokenKind::KW_load},
        {"store",      TokenKind::KW_store},
        {"program_id", TokenKind::KW_program_id},
        {"arange",     TokenKind::KW_arange},
        {"reduce_sum", TokenKind::KW_reduce_sum},
        {"constant",   TokenKind::KW_constant},
        {"max",        TokenKind::KW_max},
        {"i32",        TokenKind::KW_i32},
        {"f32",        TokenKind::KW_f32},
        {"ptr",        TokenKind::KW_ptr},
    };
    auto it = keywords.find(text);
    return (it != keywords.end()) ? it->second : TokenKind::Identifier;
}

// -----------------------------------------------------------------------
// Scanning routines
// -----------------------------------------------------------------------

Token Lexer::scanNumber() {
    int startCol = col_;
    std::string num;
    bool isFloat = false;

    while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (isFloat) break;  // Second dot, stop
            isFloat = true;
        }
        num += advance();
    }

    TokenKind kind = isFloat ? TokenKind::Float : TokenKind::Integer;
    return Token(kind, num, line_, startCol);
}

Token Lexer::scanIdentifierOrKeyword() {
    int startCol = col_;
    std::string text;

    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        text += advance();
    }

    TokenKind kind = identifierKind(text);
    return Token(kind, text, line_, startCol);
}

Token Lexer::scanString() {
    int startCol = col_;
    char quote = advance();  // consume opening quote
    std::string str;

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\') {
            advance();
            if (!isAtEnd()) str += advance();
        } else {
            str += advance();
        }
    }

    if (isAtEnd()) {
        return Token(TokenKind::Error, "Unterminated string", line_, startCol);
    }
    advance();  // consume closing quote
    return Token(TokenKind::String, str, line_, startCol);
}

void Lexer::skipComment() {
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

Token Lexer::handleIndentation() {
    // Count leading spaces at the start of a line
    int indent = 0;
    size_t savedPos = pos_;
    int savedCol = col_;

    while (!isAtEnd() && peek() == ' ') {
        advance();
        indent++;
    }

    // Skip blank lines and comment-only lines
    if (isAtEnd() || peek() == '\n' || peek() == '#') {
        atLineStart_ = false;  // Will be reset when we hit newline
        if (peek() == '#') skipComment();
        return Token();  // sentinel, caller will skip
    }

    int currentIndent = indentStack_.top();

    if (indent > currentIndent) {
        indentStack_.push(indent);
        atLineStart_ = false;
        return Token(TokenKind::Indent, "", line_, 1);
    } else if (indent < currentIndent) {
        // Count how many dedents we need
        pendingDedents_ = 0;
        while (!indentStack_.empty() && indentStack_.top() > indent) {
            indentStack_.pop();
            pendingDedents_++;
        }
        if (indentStack_.empty() || indentStack_.top() != indent) {
            return Token(TokenKind::Error, "Inconsistent indentation", line_, 1);
        }
        atLineStart_ = false;
        pendingDedents_--;
        return Token(TokenKind::Dedent, "", line_, 1);
    }

    atLineStart_ = false;
    return Token();  // Same indentation, no token needed
}

Token Lexer::scanToken() {
    // Handle pending dedents
    if (pendingDedents_ > 0) {
        pendingDedents_--;
        return Token(TokenKind::Dedent, "", line_, 1);
    }

    // Handle indentation at line start
    if (atLineStart_) {
        Token indentTok = handleIndentation();
        if (indentTok.kind != TokenKind::EndOfFile) {
            return indentTok;
        }
        // No indent change, fall through to scan actual token
    }

    // Skip spaces (not newlines)
    while (!isAtEnd() && peek() == ' ') {
        advance();
    }

    if (isAtEnd()) {
        // Emit remaining dedents
        if (indentStack_.size() > 1) {
            indentStack_.pop();
            return Token(TokenKind::Dedent, "", line_, col_);
        }
        return Token(TokenKind::EndOfFile, "", line_, col_);
    }

    char c = peek();

    // Comments
    if (c == '#') {
        skipComment();
        return scanToken();  // Recurse to get next meaningful token
    }

    // Newlines
    if (c == '\n') {
        advance();
        atLineStart_ = true;
        if (!emittedNewline_) {
            emittedNewline_ = true;
            return Token(TokenKind::Newline, "\\n", line_ - 1, col_);
        }
        return scanToken();  // Skip consecutive newlines
    }

    // Carriage return (Windows)
    if (c == '\r') {
        advance();
        return scanToken();
    }

    // Tab (treat as error for clarity)
    if (c == '\t') {
        advance();
        return makeError("Tabs not allowed, use spaces");
    }

    emittedNewline_ = false;

    // Numbers
    if (std::isdigit(c)) {
        return scanNumber();
    }

    // Identifiers / keywords
    if (std::isalpha(c) || c == '_') {
        return scanIdentifierOrKeyword();
    }

    // Strings
    if (c == '"' || c == '\'') {
        return scanString();
    }

    // Operators and punctuation
    int startCol = col_;
    advance();

    switch (c) {
        case '+': return Token(TokenKind::Plus,    "+", line_, startCol);
        case '-':
            if (peek() == '>') { advance(); return Token(TokenKind::Arrow, "->", line_, startCol); }
            return Token(TokenKind::Minus, "-", line_, startCol);
        case '*': return Token(TokenKind::Star,    "*", line_, startCol);
        case '/': return Token(TokenKind::Slash,   "/", line_, startCol);
        case '(': return Token(TokenKind::LParen,  "(", line_, startCol);
        case ')': return Token(TokenKind::RParen,  ")", line_, startCol);
        case '@': return Token(TokenKind::At,      "@", line_, startCol);
        case ':': return Token(TokenKind::Colon,   ":", line_, startCol);
        case ',': return Token(TokenKind::Comma,   ",", line_, startCol);
        case '.': return Token(TokenKind::Dot,     ".", line_, startCol);
        case '<':
            if (peek() == '=') { advance(); return Token(TokenKind::LessEqual, "<=", line_, startCol); }
            return Token(TokenKind::Less, "<", line_, startCol);
        case '>':
            if (peek() == '=') { advance(); return Token(TokenKind::GreaterEqual, ">=", line_, startCol); }
            return Token(TokenKind::Greater, ">", line_, startCol);
        case '=':
            if (peek() == '=') { advance(); return Token(TokenKind::EqualEqual, "==", line_, startCol); }
            return Token(TokenKind::Equal, "=", line_, startCol);
        case '!':
            if (peek() == '=') { advance(); return Token(TokenKind::NotEqual, "!=", line_, startCol); }
            return Token(TokenKind::Error, std::string("Unexpected '!' — did you mean '!='?"), line_, startCol);
        default:
            return Token(TokenKind::Error, std::string("Unexpected character '") + c + "'", line_, startCol);
    }
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

Token Lexer::nextToken() {
    return scanToken();
}

bool Lexer::hasMore() const {
    return !isAtEnd() || pendingDedents_ > 0 || indentStack_.size() > 1;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token tok = nextToken();

        // Skip sentinel tokens (empty tokens from indentation handling)
        if (tok.kind == TokenKind::EndOfFile && tok.value.empty() && tokens.empty()) {
            // Check if it's a real EOF
            if (isAtEnd() && indentStack_.size() <= 1 && pendingDedents_ <= 0) {
                tokens.push_back(tok);
                break;
            }
            continue;
        }

        tokens.push_back(tok);

        if (tok.kind == TokenKind::EndOfFile) break;
    }

    return tokens;
}

} // namespace minitriton
