#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <cassert>

#include <unordered_map>

#include "parserlib.hpp"

using namespace parserlib;

// --- Source type ---
using source_type = line_counting_string<>;

source_type src = R"(
// class templates1
// class templates2
// class templates3

xyz abc;
opq def;

"abcdefg";


#include <iostream>
using namespace std;

template <class T>
class mypair {
    T a, b;
  public:
    mypair (T first, T second)
      {a=first; b=second;}
    T getmax ();
};

template <class T>
T mypair<T>::getmax ()
{
  T retval;
  retval = a>b? a : b;
  return retval;
}


mypair <int> myobject (100, 75);

    )";

#include <string>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

// Assume parserlib functions (terminal, range, ->*, >>, | , +, * , - , source, terminal_not) are available.

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
    static const char* match_id_to_string(match_id_type id) {
        switch (id) {
            case match_id_type::NUMBER: return "NUMBER";
            case match_id_type::IDENTIFIER: return "IDENTIFIER";
            case match_id_type::STRING_LITERAL: return "STRING_LITERAL";
            case match_id_type::LEFT_PAREN: return "LEFT_PAREN";
            case match_id_type::RIGHT_PAREN: return "RIGHT_PAREN";
            case match_id_type::LEFT_BRACE: return "LEFT_BRACE";
            case match_id_type::RIGHT_BRACE: return "RIGHT_BRACE";
            case match_id_type::SEMICOLON: return "SEMICOLON";
            case match_id_type::COMMA: return "COMMA";
            case match_id_type::COLON: return "COLON";
            case match_id_type::DOT: return "DOT";
            case match_id_type::PLUS: return "PLUS";
            case match_id_type::MINUS: return "MINUS";
            case match_id_type::MUL: return "MUL";
            case match_id_type::DIV: return "DIV";
            case match_id_type::ASSIGN: return "ASSIGN";
            case match_id_type::LT: return "LT";
            case match_id_type::GT: return "GT";
            case match_id_type::BIT_AND: return "BIT_AND";
            case match_id_type::BIT_OR: return "BIT_OR";
            case match_id_type::BIT_XOR: return "BIT_XOR";
            case match_id_type::BIT_NOT: return "BIT_NOT";
            case match_id_type::LOGICAL_NOT: return "LOGICAL_NOT";
            case match_id_type::PLUS_ASSIGN: return "PLUS_ASSIGN";
            case match_id_type::MINUS_ASSIGN: return "MINUS_ASSIGN";
            case match_id_type::MUL_ASSIGN: return "MUL_ASSIGN";
            case match_id_type::DIV_ASSIGN: return "DIV_ASSIGN";
            case match_id_type::EQ: return "EQ";
            case match_id_type::NE: return "NE";
            case match_id_type::LE: return "LE";
            case match_id_type::GE: return "GE";
            case match_id_type::INCREMENT: return "INCREMENT";
            case match_id_type::DECREMENT: return "DECREMENT";
            case match_id_type::LOGICAL_AND: return "LOGICAL_AND";
            case match_id_type::LOGICAL_OR: return "LOGICAL_OR";
            case match_id_type::RIGHT_SHIFT: return "RIGHT_SHIFT";
            case match_id_type::LEFT_SHIFT: return "LEFT_SHIFT";
            case match_id_type::ARROW: return "ARROW";
            case match_id_type::SCOPE_RES: return "SCOPE_RES";
            case match_id_type::AUTO: return "AUTO";
            case match_id_type::BOOL: return "BOOL";
            case match_id_type::BREAK: return "BREAK";
            case match_id_type::CASE: return "CASE";
            case match_id_type::CATCH: return "CATCH";
            case match_id_type::CHAR: return "CHAR";
            case match_id_type::CLASS: return "CLASS";
            case match_id_type::CONST: return "CONST";
            case match_id_type::CONST_CAST: return "CONST_CAST";
            case match_id_type::CONTINUE: return "CONTINUE";
            case match_id_type::DEFAULT: return "DEFAULT";
            case match_id_type::DELETE: return "DELETE";
            case match_id_type::DO: return "DO";
            case match_id_type::DOUBLE: return "DOUBLE";
            case match_id_type::DYNAMIC_CAST: return "DYNAMIC_CAST";
            case match_id_type::ELSE: return "ELSE";
            case match_id_type::EXTERN: return "EXTERN";
            case match_id_type::FLOAT: return "FLOAT";
            case match_id_type::FOR: return "FOR";
            case match_id_type::GOTO: return "GOTO";
            case match_id_type::IF: return "IF";
            case match_id_type::INT: return "INT";
            case match_id_type::LONG: return "LONG";
            case match_id_type::NAMESPACE: return "NAMESPACE";
            case match_id_type::NEW: return "NEW";
            case match_id_type::OPERATOR: return "OPERATOR";
            case match_id_type::PRIVATE: return "PRIVATE";
            case match_id_type::PROTECTED: return "PROTECTED";
            case match_id_type::PUBLIC: return "PUBLIC";
            case match_id_type::REGISTER: return "REGISTER";
            case match_id_type::REINTERPRET_CAST: return "REINTERPRET_CAST";
            case match_id_type::RETURN: return "RETURN";
            case match_id_type::SHORT: return "SHORT";
            case match_id_type::SIZEOF: return "SIZEOF";
            case match_id_type::STATIC: return "STATIC";
            case match_id_type::STATIC_CAST: return "STATIC_CAST";
            case match_id_type::STRUCT: return "STRUCT";
            case match_id_type::SWITCH: return "SWITCH";
            case match_id_type::TEMPLATE: return "TEMPLATE";
            case match_id_type::THIS: return "THIS";
            case match_id_type::THROW: return "THROW";
            case match_id_type::TRY: return "TRY";
            case match_id_type::TYPEDEF: return "TYPEDEF";
            case match_id_type::TYPEID: return "TYPEID";
            case match_id_type::UNION: return "UNION";
            case match_id_type::UNSIGNED: return "UNSIGNED";
            case match_id_type::USING: return "USING";
            case match_id_type::VIRTUAL: return "VIRTUAL";
            case match_id_type::VOID: return "VOID";
            case match_id_type::VOLATILE: return "VOLATILE";
            case match_id_type::WCHAR_T: return "WCHAR_T";
            case match_id_type::WHILE: return "WHILE";
            case match_id_type::COMMENT_SINGLE_LINE: return "COMMENT_SINGLE_LINE";
            case match_id_type::COMMENT_MULTI_LINE: return "COMMENT_MULTI_LINE";
            case match_id_type::HASH: return "HASH";
            case match_id_type::PREPROCESSOR_DIRECTIVE: return "PREPROCESSOR_DIRECTIVE";
            case match_id_type::QUESTION_MARK: return "QUESTION_MARK";
            case match_id_type::LEFT_BRACKET: return "LEFT_BRACKET";
            case match_id_type::RIGHT_BRACKET: return "RIGHT_BRACKET";
            case match_id_type::ENUM: return "ENUM";
        }
        // This is necessary to avoid a compiler warning about non-void function
        // not returning a value on all control paths, even though all IDs are listed.
        throw std::logic_error("Unknown match_id_type passed to match_id_to_string");
    }

    // --- Keyword map (string -> match_id_type) ---
    static const std::unordered_map<std::string, match_id_type>& keyword_map() {
        static const std::unordered_map<std::string, match_id_type> map{
            {"if", match_id_type::IF}, {"else", match_id_type::ELSE}, {"while", match_id_type::WHILE},
            {"for", match_id_type::FOR}, {"return", match_id_type::RETURN}, {"class", match_id_type::CLASS},
            {"auto", match_id_type::AUTO}, {"bool", match_id_type::BOOL}, {"break", match_id_type::BREAK},
            {"case", match_id_type::CASE}, {"catch", match_id_type::CATCH}, {"char", match_id_type::CHAR},
            {"const", match_id_type::CONST}, {"const_cast", match_id_type::CONST_CAST}, {"continue", match_id_type::CONTINUE},
            {"default", match_id_type::DEFAULT}, {"delete", match_id_type::DELETE}, {"do", match_id_type::DO},
            {"double", match_id_type::DOUBLE}, {"dynamic_cast", match_id_type::DYNAMIC_CAST}, {"extern", match_id_type::EXTERN},
            {"float", match_id_type::FLOAT}, {"goto", match_id_type::GOTO}, {"int", match_id_type::INT},
            {"long", match_id_type::LONG}, {"namespace", match_id_type::NAMESPACE}, {"new", match_id_type::NEW},
            {"operator", match_id_type::OPERATOR}, {"private", match_id_type::PRIVATE}, {"protected", match_id_type::PROTECTED},
            {"public", match_id_type::PUBLIC}, {"register", match_id_type::REGISTER}, {"reinterpret_cast", match_id_type::REINTERPRET_CAST},
            {"short", match_id_type::SHORT}, {"sizeof", match_id_type::SIZEOF}, {"static", match_id_type::STATIC},
            {"static_cast", match_id_type::STATIC_CAST}, {"struct", match_id_type::STRUCT}, {"switch", match_id_type::SWITCH},
            {"template", match_id_type::TEMPLATE}, {"this", match_id_type::THIS}, {"throw", match_id_type::THROW},
            {"try", match_id_type::TRY}, {"typedef", match_id_type::TYPEDEF}, {"typeid", match_id_type::TYPEID},
            {"union", match_id_type::UNION}, {"unsigned", match_id_type::UNSIGNED}, {"using", match_id_type::USING},
            {"virtual", match_id_type::VIRTUAL}, {"void", match_id_type::VOID}, {"volatile", match_id_type::VOLATILE},
            {"wchar_t", match_id_type::WCHAR_T}, {"enum", match_id_type::ENUM}
        };
        return map;
    }

    template <class ParseContext>
    auto parse(ParseContext& pc) const noexcept {
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
        const auto scope_res = terminal("::") ->* match_id_type::SCOPE_RES;
        const auto logical_or = terminal("||") ->* match_id_type::LOGICAL_OR;
        const auto logical_and = terminal("&&") ->* match_id_type::LOGICAL_AND;
        const auto eq = terminal("==") ->* match_id_type::EQ;
        const auto ne = terminal("!=") ->* match_id_type::NE;
        const auto le = terminal("<=") ->* match_id_type::LE;
        const auto ge = terminal(">=") ->* match_id_type::GE;
        const auto plus_assign = terminal("+=") ->* match_id_type::PLUS_ASSIGN;
        const auto minus_assign = terminal("-=") ->* match_id_type::MINUS_ASSIGN;
        const auto mul_assign = terminal("*=") ->* match_id_type::MUL_ASSIGN;
        const auto div_assign = terminal("/=") ->* match_id_type::DIV_ASSIGN;
        const auto increment = terminal("++") ->* match_id_type::INCREMENT;
        const auto decrement = terminal("--") ->* match_id_type::DECREMENT;
        const auto right_shift = terminal(">>") ->* match_id_type::RIGHT_SHIFT;
        const auto left_shift = terminal("<<") ->* match_id_type::LEFT_SHIFT;
        const auto arrow = terminal("->") ->* match_id_type::ARROW;

        // 1-char operators/symbols
        const auto assign = terminal('=')->*match_id_type::ASSIGN;
        const auto logical_not = terminal('!')->*match_id_type::LOGICAL_NOT;
        const auto lt = terminal('<')->*match_id_type::LT;
        const auto gt = terminal('>')->*match_id_type::GT;
        const auto plus = terminal('+')->*match_id_type::PLUS;
        const auto minus = terminal('-')->*match_id_type::MINUS;
        const auto mul = terminal('*')->*match_id_type::MUL;
        const auto div = terminal('/')->*match_id_type::DIV;
        const auto bit_and = terminal('&')->*match_id_type::BIT_AND;
        const auto bit_or = terminal('|')->*match_id_type::BIT_OR;
        const auto bit_xor = terminal('^')->*match_id_type::BIT_XOR;
        const auto bit_not = terminal('~')->*match_id_type::BIT_NOT;
        const auto lparen = terminal('(')->*match_id_type::LEFT_PAREN;
        const auto rparen = terminal(')')->*match_id_type::RIGHT_PAREN;
        const auto lbrace = terminal('{')->*match_id_type::LEFT_BRACE;
        const auto rbrace = terminal('}')->*match_id_type::RIGHT_BRACE;
        const auto semicolon = terminal(';')->*match_id_type::SEMICOLON;
        const auto comma = terminal(',')->*match_id_type::COMMA;
        const auto colon = terminal(':')->*match_id_type::COLON;
        const auto dot = terminal('.')->*match_id_type::DOT;
        const auto question_mark = terminal('?')->*match_id_type::QUESTION_MARK;
        const auto lbracket = terminal('[')->*match_id_type::LEFT_BRACKET;
        const auto rbracket = terminal(']')->*match_id_type::RIGHT_BRACKET;

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

        // Parse
        auto result = grammar.parse(pc);

        // --- Postprocess: convert identifiers to keywords ---
        for (auto& m : pc.matches()) {
            if (m.id() == match_id_type::IDENTIFIER) {
                // Get the text from the source range
                std::string text{ parserlib::source(m.begin(), m.end()) };
                auto it = keyword_map().find(text);
                if (it != keyword_map().end()) {
                    m.set_id(it->second); // replace identifier with keyword
                }
            }
        }
        return result;
    }
};

