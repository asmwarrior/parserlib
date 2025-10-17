#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cassert>
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
        RIGHT_PAREN
    };

    enum class error_id_type {
        INVALID_TOKEN
    };

    template <class ParseContext>
    auto parse(ParseContext& pc) const noexcept {
        const auto whitespace = terminal(' ');
        const auto digit = range('0', '9');
        const auto letter = range('a', 'z') | range('A', 'Z') | terminal('_');

        const auto number = (+digit >> -('.' >> +digit))->*match_id_type::NUMBER;
        const auto identifier = (+letter >> *(letter | digit))->*match_id_type::IDENTIFIER;
        const auto plus = terminal('+')->*match_id_type::PLUS;
        const auto minus = terminal('-')->*match_id_type::MINUS;
        const auto mul = terminal('*')->*match_id_type::MUL;
        const auto div = terminal('/')->*match_id_type::DIV;
        const auto left_paren = terminal('(')->*match_id_type::LEFT_PAREN;
        const auto right_paren = terminal(')')->*match_id_type::RIGHT_PAREN;

        const auto token = number | identifier | plus | minus | mul | div | left_paren | right_paren;
        const auto token_error = error(error_id_type::INVALID_TOKEN, skip_until(whitespace | token));
        const auto token1 = token | token_error;

        const auto grammar = *(whitespace | token1);
        return grammar.parse(pc);
    }
};

using lexer_type = lexer<source_type, cpp_lexer_grammar>;

// --- Parser grammar ---
class cpp_parser_grammar {
public:
    using lexer_grammar = cpp_lexer_grammar;

    enum match_id_type {
        NUM,
        ID,
        ADD,
        SUB,
        MUL,
        DIV
    };

    enum error_id_type {
        INVALID_TOKEN,
        INVALID_UNARY_EXPR,
        INCOMPLETE_PARSE
    };

    static error_id_type translate_lexer_error_id(lexer_type::error_id_type error) {
        switch (error) {
            case lexer_type::error_id_type::INVALID_TOKEN:
                return error_id_type::INVALID_TOKEN;
            default:
                break;
        }
        throw std::invalid_argument("invalid lexer error id");
    }

    template <class ParseContext>
    parse_result parse(ParseContext& pc) const noexcept {
        return instance<ParseContext>().parse(pc);
    }

private:
    template <class ParseContext>
    class instance {
    public:
        instance() {
            const auto val
                = terminal(lexer_grammar::match_id_type::NUMBER)->*match_id_type::NUM
                | terminal(lexer_grammar::match_id_type::IDENTIFIER)->*match_id_type::ID;

            mul = (mul >> terminal(lexer_grammar::match_id_type::MUL) >> val)->*match_id_type::MUL
                | (mul >> terminal(lexer_grammar::match_id_type::DIV) >> val)->*match_id_type::DIV
                | val;

            add = (add >> terminal(lexer_grammar::match_id_type::PLUS) >> mul)->*match_id_type::ADD
                | (add >> terminal(lexer_grammar::match_id_type::MINUS) >> mul)->*match_id_type::SUB
                | mul;
        }

        parse_result parse(ParseContext& pc) noexcept {
            return add.parse(pc);
        }

    private:
        rule<ParseContext> mul;
        rule<ParseContext> add;
    };

public:
    // Evaluate the AST
    static double eval(const ast_node_ptr_type<match_id_type, lexer_type::iterator_type>& node) {
        switch (node->id()) {
            case NUM: {
                std::stringstream ss;
                ss << node->source();
                double v; ss >> v;
                return v;
            }
            case ADD:
                return eval(node->children()[0]) + eval(node->children()[1]);
            case SUB:
                return eval(node->children()[0]) - eval(node->children()[1]);
            case MUL:
                return eval(node->children()[0]) * eval(node->children()[1]);
            case DIV:
                return eval(node->children()[0]) / eval(node->children()[1]);
            case ID:
                throw std::runtime_error("Identifiers evaluation not implemented");
        }
        throw std::invalid_argument("invalid ast node id");
    }
};

using parser_type = parser<source_type, cpp_lexer_grammar, cpp_parser_grammar>;

// --- Test functions ---
static void test_tokenization() {
    source_type src = "a + 123 * b";
    auto result = lexer_type::parse(src);
    assert(result.errors.size() == 0);
}

static void test_parsing() {
    source_type src = "1+2*3";
    auto result = parser_type::parse(src);
    assert(result.success);
    assert(result.ast_nodes.size() == 1);
    assert(cpp_parser_grammar::eval(result.ast_nodes[0]) == 1.0 + 2.0 * 3.0);
}

void test_cpp_parser() {
    test_tokenization();
    test_parsing();
    std::cout << "All tests passed.\n";
}

int main() {
    test_cpp_parser();
}
