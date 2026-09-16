#pragma once

#include <string>
#include <ostream>

namespace minitriton {

enum class TokenKind {
    // Literals
    Integer,
    Float,
    String,

    // Identifiers & keywords
    Identifier,
    KW_def,
    KW_return,
    KW_if,
    KW_else,

    // Builtin keywords
    KW_kernel,
    KW_load,
    KW_store,
    KW_program_id,
    KW_arange,
    KW_reduce_sum,
    KW_constant,
    KW_max,

    // Type keywords
    KW_i32,
    KW_f32,
    KW_ptr,

    // Operators
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /

    // Comparison
    Less,           // <
    Greater,        // >
    LessEqual,      // <=
    GreaterEqual,   // >=
    EqualEqual,     // ==
    NotEqual,       // !=

    // Assignment & symbols
    Equal,          // =
    At,             // @
    Colon,          // :
    Comma,          // ,
    Dot,            // .
    Arrow,          // ->

    // Delimiters
    LParen,         // (
    RParen,         // )
    LAngle,         // < (type context)
    RAngle,         // > (type context)

    // Structure (indentation-based)
    Newline,
    Indent,
    Dedent,

    // Special
    EndOfFile,
    Error
};

struct Token {
    TokenKind kind;
    std::string value;
    int line;
    int col;

    Token() : kind(TokenKind::EndOfFile), line(0), col(0) {}
    Token(TokenKind k, const std::string& v, int l, int c)
        : kind(k), value(v), line(l), col(c) {}
};

// Convert TokenKind to string for debugging
const char* tokenKindToString(TokenKind kind);

// Stream output for Token
std::ostream& operator<<(std::ostream& os, const Token& tok);

} // namespace minitriton
