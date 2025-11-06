#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <cassert>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

#include "parserlib.hpp"

using namespace parserlib;

#include "cpp_lexer_grammar.h"
#include "cpp_parser_grammar.h"
#include "ast_visitor.h"

// --- Source type ---
using source_type = line_counting_string<>;

source_type src = R"(
// class templates1
// class templates2
// class templates3

xyz abc;
opq def;

string a;


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

        // For any error in the error container
        for (const auto& error : result.errors) {
            auto line = error.begin().line();
            auto column = error.begin().column();
            std::cout << "Error at line " << line << ", column " << column << std::endl;
        }

    }

    std::cout << "Lexer stopped at offset "
              << std::distance(src.begin(), result.parse_position) << "\n";
}


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


bool test_cpp_source(
    const std::string& source_str,
    cpp_parser_grammar::match_id_type expected_match_id,
    bool should_parse_completely = true
) {
    // Convert to line_counting_string (non-const)
    source_type source(source_str.c_str());

    // Lex the source
    auto lexer_result = lexer<line_counting_string<>, cpp_lexer_grammar>::parse(source);

    // Create parse context with tokens
    parse_context<
        typename lexer<line_counting_string<>, cpp_lexer_grammar>::parsed_token_container_type,
        cpp_parser_grammar::match_id_type,
        cpp_parser_grammar::error_id_type
    > pc(lexer_result.parsed_tokens);

    // Parse
    cpp_parser_grammar grammar;
    const auto result = grammar.parse(pc);

    // Print all matches for debugging
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "  Parsed " << pc.matches().size() << " matches:\n";
    for (const auto& match : pc.matches()) {
        std::cout << "    - " << cpp_parser_grammar::match_id_to_string(match.id());

        // Print the tokens in this match
        std::cout << " [tokens: ";
        auto tokens = match.source();
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << cpp_lexer_grammar::match_id_to_string(tokens[i].id());
        }
        std::cout << "]\n";
    }

    // Check basic success
    if (!result) {
        std::cout << "  Parse failed (result = false)\n";
        return false;
    }
    if (should_parse_completely && !pc.is_end_parse_position()) {
        std::cout << "  Parse incomplete (not at end position)\n";
        return false;
    }

    // Check for expected match ID
    for (const auto& match : pc.matches()) {
        if (match.id() == expected_match_id) {
            return true;
        }
    }

    std::cout << "  Expected match ID not found: "
              << cpp_parser_grammar::match_id_to_string(expected_match_id) << "\n";
    return false;
}

struct TestCase {
    std::string source;
    cpp_parser_grammar::match_id_type expected_id;
    std::string description = "";
};

void run_test_cases(const std::vector<TestCase>& test_cases) {
    for (const auto& tc : test_cases) {
        if (!test_cpp_source(tc.source, tc.expected_id)) {
            std::cout << "Test Failed: " << tc.description << "\n";
            std::cout << "  Source: " << tc.source << "\n";
        } else {
            std::cout << "Test Passed: " << tc.description << "\n";
        }
    }
}

void test_declarations() {
    std::vector<TestCase> cases = {
        {"sohu x;", cpp_parser_grammar::match_id_type::VAR_DECL, "simple var decl"},
        {"int x = 5;", cpp_parser_grammar::match_id_type::VAR_DECL},
        {"void foo();", cpp_parser_grammar::match_id_type::FUNC_DECL},
        {"class Foo { };", cpp_parser_grammar::match_id_type::CLASS_DEF, "empty class"}
        // Remove the struct test case or use CLASS_DEF for it
    };
    run_test_cases(cases);
}

extern void test_cpp_parser_with_comments();

int main() {
//    test_cpp_lexer();
//    std::cout << "\n----------------------------------------\n";
//    test_cpp_parser();
//    std::cout << "\n----------------------------------------\n";
//    test_declarations();
//    std::cout << "\n----------------------------------------\n";
    test_cpp_parser_with_comments();
    return 0;
}
