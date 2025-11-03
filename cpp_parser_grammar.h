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
        ENUM_DEF,
        TEMPLATE_PARAMS,
        TEMPLATE_FUNC_DECL,
        TEMPLATE_FUNC_DEF,
        TEMPLATE_CLASS_DEF,
        USING_DECL
    };

    enum class error_id_type {
        INVALID_TOKEN,
        INCOMPLETE_PARSE,
        INVALID_STATEMENT,
        INVALID_TEMPLATE
    };

    // Define the specific ParseContext type
    using parse_context_type = parse_context<
        typename lexer<line_counting_string<>, lexer_grammar>::parsed_token_container_type,
        match_id_type,
        error_id_type,
        case_sensitive_comparator,
        empty_parse_context_extension
    >;

    static const char* match_id_to_string(match_id_type id);

    template <typename Node>
    static void print_ast(const Node& node, int indent = 0) {
        std::string space(indent, ' ');
        std::cout << space << match_id_to_string(node->id()) << space
                  << node->children().size() << " \"" << node->source() << "\"\n";
        for (auto& c : node->children())
            print_ast(c, indent + 2);
    }

    // Non-template parse method
    parse_result parse(parse_context_type& pc) const noexcept;

private:
    class instance;  // Forward declaration
};

#endif
