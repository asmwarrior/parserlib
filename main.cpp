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
    void bar() {
        int c;
        double d;
    })";

// --- Lexer grammar (Unchanged) ---
class cpp_lexer_grammar {
public:
    // ... (omitted for brevity, assume the original code is here) ...
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


    // --- Helper to convert token ID to string ---
    static const char* match_id_to_string(match_id_type id) {
        switch (id) {
            case match_id_type::NUMBER: return "NUMBER";
            case match_id_type::IDENTIFIER: return "IDENTIFIER";
            case match_id_type::PLUS: return "PLUS";
            case match_id_type::MINUS: return "MINUS";
            case match_id_type::MUL: return "MUL";
            case match_id_type::DIV: return "DIV";
            case match_id_type::LEFT_PAREN: return "LEFT_PAREN";
            case match_id_type::RIGHT_PAREN: return "RIGHT_PAREN";
            case match_id_type::LEFT_BRACE: return "LEFT_BRACE";
            case match_id_type::RIGHT_BRACE: return "RIGHT_BRACE";
            case match_id_type::SEMICOLON: return "SEMICOLON";
            case match_id_type::COMMA: return "COMMA";
            case match_id_type::IF: return "IF";
            case match_id_type::ELSE: return "ELSE";
            case match_id_type::WHILE: return "WHILE";
            case match_id_type::FOR: return "FOR";
            case match_id_type::RETURN: return "RETURN";
            case match_id_type::CLASS: return "CLASS";
        }
        // Fallback for any unknown ID
        return "UNKNOWN_TOKEN";
    }


    // --- Keyword map (string -> match_id_type) ---
    static const std::unordered_map<std::string, match_id_type>& keyword_map() {
        static const std::unordered_map<std::string, match_id_type> map{
            {"if",      match_id_type::IF},
            {"else",    match_id_type::ELSE},
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


            // --- Variable declaration: <type> <id> ';'
            var_decl = (terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::IDENTIFIER)
                        >> terminal(id_type::SEMICOLON))
                        ->*match_id_type::VAR_DECL;

            // --- Block: '{' { var_decl | func_def } '}'
            block = (terminal(id_type::LEFT_BRACE)
                     >> *(var_decl | func_def | func_decl)
                     >> terminal(id_type::RIGHT_BRACE))
                     ->*match_id_type::BLOCK;

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
        rule<ParseContext> var_decl, func_decl, func_def, block, class_def, top_level, block_skip_rule;
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

    if (result.success && !result.ast_nodes.empty()) {
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

    } else {
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