using lexer_type = lexer<source_type, cpp_lexer_grammar>;


void test_cpp_lexer() {

    using lexer_t = lexer_type;

    // --- run lexer only ---
    auto result = lexer_t::parse(src);

    std::cout << "=== Lexer test ===\n";
    std::cout << "Success: " << std::boolalpha << result.success << "\n";

    // --- show tokens ---
    size_t index = 0;
    for (const auto& token : result.parsed_tokens) {
        std::string text(token.begin(), token.end());
        std::cout << "[" << index++ << "] id="
                  << cpp_lexer_grammar::match_id_to_string(token.id()) // <-- Using the string name
                  << " text='" << text << "'\n";
    }

    // --- show errors ---
    if (!result.errors.empty()) {
        std::cout << "\nLexer errors:\n";
        for (const auto& err : result.errors) {
            auto offset = std::distance(src.begin(), err.begin());
            std::cout << "  Error at offset " << offset
                      << " (error id: " << static_cast<int>(err.id()) << ")\n";
        }
    }

    std::cout << "Lexer stopped at offset "
              << std::distance(src.begin(), result.parse_position) << "\n";
}

// ----------------------------------------------------------------------
// --- Parser grammar (Unchanged) ---
class cpp_parser_grammar {
public:
    using lexer_grammar = cpp_lexer_grammar;

