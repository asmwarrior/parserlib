
#include <iostream>
#include <string>

#include "parserlib.hpp"
#include "cpp_lexer_grammar.h"
#include "cpp_parser_grammar.h"

using namespace parserlib;

void test_cpp_lexer() {
    const std::string src = R"(
        #include <iostream>

        int main() {
            double value = 10.5;
            if (value >= 10.0) {
                value++;
            } else {
                return 0;
            }
            return value;
        }
    )";

    cpp_lexer_grammar::parse_context_type pc{src.begin(), src.end()};
    bool success = cpp_lexer_grammar::parse(pc);

    std::cout << "=== Lexer test ===\n";
    std::cout << "Success: " << std::boolalpha << success << "\n";

for (const auto& token : pc.get_matches())
{
    // Extract underlying iterators from parse_iterator wrappers
    auto begin_iter = token.begin().get_iterator();
    auto end_iter = token.end().get_iterator();
    std::string text(begin_iter, end_iter);

    auto pos = token.begin().get_text_position();

    std::cout
        << cpp_lexer_grammar::match_id_to_string(token.get_id())
        << " \"" << text << "\""
        << " @ " << pos.get_line()
        << ":" << pos.get_column()
        << "\n";
}

for (const auto& err : pc.get_errors()) {
    auto pos = err.begin().get_text_position();

    // Extract underlying iterators from parse_iterator wrappers
    auto begin_iter = err.begin().get_iterator();
    auto end_iter = err.end().get_iterator();
    std::string text(begin_iter, end_iter);

    std::cout
        << "  Error at "
        << pos.get_line() << ":" << pos.get_column()
        << " id=" << static_cast<int>(err.get_id())
        << " near '" << text << "'\n";
}

}

void test_cpp_parser() {
    const std::string src = R"(
        class AAA
        {
            abc x;
            uvw y;
        };

        abc x;
        xyz y;
    )";

    // --- Lex ---
    cpp_lexer_grammar::parse_context_type lex_pc{src.begin(), src.end()};
    if (!cpp_lexer_grammar::parse(lex_pc)) {
        std::cout << "Lexing failed\n";
        return;
    }

    // --- Parse ---
    cpp_parser_grammar::parse_context_type parse_pc{
        lex_pc.get_matches().begin(),
        lex_pc.get_matches().end()
    };

    cpp_parser_grammar grammar;
    bool ok = grammar.parse(parse_pc);

    std::cout << "Parser success: " << std::boolalpha << ok << "\n";

    for (const auto& m : parse_pc.get_matches()) {
        std::cout << "match: "
                  << cpp_parser_grammar::match_id_to_string(m.get_id())
                  << "\n";
    }
//    // use another method to print the AST
//    auto ast = make_ast_node(parse_pc.get_matches()[0]);
//    cpp_parser_grammar::print_ast(ast);

    std::cout << "\n";

    // Recurse to children
    for (const auto& child : parse_pc.get_matches()) {
        cpp_parser_grammar::print_match(child, 0);
    }


    if (!parse_pc.get_errors().empty()) {
        std::cout << "Parser errors:\n";
        for (const auto& e : parse_pc.get_errors()) {
            std::cout << "  error id="
                      << static_cast<int>(e.get_id()) << "\n";
        }
    }
}

