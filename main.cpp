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

// --- Lexer grammar ---
class cpp_lexer_grammar {
public:
    // --- Token IDs ---
    enum class match_id_type {
        NUMBER,
        IDENTIFIER,
        PLUS,
        MINUS,
        MUL,
        DIV,
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        SEMICOLON,
        COMMA,
        IF,
        ELSE,
        WHILE,
        FOR,
        RETURN,
        FUNC,
        CLASS
    };

    // --- Error IDs ---
    enum class error_id_type {
        INVALID_TOKEN
    };

    // --- Keyword map (string -> match_id_type) ---
    static const std::unordered_map<std::string, match_id_type>& keyword_map() {
        static const std::unordered_map<std::string, match_id_type> map{
            {"if",     match_id_type::IF},
            {"else",   match_id_type::ELSE},
            {"while",  match_id_type::WHILE},
            {"for",    match_id_type::FOR},
            {"return", match_id_type::RETURN},
            {"func",   match_id_type::FUNC},
            {"class",  match_id_type::CLASS}
        };
        return map;
    }

    template <class ParseContext>
    auto parse(ParseContext& pc) const noexcept {
        const auto whitespace = terminal(' ') | terminal('\t') | terminal('\n') | terminal('\r');

        const auto digit      = range('0', '9');
        const auto letter     = range('a', 'z') | range('A', 'Z') | terminal('_');

        // Number token
        const auto number = (+digit >> -('.' >> +digit))->*match_id_type::NUMBER;

        // Identifier token
        const auto identifier = (+letter >> *(letter | digit))->*match_id_type::IDENTIFIER;

        // Symbols
        const auto plus  = terminal('+')->*match_id_type::PLUS;
        const auto minus = terminal('-')->*match_id_type::MINUS;
        const auto mul   = terminal('*')->*match_id_type::MUL;
        const auto div   = terminal('/')->*match_id_type::DIV;
        const auto lparen = terminal('(')->*match_id_type::LEFT_PAREN;
        const auto rparen = terminal(')')->*match_id_type::RIGHT_PAREN;
        const auto lbrace = terminal('{')->*match_id_type::LEFT_BRACE;
        const auto rbrace = terminal('}')->*match_id_type::RIGHT_BRACE;
        const auto semicolon = terminal(';')->*match_id_type::SEMICOLON;
        const auto comma     = terminal(',')->*match_id_type::COMMA;

        // Token: number, identifier, symbols
        const auto token = number | identifier
                         | plus | minus | mul | div
                         | lparen | rparen | lbrace | rbrace | semicolon | comma;

//        // Token error
//        const auto token_error = error(error_id_type::INVALID_TOKEN, skip_until(whitespace | token));
//        const auto token1 = token | token_error;
//
//        // Grammar: zero or more tokens or whitespace
//        const auto grammar = *(whitespace | token1);

        // Grammar: zero or more tokens or whitespace
        const auto grammar = *(+whitespace | token);

        // Parse
        auto result = grammar.parse(pc);

        // --- Postprocess: convert identifiers to keywords ---

        for (auto& m : pc.matches()) {
            if (m.id() == match_id_type::IDENTIFIER) {
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
    source_type src = R"(
        class MyClass {
            func foo(a, b) {
                return a + b;
            }
            func bar() {
                if (1) { return 2; } else { return 3; }
            }
        }
    )";

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
                  << static_cast<int>(token.id())
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

    std::cout << "Parse stopped at offset "
              << std::distance(src.begin(), result.parse_position) << "\n";
}


// --- Parser grammar ---
class cpp_parser_grammar {
public:
    using lexer_grammar = cpp_lexer_grammar;

    enum class match_id_type {
        VAR_DECL,
        BLOCK,
        CLASS_DEF,
    };

    enum class error_id_type {
        INVALID_TOKEN,
        INCOMPLETE_PARSE
    };

    template <class ParseContext>
    parse_result parse(ParseContext& pc) const noexcept {
        return instance<ParseContext>().parse(pc);
    }

    // --- AST printer ---
    template <typename Node>
    static void print_ast(const Node& node, int indent = 0) {
        std::string space(indent, ' ');
        std::cout << space << static_cast<int>(node->id())
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

            // --- Variable declaration:  <type> <id> ';'
            var_decl = (terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::SEMICOLON))
                        ->*match_id_type::VAR_DECL;

            // --- Block = '{' { var_decl } '}'
            block = (terminal(id_type::LEFT_BRACE)
                     >> *var_decl
                     >> terminal(id_type::RIGHT_BRACE))
                     ->*match_id_type::BLOCK;

            // --- Class definition: 'class' IDENTIFIER block
            class_def = (terminal(id_type::CLASS)
                         >> terminal(id_type::IDENTIFIER)
                         >> block)
                         ->*match_id_type::CLASS_DEF;
        }

        parse_result parse(ParseContext& pc) noexcept {
            return class_def.parse(pc);
        }

    private:
        rule<ParseContext> var_decl, block, class_def;
    };
};




using parser_type = parser<source_type, cpp_lexer_grammar, cpp_parser_grammar>;


void test_cpp_parser() {
    using namespace parserlib;

    source_type src = R"(
        class MyClass {
            int a;
            float b;
        }
    )";

    auto result = parser_type::parse(src);

    if (result.success && !result.ast_nodes.empty()) {
        std::cout << "Parsing succeeded!\n";
        for (auto &node : result.ast_nodes)
            cpp_parser_grammar::print_ast(node);
    } else {
        std::cout << "Parsing failed.\n";

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
    test_cpp_parser();
    return 0;
}