    enum class match_id_type {
        VAR_DECL,
        FUNC_DECL,
        FUNC_DEF,
        BLOCK,
        BLOCK_SKIP, // skipped function bodies
        CLASS_DEF,
        CLASS_ID,
        TOP_LEVEL,
        EXPRESSION_STATEMENT,
        FOR_LOOP,
        CONTROL_FLOW,
        PARAM_LIST,
        ENUM_DEF
    };

    enum class error_id_type {
        INVALID_TOKEN,
        INCOMPLETE_PARSE
    };

    template <class ParseContext>
    parse_result parse(ParseContext& pc) const noexcept {
        return instance<ParseContext>().parse(pc);
    }

    // --- Helper to convert enum to string ---
    static const char* match_id_to_string(match_id_type id) {
        switch (id) {
            case match_id_type::VAR_DECL: return "VAR_DECL";
            case match_id_type::FUNC_DECL: return "FUNC_DECL";
            case match_id_type::FUNC_DEF: return "FUNC_DEF";
            case match_id_type::BLOCK: return "BLOCK";
            case match_id_type::BLOCK_SKIP: return "BLOCK_SKIP";
            case match_id_type::CLASS_DEF: return "CLASS_DEF";
            case match_id_type::CLASS_ID: return "CLASS_ID";
            case match_id_type::TOP_LEVEL: return "TOP_LEVEL";
            case match_id_type::EXPRESSION_STATEMENT: return "EXPRESSION_STATEMENT";
            case match_id_type::FOR_LOOP: return "FOR_LOOP";
            case match_id_type::CONTROL_FLOW: return "CONTROL_FLOW";
            case match_id_type::PARAM_LIST: return "PARAM_LIST";
            case match_id_type::ENUM_DEF: return "ENUM_DEF";
        }
        return "UNKNOWN";
    }

