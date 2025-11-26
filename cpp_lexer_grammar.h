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
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
        SEMICOLON, COMMA, COLON, DOT,

        // Unary/Binary Operators
        PLUS, MINUS, MUL, DIV,
        ASSIGN, LT, GT,
        BIT_AND, BIT_OR, BIT_XOR, BIT_NOT,
        LOGICAL_NOT,

        // Compound/Multi-Character Operators (Longest first)
        PLUS_ASSIGN, MINUS_ASSIGN, MUL_ASSIGN, DIV_ASSIGN,
        EQ, NE, LE, GE,
        INCREMENT, DECREMENT,
        LOGICAL_AND, LOGICAL_OR,
        RIGHT_SHIFT, LEFT_SHIFT,
        ARROW, SCOPE_RES, // ->, ::

        // Keywords (Comprehensive set)
        AUTO, BOOL, BREAK, CASE, CATCH, CHAR, CLASS, CONST, CONST_CAST, CONTINUE,
        DEFAULT, DELETE, DO, DOUBLE, DYNAMIC_CAST, ELSE, EXTERN, FLOAT, FOR, GOTO,
        IF, INT, LONG, NAMESPACE, NEW, OPERATOR, PRIVATE, PROTECTED, PUBLIC,
        REGISTER, REINTERPRET_CAST, RETURN, SHORT, SIZEOF, STATIC, STATIC_CAST,
        STRUCT, SWITCH, TEMPLATE, THIS, THROW, TRY, TYPEDEF, TYPEID,
        UNION, UNSIGNED, USING, VIRTUAL, VOID, VOLATILE, WCHAR_T, WHILE, ENUM,

        // NOTE: Comments and general whitespace are typically *not* assigned IDs,
        // or they are assigned a special ID that is immediately discarded.
        // For simplicity here, they are just matched and included in the grammar.
        // The parserlib framework's `*` operator on the grammar suggests they are skipped/ignored.

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

    using parse_context_type = parse_context<std::string, match_id_type, error_id_type, text_position, default_symbol_comparator_type>;





    // --- Helper to convert token ID to string ---
    static const char* match_id_to_string(match_id_type id);

    // --- Keyword map (string -> match_id_type) ---
    static const std::unordered_map<std::string, match_id_type>& keyword_map();

    parse_result parse(parse_context_type& pc) const noexcept;

    using grammar_type = rule<parse_context_type>;
    static grammar_type& get_grammar();

private:


};


#endif // CPP_LEXER_GRAMMAR_H
