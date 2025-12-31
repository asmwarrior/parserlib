#ifndef CPP_PARSER_GRAMMAR_H
#define CPP_PARSER_GRAMMAR_H

#include <string>
#include <unordered_map>
#include "parserlib.hpp"

#include "cpp_lexer_grammar.h"


using namespace parserlib;


class cpp_parser_grammar {
public:
    // --- Token IDs ---
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
        USING_DECL,

        STRUCT_DEF,
        NAMESPACE_DEF,
        TYPEDEF_DECL,
        ALIAS_DECL,
        RETURN_STMT,
        BREAK_STMT,
        CONTINUE_STMT,
        INCLUDE_DIRECTIVE,
        MACRO_DEFINE,
        MACRO_IF_BLOCK,
        SWITCH_STMT,
        CASE_LABEL,
        DEFAULT_LABEL,
        TRY_CATCH,
        THROW_STMT,
        OPERATOR_FUNC_DEF,
        OPERATOR_FUNC_DECL,
        CONSTRUCTOR_DEF,
        CONSTRUCTOR_DECL,
        DESTRUCTOR_DEF,
        DESTRUCTOR_DECL,
        TEMPLATE_SPEC_DEF,
        COMMENT,
        UNKNOWN_STATEMENT,
        ENUMBERATOR,
        ENUM_LIST,
        ENUM_ID,
        ENUMERATOR
    };

    // --- Error IDs ---
    enum class error_id_type {
        INVALID_TOKEN,
        INCOMPLETE_PARSE,
        INVALID_STATEMENT,
        INVALID_TEMPLATE
    };

    using parser_match_container_type = cpp_lexer_grammar::parse_context_type::match_container_type;

    using parse_context_type = parse_context<parser_match_container_type::const_iterator, match_id_type, error_id_type>;

    // --- parse node pointer type ---
    using parse_node_ptr_type = parse_node_ptr<parse_context_type>;

    // --- grammar type ---
    using grammar_type = parse_node_ptr_type;

    // --- returns string from match id type ---
    static const char* match_id_to_string(match_id_type id);

    // --- parse ---
    bool parse(parse_context_type& pc);

//    template <typename Node>
//    static void print_ast(const Node& node, int indent = 0) {
//        std::string space(indent, ' ');
//        std::cout << space << match_id_to_string(node->id()) << space
//                  << node->children().size() << " \"" << node->source() << "\"\n";
//        for (auto& c : node->children())
//            print_ast(c, indent + 2);
//    }


private:
    class instance;  // Forward declaration

};


#endif // CPP_PARSER_GRAMMAR_H