    // --- AST printer (Modified, now a free function for reuse) ---
    template <typename Node>
    static void print_ast(const Node& node, int indent = 0) {
        std::string space(indent, ' ');

        // Use the helper function instead of casting to int
        std::cout << space << match_id_to_string(node->id()) << space << node->children().size()
                  << " \"" << node->source() << "\"\n";

        for (auto& c : node->children())
            print_ast(c, indent + 2);
    }

private:
    template <class ParseContext>
    class instance {
    public:
        instance() {
            using id_type = typename lexer_grammar::match_id_type;

            // --- Custom rule to skip a block based on nested brace levels ---
            block_skip_rule = function([](auto& pc) -> parse_result {
                using id_type = typename cpp_lexer_grammar::match_id_type;

                // The rule must start with the opening brace token
                if (!pc.is_valid_parse_position() || pc.parse_position()->id() != id_type::LEFT_BRACE) {
                    return false;
                }

                pc.increment_parse_position(); // Consume the initial '{'
                int brace_level = 1; // Start at 1, as the first '{' has been consumed

                // Loop through tokens until the brace level returns to 0
                while (brace_level > 0 && pc.is_valid_parse_position()) {
                    const auto token_id = pc.parse_position()->id();

                    if (token_id == id_type::LEFT_BRACE) {
                        brace_level++;
                    } else if (token_id == id_type::RIGHT_BRACE) {
                        brace_level--;
                    }

                    pc.increment_parse_position(); // Consume the current token (brace or content)
                }

                // Success if the matching '}' was found (brace_level is 0)
                return brace_level == 0;
            });

            // --- Custom rule to skip a full statement (e.g., to the next ';') ---
              // This respects nested parentheses, braces, and brackets.
              skip_to_semicolon_rule = function([&](auto& pc) -> parse_result {
                    using id_type = typename cpp_lexer_grammar::match_id_type;
                    int paren_level = 0;
                    int brace_level = 0;
                    int bracket_level = 0;

                    if (!pc.is_valid_parse_position()) {
                          return false; // Nothing to parse
                    }

                    while (pc.is_valid_parse_position()) {
                          const auto token_id = pc.parse_position()->id();

                          // Check for the end-of-statement
                          if (token_id == id_type::SEMICOLON) {
                                if (paren_level == 0 && brace_level == 0 && bracket_level == 0) {
                                      pc.increment_parse_position(); // Consume the ';'
                                      return true; // Found it!
                                }
                          }

                          // Update nesting levels
                          if (token_id == id_type::LEFT_PAREN)       paren_level++;
                          else if (token_id == id_type::RIGHT_PAREN)    paren_level--;
                          else if (token_id == id_type::LEFT_BRACE)       brace_level++;
                          else if (token_id == id_type::RIGHT_BRACE)    brace_level--;
                          else if (token_id == id_type::LEFT_BRACKET)    bracket_level++;
                          else if (token_id == id_type::RIGHT_BRACKET) bracket_level--;

                          pc.increment_parse_position(); // Consume the current token
                    }

                    // Reached end of input without finding a ';' at level 0
                    return false;
              });

            // --- This is the "a = ... ;" rule you wanted ---
                  // It's a "catch-all" for any simple statement:
                  // - int x = 5;
                  // - a = b + c;
                  // - do_something(1, 2);
                  // - return x;
                  // We just skip the whole thing.
                  expression_statement = (
                        skip_to_semicolon_rule
                  )->*match_id_type::EXPRESSION_STATEMENT;

                  // 'for' is special due to its '(...)' content
                  for_loop = (
                        terminal(id_type::FOR)
                        >> (paren_skip_rule->*match_id_type::PARAM_LIST) // Skips the (init; cond; inc)
                        >> (block_skip_rule | expression_statement) // Body is a block OR single statement
                  )->*match_id_type::FOR_LOOP;

                  // Basic control flow: if, while, switch
                  control_flow_statement = (
                        (terminal(id_type::IF) | terminal(id_type::WHILE) | terminal(id_type::SWITCH))
                        >> (paren_skip_rule->*match_id_type::PARAM_LIST) // Skip the (...) condition
                        >> (block_skip_rule | expression_statement) // Body is a block OR single statement
                  )->*match_id_type::CONTROL_FLOW;

                  // 'enum' definition
                  enum_def = (
                        terminal(id_type::ENUM)
                        >> -(terminal(id_type::CLASS) | terminal(id_type::STRUCT)) // Optional 'class'
                        >> terminal(id_type::IDENTIFIER) // Enum name
                        >> -(terminal(id_type::COLON) >> terminal(id_type::IDENTIFIER)) // Optional base type
                        >> (block_skip_rule->*match_id_type::BLOCK_SKIP) // Skip the { ... } enumerators
                        >> -terminal(id_type::SEMICOLON)
                  )->*match_id_type::ENUM_DEF;

            // --- Variable declaration: <type> <id> ';'
            var_decl = (terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::SEMICOLON))
                        ->*match_id_type::VAR_DECL;

//            // --- Block: '{' { var_decl | func_def } '}'
//            block = (terminal(id_type::LEFT_BRACE)
//                     >> *(var_decl | func_def | func_decl)
//                     >> terminal(id_type::RIGHT_BRACE))
//                     ->*match_id_type::BLOCK;

// --- Block: '{' { statements } '}'
                  // This rule is for parsing the *inside* of a class or namespace.
                  // We must order this from most-specific to least-specific.
                  // 'expression_statement' MUST come last.
                  block = (terminal(id_type::LEFT_BRACE)
                                >> *(
                                       // Full definitions (highest priority)
                                       func_def
                                       | class_def
                                       | enum_def
                                       | func_decl
                                       | control_flow_statement
                                       | for_loop
                                       | expression_statement
                                    )
                                >> terminal(id_type::RIGHT_BRACE))
                                ->*match_id_type::BLOCK;

