#ifndef CPP_PARSER_GRAMMAR_H
#define CPP_PARSER_GRAMMAR_H

#include <iostream>
#include <cassert>
#include <string>

#include "cpp_lexer_grammar.h"

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
    static const char* match_id_to_string(match_id_type id);

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
                      if (token_id == id_type::LEFT_PAREN)          paren_level++;
                      else if (token_id == id_type::RIGHT_PAREN)    paren_level--;
                      else if (token_id == id_type::LEFT_BRACE)     brace_level++;
                      else if (token_id == id_type::RIGHT_BRACE)    brace_level--;
                      else if (token_id == id_type::LEFT_BRACKET)   bracket_level++;
                      else if (token_id == id_type::RIGHT_BRACKET)  bracket_level--;

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


                  // This rule is for parsing the *inside* of a class or namespace.
                  // We must order this from most-specific to least-specific.
                  // 'expression_statement' MUST come last.
            block = (terminal(id_type::LEFT_BRACE) >>
                *( func_def
                   | class_def
                   | enum_def
                   | func_decl
                   | control_flow_statement
                   | for_loop
                   | expression_statement)
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
                        )) ->* match_id_type::TOP_LEVEL;
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

#endif // CPP_PARSER_GRAMMAR_H
