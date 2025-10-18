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

    enum class error_id_type {
        INVALID_TOKEN
    };

    template <class ParseContext>
    auto parse(ParseContext& pc) const noexcept {
        const auto whitespace = +(
            terminal(' ') |
            terminal('\t') |
            terminal('\r') |
            terminal('\n')
        );

        const auto digit = range('0', '9');
        const auto letter = range('a', 'z') | range('A', 'Z') | terminal('_');

        const auto number = (+digit >> -('.' >> +digit))->*match_id_type::NUMBER;
        const auto identifier = (+letter >> *(letter | digit))->*match_id_type::IDENTIFIER;

        const auto plus  = terminal('+')->*match_id_type::PLUS;
        const auto minus = terminal('-')->*match_id_type::MINUS;
        const auto mul   = terminal('*')->*match_id_type::MUL;
        const auto div   = terminal('/')->*match_id_type::DIV;

        const auto lparen = terminal('(')->*match_id_type::LEFT_PAREN;
        const auto rparen = terminal(')')->*match_id_type::RIGHT_PAREN;
        const auto lbrace = terminal('{')->*match_id_type::LEFT_BRACE;
        const auto rbrace = terminal('}')->*match_id_type::RIGHT_BRACE;
        const auto semicolon = terminal(';')->*match_id_type::SEMICOLON;
        const auto comma = terminal(',')->*match_id_type::COMMA;

        const auto kw_if     = terminal("if")->*match_id_type::IF;
        const auto kw_else   = terminal("else")->*match_id_type::ELSE;
        const auto kw_while  = terminal("while")->*match_id_type::WHILE;
        const auto kw_for    = terminal("for")->*match_id_type::FOR;
        const auto kw_return = terminal("return")->*match_id_type::RETURN;
        const auto kw_func   = terminal("func")->*match_id_type::FUNC;
        const auto kw_class  = terminal("class")->*match_id_type::CLASS;

        const auto token = number | identifier | plus | minus | mul | div
            | lparen | rparen | lbrace | rbrace | semicolon | comma
            | kw_if | kw_else | kw_while | kw_for | kw_return | kw_func | kw_class;

        const auto token_error = error(error_id_type::INVALID_TOKEN, skip_until(whitespace | token));
        const auto token1 = token | token_error;

        const auto grammar = *(whitespace | token1);
        return grammar.parse(pc);
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
        NUM,
        ID,
        ADD,
        SUB,
        MUL,
        DIV,
        BLOCK,
        STMT,
        IF_STMT,
        WHILE_STMT,
        FOR_STMT,
        FUNC_DEF,
        CLASS_DEF,
        RETURN_STMT
    };

    enum class error_id_type {
        INVALID_TOKEN,
        INVALID_UNARY_EXPR,
        INCOMPLETE_PARSE
    };

    template <class ParseContext>
    parse_result parse(ParseContext& pc) const noexcept {
        return instance<ParseContext>().parse(pc);
    }