                  // --- Top-level declarations ---
                  // We replace 'var_decl' with 'expression_statement' to handle
                  // global variables with initializers (e.g., int g_var = 10;)
                  top_level = (+(
                                  class_def
                                  | enum_def
                                  | func_def
                                  | func_decl
                                  | control_flow_statement
                               ))->*match_id_type::TOP_LEVEL;

            // --- Function declaration: <type> <id> '(' ')' ';';
            func_decl = (terminal(id_type::IDENTIFIER)
                         >> debug(terminal(id_type::IDENTIFIER))
                         >> terminal(id_type::LEFT_PAREN)
                         >> terminal(id_type::RIGHT_PAREN)
                         >> terminal(id_type::SEMICOLON))
                         ->* match_id_type::FUNC_DECL;


            // --- Function definition: <type> <id> '(' ')' block
            func_def = (terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::LEFT_PAREN)
                        >> terminal(id_type::RIGHT_PAREN)
                        >> (block_skip_rule->*match_id_type::BLOCK_SKIP)
                        >> -terminal(id_type::SEMICOLON)) // optional semicolon
                        ->* match_id_type::FUNC_DEF;

            // --- Class definition: 'class' IDENTIFIER block
            class_def = (terminal(id_type::CLASS)
                         >> terminal(id_type::IDENTIFIER) ->* match_id_type::CLASS_ID
                         >> block
                         >> -terminal(id_type::SEMICOLON)) // optional semicolon
                         ->* match_id_type::CLASS_DEF;

