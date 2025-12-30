#ifndef CPP_LEXER_GRAMMAR_H
#define CPP_LEXER_GRAMMAR_H

#include <string>
#include <unordered_map>
#include "parserlib.hpp"


using namespace parserlib;


class cpp_lexer_grammar {
public:
    // --- Token IDs ---
    enum class match_id_type {
        // Core Types
        NUMBER,
        IDENTIFIER,
        STRING_LITERAL,

        // Basic Symbols/Delimiters
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        SEMICOLON,
        COMMA,
        COLON,
        DOT,

        // Unary/Binary Operators
        PLUS,
        MINUS,
        MUL,
        DIV,
        ASSIGN,
        LT,
        GT,
        BIT_AND,
        BIT_OR,
        BIT_XOR,
        BIT_NOT,
        LOGICAL_NOT,

        // Compound/Multi-Character Operators (Longest first)
        PLUS_ASSIGN,
        MINUS_ASSIGN,
        MUL_ASSIGN,
        DIV_ASSIGN,
        EQ,
        NE,
        LE,
        GE,
        INCREMENT,
        DECREMENT,
        LOGICAL_AND,
        LOGICAL_OR,
        RIGHT_SHIFT,
        LEFT_SHIFT,
        ARROW,
        SCOPE_RES,

        // Keywords (Comprehensive set)
        AUTO,
        BOOL,
        BREAK,
        CASE,
        CATCH,
        CHAR,
        CLASS,
        CONST,
        CONST_CAST,
        CONTINUE,
        DEFAULT,
        DELETE,
        DO,
        DOUBLE,
        DYNAMIC_CAST,
        ELSE,
        EXTERN,
        FLOAT,
        FOR,
        GOTO,
        IF,
        INT,
        LONG,
        NAMESPACE,
        NEW,
        OPERATOR,
        PRIVATE,
        PROTECTED,
        PUBLIC,
        REGISTER,
        REINTERPRET_CAST,
        RETURN,
        SHORT,
        SIZEOF,
        STATIC,
        STATIC_CAST,
        STRUCT,
        SWITCH,
        TEMPLATE,
        THIS,
        THROW,
        TRY,
        TYPEDEF,
        TYPEID,
        UNION,
        UNSIGNED,
        USING,
        VIRTUAL,
        VOID,
        VOLATILE,
        WCHAR_T,
        WHILE,
        ENUM,

        COMMENT_SINGLE_LINE,
        COMMENT_MULTI_LINE,
        HASH,
        PREPROCESSOR_DIRECTIVE,
        QUESTION_MARK,
        LEFT_BRACKET,
        RIGHT_BRACKET,

        INCLUDE,
        DEFINE,
        IFDEF,
        IFNDEF,
        ENDIF
    };

    // --- Error IDs ---
    enum class error_id_type {
        INVALID_TOKEN,
        UNCLOSED_STRING_LITERAL
    };

    // --- Parse context type ---
    using parse_context_type = parse_context<std::string::const_iterator, match_id_type, error_id_type, default_symbol_comparator>;

    // --- parse node pointer type ---
    using parse_node_ptr_type = parse_node_ptr<parse_context_type>;

    // --- grammar type ---
    using grammar_type = parse_node_ptr_type;

    // --- returns string from match id type ---
    static const char* match_id_to_string(match_id_type id);

    // --- parse ---
    static bool parse(parse_context_type& pc);

    // -- get the keyword grammar
    static grammar_type get_keyword_grammar();

    // --- get the grammar ---
    static grammar_type get_grammar();

};


#endif // CPP_LEXER_GRAMMAR_H
