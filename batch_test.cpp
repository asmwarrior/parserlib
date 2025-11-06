#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

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




//-----------------------------------------------------
// Expected hierarchy structures (from previous logic)
//-----------------------------------------------------

struct Expected {
    std::string type;
    std::string name;
    std::vector<Expected> children;

    Expected(std::string t = {}, std::string n = {})
        : type(std::move(t)), name(std::move(n)) {}
};

struct AnnotatedLine {
    std::string code;
    std::string type;
    std::string name;
    int brace_diff = 0;  // +1 for '{', -1 for '}'
};

//-----------------------------------------------------
// Parse annotated source comments like "// CLASS_DEF Foo"
//-----------------------------------------------------

std::vector<AnnotatedLine> parse_annotated_source(const std::string& src) {
    std::vector<AnnotatedLine> result;
    std::istringstream in(src);
    std::string line;

    while (std::getline(in, line)) {
        AnnotatedLine l;

        // detect braces
        for (char c : line) {
            if (c == '{') l.brace_diff++;
            else if (c == '}') l.brace_diff--;
        }

        // find comment
        auto pos = line.find("//");
        if (pos != std::string::npos) {
            std::string comment = line.substr(pos + 2);
            auto first = comment.find_first_not_of(" \t");
            if (first != std::string::npos)
                comment = comment.substr(first);

            std::istringstream ws(comment);
            ws >> l.type; // e.g. CLASS_DEF
            ws >> l.name; // e.g. Foo
        }

        if (!l.type.empty())
            result.push_back(std::move(l));
    }

    return result;
}

//-----------------------------------------------------
// Build expected tree (supports multiple top-levels)
//-----------------------------------------------------

Expected build_expected_tree(const std::vector<AnnotatedLine>& lines) {
    Expected root("ROOT", "");
    std::vector<Expected*> stack = { &root };
    int current_depth = 0;

    for (const auto& line : lines) {
        Expected node(line.type, line.name);
        stack.back()->children.push_back(node);
        Expected* node_ptr = &stack.back()->children.back();

        if (line.brace_diff > 0) {
            stack.push_back(node_ptr);
            current_depth += line.brace_diff;
        }
        else if (line.brace_diff < 0) {
            for (int i = 0; i < -line.brace_diff && stack.size() > 1; ++i) {
                stack.pop_back();
                current_depth--;
            }
        }
    }

    return root;
}

//-----------------------------------------------------
// Print tree for debug
//-----------------------------------------------------

void print_expected(const Expected& node, int indent = 0) {
    if (node.type != "ROOT") {
        std::cout << std::string(indent * 4, ' ')
                  << node.type;
        if (!node.name.empty())
            std::cout << " " << node.name;
        std::cout << "\n";
    }
    for (const auto& c : node.children)
        print_expected(c, indent + 1);
}

//-----------------------------------------------------
// Compare two Expected trees recursively
//-----------------------------------------------------

bool compare_expected(const Expected& a, const Expected& b, int depth = 0) {
    bool type_match = (a.type == b.type);

    // Allow name to be skipped if one side is empty
    bool name_match = (a.name == b.name) || a.name.empty() || b.name.empty();

    if (!type_match || !name_match) {
        std::cout << std::string(depth * 4, ' ')
                  << "Mismatch: expected (" << a.type << " " << a.name
                  << ") got (" << b.type << " " << b.name << ")\n";
        return false;
    }

    if (a.type == b.type && a.name != b.name && (a.name.empty() || b.name.empty())) {
    std::cout << std::string(depth * 4, ' ')
              << "(note: matched by type only, name missing)\n";
    }


    if (a.children.size() != b.children.size()) {
        std::cout << std::string(depth * 4, ' ')
                  << "Child count mismatch for " << a.type << " " << a.name
                  << ": expected " << a.children.size()
                  << ", got " << b.children.size() << "\n";
        return false;
    }

    for (size_t i = 0; i < a.children.size(); ++i) {
        if (!compare_expected(a.children[i], b.children[i], depth + 1))
            return false;
    }

    return true;
}

//-----------------------------------------------------
// Integrate with your parser test
//-----------------------------------------------------

bool test_cpp_source_annotated(const std::string& source_str) {
    // Build expected tree from comments
    auto lines = parse_annotated_source(source_str);
    Expected expected_root = build_expected_tree(lines);

    std::cout << "Expected parse structure:\n";
    print_expected(expected_root);
    std::cout << "------------------------------------------------------------\n";

    // === your existing parser logic ===
    source_type source(source_str.c_str());

    using parser_type = parser<source_type, cpp_lexer_grammar, cpp_parser_grammar>;
    auto result = parser_type::parse(source);

    if (!result.success) {
        std::cout << "Parser failed. Return! \n";
        return false;
    }
    else
        std::cout << "Parser success!\n";

    std::cout << "------------------------------------------------------------\n";
    std::cout << "Parsing result print:\n";
    if (!result.ast_nodes.empty()) // result.success &&
    {
        for (auto &node : result.ast_nodes)
            cpp_parser_grammar::print_ast(node);
    }

    // Convert parser matches to Expected tree
    Expected actual_root("ROOT", "");
    for (const auto& node : result.ast_nodes) {
        actual_root.children.emplace_back(
            cpp_parser_grammar::match_id_to_string(node->id()), ""
        );
    }


    std::cout << "Actual parser matches:\n";
    print_expected(actual_root);
    std::cout << "------------------------------------------------------------\n";

    // Compare
    return compare_expected(expected_root, actual_root);
}

//-----------------------------------------------------
// Example test driver
//-----------------------------------------------------

void test_cpp_parser_with_comments() {
    const std::string src = R"(
class Foo    // CLASS_DEF Foo
{

};

abc x;   // VAR_DECL x

)";

    if (test_cpp_source_annotated(src))
        std::cout << " Test Passed\n";
    else
        std::cout << " Test Failed\n";
}