            // --- Top-level declarations: { class_def | func_def | var_decl }+
            top_level = (+(
                            class_def
                            | func_def
                            | var_decl
                            | func_decl
                          ))
                          ->*match_id_type::TOP_LEVEL;
        }

        parse_result parse(ParseContext& pc) noexcept {
            auto r = top_level.parse(pc);
            return r;
        }

    private:
        rule<ParseContext> var_decl, func_decl, func_def, block, class_def, top_level, block_skip_rule,
            paren_skip_rule, skip_to_semicolon_rule, expression_statement, for_loop, control_flow_statement, enum_def;
    };
};


// ----------------------------------------------------------------------
// AST Visitor Implementation
// ----------------------------------------------------------------------

/**
 * @brief Base AST Visitor class.
 *
 * This provides the interface for visiting each specific node type.
 * Note: Node is expected to be an AST node wrapper (e.g., std::shared_ptr<...>)
 * based on the usage in cpp_parser_grammar::print_ast.
 */
template <typename Node>
class AstVisitor {
public:
    using MatchId = typename cpp_parser_grammar::match_id_type;

    virtual ~AstVisitor() = default;

    // General visit function (dispatches to specific visit methods)
    void visit(const Node& node) {
        switch (node->id()) {
            case MatchId::TOP_LEVEL: visit_top_level(node); break;
            case MatchId::CLASS_DEF: visit_class_def(node); break;
            case MatchId::CLASS_ID:  visit_class_id(node); break;
            case MatchId::VAR_DECL:  visit_var_decl(node); break;
            case MatchId::FUNC_DECL: visit_func_decl(node); break;
            case MatchId::FUNC_DEF:  visit_func_def(node); break;
            case MatchId::BLOCK:     visit_block(node); break;
            case MatchId::BLOCK_SKIP: visit_block_skip(node); break;
            default:
                // Handle unexpected or generic nodes
                std::cout << "Warning: Unhandled node type: "
                          << cpp_parser_grammar::match_id_to_string(node->id()) << "\n";
                visit_generic(node);
                break;
        }
    }

protected:
    // Fallback for nodes that don't have a specific handler
    virtual void visit_generic(const Node& node) {
        // Default behavior: just traverse children
        for (const auto& child : node->children()) {
            visit(child);
        }
    }