bool test_cpp_source(
    const std::string& source_str,
    cpp_parser_grammar::match_id_type expected_match_id,
    bool should_parse_completely = true
) {
    // --- Lex ---
    cpp_lexer_grammar::parse_context_type lex_pc{
        source_str.begin(),
        source_str.end()
    };

    if (!cpp_lexer_grammar::parse(lex_pc)) {
        std::cout << "  Lexing failed\n";
        return false;
    }

    // --- Parse ---
    cpp_parser_grammar::parse_context_type parse_pc{
        lex_pc.get_matches().begin(),
        lex_pc.get_matches().end()
    };

    cpp_parser_grammar grammar;
    const bool result = grammar.parse(parse_pc);

    // Debug output
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "  Parsed " << parse_pc.get_matches().size() << " matches:\n";

    for (const auto& match : parse_pc.get_matches()) {
        std::cout << "    - "
                  << cpp_parser_grammar::match_id_to_string(match.get_id())
                  << "\n";
    }

    // Basic checks
    if (!result) {
        std::cout << "  Parse failed (result = false)\n";
        return false;
    }

    if (should_parse_completely && !parse_pc.is_end_parse_position()) {
        std::cout << "  Parse incomplete (not at end position)\n";
        return false;
    }

    // Look for expected match
    for (const auto& match : parse_pc.get_matches()) {
        if (match.get_id() == expected_match_id) {
            return true;
        }
    }

    std::cout << "  Expected match ID not found: "
              << cpp_parser_grammar::match_id_to_string(expected_match_id)
              << "\n";

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


int main() {
    test_cpp_lexer();
    test_cpp_parser();
    test_declarations();
    return 0;
}


#if 0































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
using source_type = std::string;

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



//using lexer_type = lexer<source_type, cpp_lexer_grammar>;


//void test_cpp_lexer() {
//
//    //using lexer_t = lexer_type;
//
//    cpp_lexer_grammar::parse_context_type pc;
//
//    // --- run lexer only ---
//    auto result = cpp_lexer_grammar::parse(pc);
//
//    std::cout << "=== Lexer test ===\n";
//    std::cout << "Success: " << std::boolalpha << result << "\n";
//
//    // --- show tokens ---
//    size_t index = 0;
//    for (const auto& token : result.parsed_tokens) {
//        std::string text(token.begin(), token.end());
//        std::cout << "[" << index++ << "] id="
//                  << cpp_lexer_grammar::match_id_to_string(token.id()) // <-- Using the string name
//                  << " text='" << text << "'\n";
//    }
//
//    // --- show errors ---
//    if (!result.errors.empty()) {
//        std::cout << "\nLexer errors:\n";
//        for (const auto& err : result.errors) {
//            auto offset = std::distance(src.begin(), err.begin());
//            std::cout << "  Error at offset " << offset
//                      << " (error id: " << static_cast<int>(err.id()) << ")\n";
//        }
//
//        // For any error in the error container
//        for (const auto& error : result.errors) {
//            auto line = error.begin().line();
//            auto column = error.begin().column();
//            std::cout << "Error at line " << line << ", column " << column << std::endl;
//        }
//
//    }
//
//    std::cout << "Lexer stopped at offset "
//              << std::distance(src.begin(), result.parse_position) << "\n";
//}



void test_cpp_lexer() {
    // Define a source string for testing the lexer
    // This was missing and needed for `std::distance(src.begin(), ...)` to work.
    const std::string src =
        R"(
            // Example C++ Code
            #include <iostream>

            int main() {
                double value = 10.5;
                if (value >= 10.0) {
                    value++;
                } else {
                    return 0;
                }
                return value;
            }
        )";

    // 1. Initialize the parse context with the source code
    cpp_lexer_grammar::parse_context_type pc{src.begin(), src.end()};

    // 2. Run the lexer
    // The function returns a bool indicating overall success, but results are in pc
    bool success = cpp_lexer_grammar::parse(pc);

    std::cout << "=== Lexer test ===\n";
    std::cout << "Success: " << std::boolalpha << success << "\n";

    // 3. Show tokens - CORRECTED to use pc.matches()
    size_t index = 0;
    // pc.matches() returns a container of the parsed tokens
    for (const auto& token : pc.matches()) {
        // token.source() gives the iterator range for the token text
        std::string text(token.source().begin(), token.source().end());
        std::cout << "[" << index++ << "] id="
                  << cpp_lexer_grammar::match_id_to_string(token.id()) // <-- Using the string name
                  << " text='" << text << "'\n";
    }

    // 4. Show errors - CORRECTED to use pc.errors()
    if (!pc.errors().empty()) {
        std::cout << "\nLexer errors:\n";
        for (const auto& err : pc.errors()) {
            auto offset = std::distance(src.begin(), err.begin());
            std::cout << "  Error at offset " << offset
                      << " (error id: " << static_cast<int>(err.id()) << ")\n";
        }

//        // For any error in the error container
//        for (const auto& error : pc.errors()) {
//            // Assuming text_position has line() and column() methods
//            auto line = error.begin().line();
//            auto column = error.begin().column();
//            std::cout << "Error at line " << line << ", column " << column << std::endl;
//        }

    }

    // 5. Show final position - CORRECTED to use pc.parse_position()
//    std::cout << "Lexer stopped at offset "
//              << std::distance(src.begin(), pc.parse_position()) << "\n";
}

#if 0
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






extern void test_cpp_parser_with_comments();

#endif // 0

int main() {
    test_cpp_lexer();

#if 0
    std::cout << "\n----------------------------------------\n";
    test_cpp_parser();
    std::cout << "\n----------------------------------------\n";
    test_declarations();
    std::cout << "\n----------------------------------------\n";
//    test_cpp_parser_with_comments();

#endif // 0
    return 0;
}


#endif // 0