private:
    template <class ParseContext>
    class instance {
    public:
        instance() {
            // --- Expression ---
            const auto val = terminal(lexer_grammar::match_id_type::NUMBER)->*match_id_type::NUM
                           | terminal(lexer_grammar::match_id_type::IDENTIFIER)->*match_id_type::ID;

            mul = (mul >> terminal(lexer_grammar::match_id_type::MUL) >> val)->*match_id_type::MUL
                | (mul >> terminal(lexer_grammar::match_id_type::DIV) >> val)->*match_id_type::DIV
                | val;

            add = (add >> terminal(lexer_grammar::match_id_type::PLUS) >> mul)->*match_id_type::ADD
                | (add >> terminal(lexer_grammar::match_id_type::MINUS) >> mul)->*match_id_type::SUB
                | mul;

            expr = rule_ref_parse_node(add);

            // --- Simple statements ---
            stmt = (expr >> terminal(lexer_grammar::match_id_type::SEMICOLON))->*match_id_type::STMT;

            // --- Return ---
            auto ret_stmt = terminal(lexer_grammar::match_id_type::RETURN) >> expr >> terminal(lexer_grammar::match_id_type::SEMICOLON);
            stmt = stmt | ret_stmt->*match_id_type::RETURN_STMT;

            // --- If / While / For ---
            auto if_stmt = terminal(lexer_grammar::match_id_type::IF) >> terminal(lexer_grammar::match_id_type::LEFT_PAREN)
                           >> expr >> terminal(lexer_grammar::match_id_type::RIGHT_PAREN) >> block;
            stmt = stmt | if_stmt->*match_id_type::IF_STMT;

            auto while_stmt = terminal(lexer_grammar::match_id_type::WHILE) >> terminal(lexer_grammar::match_id_type::LEFT_PAREN)
                              >> expr >> terminal(lexer_grammar::match_id_type::RIGHT_PAREN) >> block;
            stmt = stmt | while_stmt->*match_id_type::WHILE_STMT;

            auto for_stmt = terminal(lexer_grammar::match_id_type::FOR) >> terminal(lexer_grammar::match_id_type::LEFT_PAREN)
                            >> -(expr >> terminal(lexer_grammar::match_id_type::SEMICOLON))
                            >> -(expr >> terminal(lexer_grammar::match_id_type::SEMICOLON))
                            >> -expr >> terminal(lexer_grammar::match_id_type::RIGHT_PAREN) >> block;
            stmt = stmt | for_stmt->*match_id_type::FOR_STMT;

            // --- Block ---
            block = terminal(lexer_grammar::match_id_type::LEFT_BRACE)
                  >> *stmt
                  >> terminal(lexer_grammar::match_id_type::RIGHT_BRACE)
                  ->*match_id_type::BLOCK;

            // --- Function ---
            auto func = terminal(lexer_grammar::match_id_type::FUNC)
                >> terminal(lexer_grammar::match_id_type::IDENTIFIER)
                >> terminal(lexer_grammar::match_id_type::LEFT_PAREN)
                >> *(terminal(lexer_grammar::match_id_type::IDENTIFIER)
                     >> -(terminal(lexer_grammar::match_id_type::COMMA)
                          >> terminal(lexer_grammar::match_id_type::IDENTIFIER)))
                >> terminal(lexer_grammar::match_id_type::RIGHT_PAREN)
                >> block
                ->*match_id_type::FUNC_DEF;
            stmt = stmt | func;

            // --- Class ---
            auto class_dec = terminal(lexer_grammar::match_id_type::CLASS)
                >> terminal(lexer_grammar::match_id_type::IDENTIFIER)
                >> block
                ->*match_id_type::CLASS_DEF;
            stmt = stmt | class_dec;
        }

        parse_result parse(ParseContext& pc) noexcept {
            return block.parse(pc);
        }

    private:
        rule<ParseContext> expr, add, mul, stmt, block;
    };

public:
    // --- AST pretty printer ---
    template <typename Node>
    static void print_ast(const Node& node, int indent = 0) {
        static const std::unordered_map<int, std::string> id_names = {
            { (int)match_id_type::NUM, "NUM" },
            { (int)match_id_type::ID, "ID" },
            { (int)match_id_type::ADD, "ADD" },
            { (int)match_id_type::SUB, "SUB" },
            { (int)match_id_type::MUL, "MUL" },
            { (int)match_id_type::DIV, "DIV" },
            { (int)match_id_type::BLOCK, "BLOCK" },
            { (int)match_id_type::STMT, "STMT" },
            { (int)match_id_type::IF_STMT, "IF_STMT" },
            { (int)match_id_type::WHILE_STMT, "WHILE_STMT" },
            { (int)match_id_type::FOR_STMT, "FOR_STMT" },
            { (int)match_id_type::FUNC_DEF, "FUNC_DEF" },
            { (int)match_id_type::CLASS_DEF, "CLASS_DEF" },
            { (int)match_id_type::RETURN_STMT, "RETURN_STMT" },
        };

        std::string space(indent, ' ');

        auto id = static_cast<int>(node->id());
        auto it = id_names.find(id);
        std::string id_str = (it != id_names.end()) ? it->second : ("ID#" + std::to_string(id));

        // Get token text, truncated if long
        std::string src(node->source());
        if (src.size() > 30) src = src.substr(0, 30) + "...";

        std::cout << space << id_str << "  \"" << src << "\"\n";

        for (auto& c : node->children())
            print_ast(c, indent + 2);
    }

};

using parser_type = parser<source_type, cpp_lexer_grammar, cpp_parser_grammar>;

//// --- Test ---
//void test_cpp_parser() {
//    source_type src = R"(
//        int a;
//        float b;
//    )";
//
//    auto result = parser_type::parse(src);
//
//    if (!result.success)
//    {
//        std::cerr << "Parse failed!\n";
//        if (result.error_position != src.end())
//        {
//            auto pos = result.error_position - src.begin();
//            std::cerr << "Stopped near position: " << pos << "\n";
//            std::cerr << "Remaining text: \"" << std::string(src.begin() + pos, src.begin() + std::min(pos + 40, (size_t)src.size())) << "\"\n";
//        }
//    }
//
//
//    if(result.success && !result.ast_nodes.empty()) {
//        for(auto& node : result.ast_nodes)
//            cpp_parser_grammar::print_ast(node);
//    }
//}


void test_cpp_parser() {
    using namespace parserlib;

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