    // Specific visit methods (Override these in derived classes)
    virtual void visit_top_level(const Node& node)  { visit_generic(node); }
    virtual void visit_class_def(const Node& node)  { visit_generic(node); }
    virtual void visit_class_id(const Node& node)   { visit_generic(node); }
    virtual void visit_var_decl(const Node& node)   { visit_generic(node); }
    virtual void visit_func_decl(const Node& node)  { visit_generic(node); }
    virtual void visit_func_def(const Node& node)   { visit_generic(node); }
    virtual void visit_block(const Node& node)      { visit_generic(node); }
    virtual void visit_block_skip(const Node& node) { visit_generic(node); }
};

/**
 * @brief Concrete Visitor to print the AST structure and a summary.
 */
template <typename Node>
class AstPrinterVisitor : public AstVisitor<Node> {
    using Base = AstVisitor<Node>;
    int indent_level = 0;

    void print_node(const Node& node, const std::string& type_name) {
        std::string space(indent_level * 2, ' ');
        std::cout << space << "-> " << type_name
                  << " [" << node->source() << "]\n";
    }

public:
    void traverse_children(const Node& node) {
        indent_level++;
        for (const auto& child : node->children()) {
            this->visit(child); // Use this-> for base class call
        }
        indent_level--;
    }

protected:
    void visit_top_level(const Node& node) override {
        print_node(node, "TOP_LEVEL");
        traverse_children(node);
    }

    void visit_class_def(const Node& node) override {
        // Children: [0] = 'class', [1] = IDENTIFIER (Class Name), [2] = BLOCK
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> CLASS_DEF: " << node->children()[0]->source() << "\n";
        traverse_children(node);
    }

    void visit_class_id(const Node& node) override {
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> CLASS_ID: " << node->source() << "\n";
        traverse_children(node);
    }

    void visit_var_decl(const Node& node) override {
        // Children: [0] = IDENTIFIER (Type), [1] = IDENTIFIER (Name), [2] = SEMICOLON
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> VAR_DECL: " << node->source() << "\n";
    }

    void visit_func_decl(const Node& node) override {
        // Children: [0] = IDENTIFIER (Return Type), [1] = IDENTIFIER (Name), ...
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> FUNC_DECL: " << node->source() << "\n";
    }

    void visit_func_def(const Node& node) override {
        // Children: [0] = IDENTIFIER (Return Type), [1] = IDENTIFIER (Name), ..., [4] = BLOCK_SKIP
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> FUNC_DEF: " << node->source() << "'\n";
        traverse_children(node); // Traverses the BLOCK_SKIP child
    }

    void visit_block(const Node& node) override {
        print_node(node, "BLOCK");
        traverse_children(node);
    }

    void visit_block_skip(const Node& node) override {
        print_node(node, "BLOCK_SKIP (Skipped)");
        // No traverse_children here, as the contents were skipped by the parser.
    }

