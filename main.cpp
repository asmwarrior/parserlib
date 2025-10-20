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

source_type src = R"(class MyClass {
        int a;
        float b;
        void foo();
    }

    int x;
    void bar() { })";


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

    std::cout << "Lexer stopped at offset "
              << std::distance(src.begin(), result.parse_position) << "\n";
}


// --- Parser grammar ---
class cpp_parser_grammar {
public:
    using lexer_grammar = cpp_lexer_grammar;

    enum class match_id_type {
        VAR_DECL,
        FUNC_DEF,
        BLOCK,
        CLASS_DEF,
        TOP_LEVEL
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

            // --- Variable declaration: <type> <id> ';'
            var_decl = (terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::SEMICOLON))
                        ->*match_id_type::VAR_DECL;

            // --- Block: '{' { var_decl | func_def } '}'
            block = (terminal(id_type::LEFT_BRACE)
                     >> *(var_decl | func_def)
                     >> terminal(id_type::RIGHT_BRACE))
                     ->*match_id_type::BLOCK;

            // --- Function definition: <type> <id> '(' ')' block
            func_def = (terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::LEFT_PAREN)
                        >> terminal(id_type::RIGHT_PAREN)
                        >> block
                        >> -terminal(id_type::SEMICOLON))  // optional semicolon))
                        ->*match_id_type::FUNC_DEF;

            // --- Class definition: 'class' IDENTIFIER block
            class_def = (terminal(id_type::CLASS)
                         >> terminal(id_type::IDENTIFIER)
                         >> block
                         >> -terminal(id_type::SEMICOLON))  // optional semicolon)
                         ->*match_id_type::CLASS_DEF;

            // --- Top-level declarations: { class_def | func_def | var_decl }+
            top_level = (+(
                            class_def
                            | func_def
                            | var_decl
                         ))
                         ->*match_id_type::TOP_LEVEL;
        }

        parse_result parse(ParseContext& pc) noexcept {
            auto r = top_level.parse(pc);
            return r;
        }

    private:
        rule<ParseContext> var_decl, func_def, block, class_def, top_level;
    };
};




using parser_type = parser<source_type, cpp_lexer_grammar, cpp_parser_grammar>;


void test_cpp_parser() {
    using namespace parserlib;

    auto result = parser_type::parse(src);

    if (result.success && !result.ast_nodes.empty()) {
        std::cout << "Parsing succeeded!\n";
        for (auto &node : result.ast_nodes)
            cpp_parser_grammar::print_ast(node);
    } else {
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
    test_cpp_parser();
    return 0;
}
