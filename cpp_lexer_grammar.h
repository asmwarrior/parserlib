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
        RIGHT_BRACKET
    };

    // --- Error IDs ---
    enum class error_id_type {
        INVALID_TOKEN,
        UNCLOSED_STRING_LITERAL
    };

    // --- Helper to convert token ID to string ---
    static const char* match_id_to_string(match_id_type id);

    // --- Keyword map (string -> match_id_type) ---
    static const std::unordered_map<std::string, match_id_type>& keyword_map();

    template <class ParseContext>
    auto parse(ParseContext& pc) const noexcept;

private:
    static const auto& build_grammar()
    {
        static const auto grammar = []{
            const auto newline = terminal("\r\n") | terminal('\n') | terminal('\r');
            const auto whitespace = terminal(' ') | terminal('\t') | newline;

            const auto digit = range('0', '9');
            const auto letter = range('a', 'z') | range('A', 'Z') | terminal('_');

            // --- Comment Rules (Skipped) ---
            // Single-line comment: // up to the end of the line (but not including the newline)
            // const auto single_line_comment = (terminal("//") >> *(any() - newline))->*match_id_type::COMMENT_SINGLE_LINE;
            const auto single_line_comment = terminal("//") >> *(any() - newline);

            // Multi-line comment: /* followed by anything non-greedily, ending with */
            // NOTE: A true regex for C-style comments is complex due to greediness and nesting.
            // We use a simplified pattern that matches "/*" followed by any characters until "*/".
            // A robust lexer would often use state management for this.
            const auto multi_line_comment_content = *(any() - terminal("*/"));
            // const auto multi_line_comment = (terminal("/*") >> multi_line_comment_content >> terminal("*/"))->*match_id_type::COMMENT_MULTI_LINE;
            const auto multi_line_comment = terminal("/*") >> multi_line_comment_content >> terminal("*/");

            // --- ADDED: Preprocessor Directive Rule ---
            // Matches '#' followed by everything until the newline. Since it has NO ->*match_id_type, it's skipped.
            const auto preprocessor_directive = terminal('#') >> *(any() - newline);

            // The 'skippable' element: whitespace OR a comment. We use '+' to match one or more.
            const auto skip_element = *(whitespace | single_line_comment | multi_line_comment | preprocessor_directive);


            // --- Token Definitions ---

            // Number token
            const auto number = (+digit >> -('.' >> +digit))->*match_id_type::NUMBER;

            // String Literal
            // Matches '"' followed by any character that is not '"' or a newline, or an escaped character.
            const auto string_char = terminal("\\\"") | terminal("\\\t") | terminal("\\\r") | terminal("\\\n") | (any() - terminal('\"'));
            const auto string_literal = (terminal('\"') >> *string_char >> terminal('\"'))->*match_id_type::STRING_LITERAL;

            // Identifier token
            const auto identifier = (+letter >> *(letter | digit))->*match_id_type::IDENTIFIER;

            // --- Operators and Symbols (Longest match first) ---

            // 3-char operators/symbols
            const auto left_shift_assign  = terminal("<<=") ->* match_id_type::LEFT_SHIFT;
            const auto right_shift_assign = terminal(">>=") ->* match_id_type::RIGHT_SHIFT;
            const auto ellipsis           = terminal("...") ->* match_id_type::COMMA; // Represents '...' variadic operator

            // 2-char operators/symbols
            const auto scope_res    = terminal("::") ->* match_id_type::SCOPE_RES;
            const auto logical_or   = terminal("||") ->* match_id_type::LOGICAL_OR;
            const auto logical_and  = terminal("&&") ->* match_id_type::LOGICAL_AND;
            const auto eq           = terminal("==") ->* match_id_type::EQ;
            const auto ne           = terminal("!=") ->* match_id_type::NE;
            const auto le           = terminal("<=") ->* match_id_type::LE;
            const auto ge           = terminal(">=") ->* match_id_type::GE;
            const auto plus_assign  = terminal("+=") ->* match_id_type::PLUS_ASSIGN;
            const auto minus_assign = terminal("-=") ->* match_id_type::MINUS_ASSIGN;
            const auto mul_assign   = terminal("*=") ->* match_id_type::MUL_ASSIGN;
            const auto div_assign   = terminal("/=") ->* match_id_type::DIV_ASSIGN;
            const auto increment    = terminal("++") ->* match_id_type::INCREMENT;
            const auto decrement    = terminal("--") ->* match_id_type::DECREMENT;
            const auto right_shift  = terminal(">>") ->* match_id_type::RIGHT_SHIFT;
            const auto left_shift   = terminal("<<") ->* match_id_type::LEFT_SHIFT;
            const auto arrow        = terminal("->") ->* match_id_type::ARROW;

            // 1-char operators/symbols
            const auto assign        = terminal('=')->*match_id_type::ASSIGN;
            const auto logical_not   = terminal('!')->*match_id_type::LOGICAL_NOT;
            const auto lt            = terminal('<')->*match_id_type::LT;
            const auto gt            = terminal('>')->*match_id_type::GT;
            const auto plus          = terminal('+')->*match_id_type::PLUS;
            const auto minus         = terminal('-')->*match_id_type::MINUS;
            const auto mul           = terminal('*')->*match_id_type::MUL;
            const auto div           = terminal('/')->*match_id_type::DIV;
            const auto bit_and       = terminal('&')->*match_id_type::BIT_AND;
            const auto bit_or        = terminal('|')->*match_id_type::BIT_OR;
            const auto bit_xor       = terminal('^')->*match_id_type::BIT_XOR;
            const auto bit_not       = terminal('~')->*match_id_type::BIT_NOT;
            const auto lparen        = terminal('(')->*match_id_type::LEFT_PAREN;
            const auto rparen        = terminal(')')->*match_id_type::RIGHT_PAREN;
            const auto lbrace        = terminal('{')->*match_id_type::LEFT_BRACE;
            const auto rbrace        = terminal('}')->*match_id_type::RIGHT_BRACE;
            const auto semicolon     = terminal(';')->*match_id_type::SEMICOLON;
            const auto comma         = terminal(',')->*match_id_type::COMMA;
            const auto colon         = terminal(':')->*match_id_type::COLON;
            const auto dot           = terminal('.')->*match_id_type::DOT;
            const auto question_mark = terminal('?')->*match_id_type::QUESTION_MARK;
            const auto lbracket      = terminal('[')->*match_id_type::LEFT_BRACKET;
            const auto rbracket      = terminal(']')->*match_id_type::RIGHT_BRACKET;

            // Token: Ordered from longest/most specific to shortest/least specific
            const auto token =
                string_literal | number |
                // 3-char ops
                left_shift_assign | right_shift_assign | ellipsis |
                // 2-char ops
                scope_res | logical_or | logical_and | eq | ne | le | ge |
                plus_assign | minus_assign | mul_assign | div_assign |
                increment | decrement | right_shift | left_shift | arrow |
                // 1-char ops/symbols
                assign | logical_not | lt | gt | plus | minus | mul | div |
                bit_and | bit_or | bit_xor | bit_not |
                lparen | rparen | lbrace | rbrace | semicolon | comma |
                colon | dot | question_mark |
                // Identifier comes last to prevent consuming keywords
                identifier;

            // Grammar: zero or more 'skip_element' OR 'token'
            const auto grammar = *( skip_element >> token >> skip_element );
            return grammar;
        }();
        return grammar;
    }
};


template <class ParseContext>
auto cpp_lexer_grammar::parse(ParseContext& pc) const noexcept {
    const auto& grammar = build_grammar();
    auto result = grammar.parse(pc);

    for (auto& m : pc.matches()) {
        if (m.id() == match_id_type::IDENTIFIER) {
            std::string text{ parserlib::source(m.begin(), m.end()) };
            auto it = keyword_map().find(text);
            if (it != keyword_map().end())
                m.set_id(it->second);
        }
    }
    return result;
}

#endif // CPP_LEXER_GRAMMAR_H