    void visit_generic(const Node& node) override {
        // This is primarily for skipping non-structural elements like terminal tokens
        // which often don't need dedicated handling in a high-level AST visitor.
        // For example, in the ClassDef, we skip the 'class' token and the braces.
        // We only traverse children for non-terminal nodes that are structural.

        // If a node is a structural node but has no specific handler,
        // we can still traverse its children.
        if (node->children().size() > 0) {
            traverse_children(node);
        }
    }
};


// ----------------------------------------------------------------------

using parser_type = parser<source_type, cpp_lexer_grammar, cpp_parser_grammar>;

// ... (test_cpp_lexer is here) ...

void test_cpp_parser() {
    using namespace parserlib;

    auto result = parser_type::parse(src);

    if (!result.ast_nodes.empty()) // result.success &&
    {
        std::cout << "Parsing succeeded!\n";
        std::cout << "\n=== AST Printing (Original Recursive Function) ===\n";
        for (auto &node : result.ast_nodes)
            cpp_parser_grammar::print_ast(node);

        // -------------------------------------------------------------------
        // New: AST Visitor Test
        // -------------------------------------------------------------------
        std::cout << "\n=== AST Traversal (Visitor Pattern) ===\n";
        using AstNodePtr = decltype(result.ast_nodes)::value_type;
        AstPrinterVisitor<AstNodePtr> visitor;

        for (auto &node : result.ast_nodes)
            visitor.visit(node);

    }

    if (!result.success)
    {
        // ... (Error handling code remains the same) ...
        std::cout << "Parsing failed.\n";

        // --- Debug: locate token near parse_position ---
        auto parsed_len = std::distance(src.begin(), result.lexer.parse_position);
        std::cout << "Parsing stopped at offset " << parsed_len << "\n";

        auto &tokens = result.lexer.parsed_tokens;
        bool matched = false;

        for (size_t i = 0; i < tokens.size(); ++i) {
            auto &t = tokens[i];
            auto token_start = std::distance(src.begin(), t.begin());
            auto token_end = std::distance(src.begin(), t.end());

            if (parsed_len >= token_start && parsed_len < token_end) {
                std::cout << "Stopped *inside* token #" << i
                          << " id=" << static_cast<int>(t.id())
                          << " text='" << std::string(t.begin(), t.end()) << "'\n";
                matched = true;
                break;
            }
            // Add: allow stop *exactly at the end* of a token
            if (parsed_len == token_end) {
                std::cout << "Stopped *right after* token #" << i
                          << " id=" << static_cast<int>(t.id())
                          << " text='" << std::string(t.begin(), t.end()) << "'\n";
                matched = true;
                break;
            }
            if (parsed_len < token_start) {
                std::cout << "Stopped *before* token #" << i
                          << " id=" << static_cast<int>(t.id())
                          << " text='" << std::string(t.begin(), t.end()) << "'\n";
                matched = true;
                break;
            }
        }

        if (!matched && !tokens.empty()) {
            std::cout << "Stopped after the last token #" << tokens.size() - 1
                      << " id=" << static_cast<int>(tokens.back().id())
                      << " text='" << std::string(tokens.back().begin(), tokens.back().end()) << "'\n";
        }


        if (parsed_len < static_cast<int>(src.size())) {
            std::cout << "Remaining text:\n"
                      << std::string(result.lexer.parse_position, src.end()) << "\n";
        }

        // --- Lexer errors ---
        if (!result.lexer.errors.empty()) {
            for (auto &e : result.lexer.errors) {
                auto offset = std::distance(src.begin(), e.begin());
                std::cout << "Lexer error at offset "
                          << offset
                          << " (error id: " << static_cast<int>(e.id()) << ")\n";
            }
        }

        // --- Parser errors ---
        if (!result.errors.empty()) {
            for (auto &e : result.errors) {
                auto offset = std::distance(src.begin(), e.begin());
                std::cout << "Parser error at offset "
                          << offset
                          << " (error id: " << static_cast<int>(e.id()) << ")\n";
            }
        }
    }
}


int main() {
    test_cpp_lexer();
    std::cout << "\n----------------------------------------\n";
    test_cpp_parser();
    return 0;
}
