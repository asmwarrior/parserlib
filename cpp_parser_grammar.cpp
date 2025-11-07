#include "cpp_parser_grammar.h"

const char* cpp_parser_grammar::match_id_to_string(match_id_type id) {
    switch (id) {
        // Core declarations
        case match_id_type::VAR_DECL:              return "VAR_DECL";
        case match_id_type::FUNC_DECL:             return "FUNC_DECL";
        case match_id_type::FUNC_DEF:              return "FUNC_DEF";
        case match_id_type::CLASS_DEF:             return "CLASS_DEF";
        case match_id_type::STRUCT_DEF:            return "STRUCT_DEF";
        case match_id_type::ENUM_DEF:              return "ENUM_DEF";
        case match_id_type::NAMESPACE_DEF:         return "NAMESPACE_DEF";
        case match_id_type::TYPEDEF_DECL:          return "TYPEDEF_DECL";
        case match_id_type::ALIAS_DECL:            return "ALIAS_DECL";
        case match_id_type::USING_DECL:            return "USING_DECL";
        case match_id_type::TEMPLATE_CLASS_DEF:    return "TEMPLATE_CLASS_DEF";
        case match_id_type::TEMPLATE_FUNC_DEF:     return "TEMPLATE_FUNC_DEF";
        case match_id_type::TEMPLATE_FUNC_DECL:    return "TEMPLATE_FUNC_DECL";
        case match_id_type::TEMPLATE_PARAMS:       return "TEMPLATE_PARAMS";

        // Statements and expressions
        case match_id_type::BLOCK:                 return "BLOCK";
        case match_id_type::BLOCK_SKIP:            return "BLOCK_SKIP";
        case match_id_type::EXPRESSION_STATEMENT:  return "EXPRESSION_STATEMENT";
        case match_id_type::CONTROL_FLOW:          return "CONTROL_FLOW";
        case match_id_type::FOR_LOOP:              return "FOR_LOOP";
        case match_id_type::SWITCH_STMT:           return "SWITCH_STMT";
        case match_id_type::CASE_LABEL:            return "CASE_LABEL";
        case match_id_type::DEFAULT_LABEL:         return "DEFAULT_LABEL";
        case match_id_type::RETURN_STMT:           return "RETURN_STMT";
        case match_id_type::BREAK_STMT:            return "BREAK_STMT";
        case match_id_type::CONTINUE_STMT:         return "CONTINUE_STMT";
        case match_id_type::THROW_STMT:            return "THROW_STMT";
        case match_id_type::TRY_CATCH:             return "TRY_CATCH";

        // Functions, operators, constructors
        case match_id_type::CONSTRUCTOR_DEF:       return "CONSTRUCTOR_DEF";
        case match_id_type::CONSTRUCTOR_DECL:      return "CONSTRUCTOR_DECL";
        case match_id_type::DESTRUCTOR_DEF:        return "DESTRUCTOR_DEF";
        case match_id_type::DESTRUCTOR_DECL:       return "DESTRUCTOR_DECL";
        case match_id_type::OPERATOR_FUNC_DEF:     return "OPERATOR_FUNC_DEF";
        case match_id_type::OPERATOR_FUNC_DECL:    return "OPERATOR_FUNC_DECL";

        // Preprocessor & macros
        case match_id_type::INCLUDE_DIRECTIVE:     return "INCLUDE_DIRECTIVE";
        case match_id_type::MACRO_DEFINE:          return "MACRO_DEFINE";
        case match_id_type::MACRO_IF_BLOCK:        return "MACRO_IF_BLOCK";

        // Miscellaneous
        case match_id_type::CLASS_ID:              return "CLASS_ID";
        case match_id_type::TOP_LEVEL:             return "TOP_LEVEL";
        case match_id_type::COMMENT:               return "COMMENT";
        case match_id_type::UNKNOWN_STATEMENT:     return "UNKNOWN_STATEMENT";

        default:
            return "UNKNOWN";
    }
}


// Define the instance class in the .cpp file
class cpp_parser_grammar::instance {
public:
    using ParseContext = cpp_parser_grammar::parse_context_type;

    instance() {
        using id_type = typename cpp_lexer_grammar::match_id_type;

        // ===== helper skip rules (keep your originals) =====
        angle_bracket_skip_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            if (!pc.is_valid_parse_position() ||
                pc.parse_position()->id() != id_type::LT)
                return false;

            pc.increment_parse_position();
            int angle_level = 1;
            while (angle_level > 0 && pc.is_valid_parse_position()) {
                auto token_id = pc.parse_position()->id();
                if (token_id == id_type::LT) angle_level++;
                else if (token_id == id_type::GT) angle_level--;
                pc.increment_parse_position();
            }
            return angle_level == 0;
        });

        block_skip_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            if (!pc.is_valid_parse_position() ||
                pc.parse_position()->id() != id_type::LEFT_BRACE)
                return false;

            pc.increment_parse_position();
            int brace_level = 1;
            while (brace_level > 0 && pc.is_valid_parse_position()) {
                auto id = pc.parse_position()->id();
                if (id == id_type::LEFT_BRACE) brace_level++;
                else if (id == id_type::RIGHT_BRACE) brace_level--;
                pc.increment_parse_position();
            }
            return brace_level == 0;
        });

        skip_to_semicolon_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            int paren = 0, brace = 0, bracket = 0;
            while (pc.is_valid_parse_position()) {
                auto id = pc.parse_position()->id();
                if (id == id_type::SEMICOLON && !paren && !brace && !bracket) {
                    pc.increment_parse_position();
                    return true;
                }
                if (id == id_type::LEFT_PAREN) paren++;
                else if (id == id_type::RIGHT_PAREN) paren--;
                else if (id == id_type::LEFT_BRACE) brace++;
                else if (id == id_type::RIGHT_BRACE) brace--;
                else if (id == id_type::LEFT_BRACKET) bracket++;
                else if (id == id_type::RIGHT_BRACKET) bracket--;
                pc.increment_parse_position();
            }
            return false;
        });

        paren_skip_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            if (!pc.is_valid_parse_position() ||
                pc.parse_position()->id() != id_type::LEFT_PAREN)
                return false;

            pc.increment_parse_position();
            int level = 1;
            while (level > 0 && pc.is_valid_parse_position()) {
                auto id = pc.parse_position()->id();
                if (id == id_type::LEFT_PAREN) level++;
                else if (id == id_type::RIGHT_PAREN) level--;
                pc.increment_parse_position();
            }
            return level == 0;
        });

        // ===== base statements =====
        expression_statement = (skip_to_semicolon_rule)
            ->*match_id_type::EXPRESSION_STATEMENT;

        var_decl = (terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::VAR_DECL;

        typedef_decl = (terminal(id_type::TYPEDEF)
            >> +(terminal(id_type::IDENTIFIER))
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::TYPEDEF_DECL;

        alias_decl = (terminal(id_type::USING)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::ASSIGN)
            >> +(terminal(id_type::IDENTIFIER))
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::ALIAS_DECL;

        return_stmt = (terminal(id_type::RETURN)
            >> skip_to_semicolon_rule)
            ->*match_id_type::RETURN_STMT;

        break_stmt = (terminal(id_type::BREAK)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::BREAK_STMT;

        continue_stmt = (terminal(id_type::CONTINUE)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::CONTINUE_STMT;

        throw_stmt = (terminal(id_type::THROW)
            >> skip_to_semicolon_rule)
            ->*match_id_type::THROW_STMT;

        control_flow_statement = (
            (terminal(id_type::IF) | terminal(id_type::WHILE) | terminal(id_type::SWITCH))
            >> (paren_skip_rule->*match_id_type::PARAM_LIST)
            >> (block_skip_rule | expression_statement))
            ->*match_id_type::CONTROL_FLOW;

        for_loop = (terminal(id_type::FOR)
            >> (paren_skip_rule->*match_id_type::PARAM_LIST)
            >> (block_skip_rule | expression_statement))
            ->*match_id_type::FOR_LOOP;

        switch_stmt = (terminal(id_type::SWITCH)
            >> (paren_skip_rule->*match_id_type::PARAM_LIST)
            >> block_skip_rule)
            ->*match_id_type::SWITCH_STMT;

        case_label = (terminal(id_type::CASE)
            >> skip_to_semicolon_rule)
            ->*match_id_type::CASE_LABEL;

        default_label = (terminal(id_type::DEFAULT)
            >> terminal(id_type::COLON))
            ->*match_id_type::DEFAULT_LABEL;

        try_catch = (terminal(id_type::TRY)
            >> block_skip_rule
            >> +(terminal(id_type::CATCH) >> paren_skip_rule >> block_skip_rule))
            ->*match_id_type::TRY_CATCH;

        func_decl = (terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::LEFT_PAREN)
            >> terminal(id_type::RIGHT_PAREN)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::FUNC_DECL;

        func_def = (terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::LEFT_PAREN)
            >> terminal(id_type::RIGHT_PAREN)
            >> (block_skip_rule->*match_id_type::BLOCK_SKIP)
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::FUNC_DEF;

        struct_def = (terminal(id_type::STRUCT)
            >> (terminal(id_type::IDENTIFIER)->*match_id_type::CLASS_ID)
            >> block
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::STRUCT_DEF;

        namespace_def = (terminal(id_type::NAMESPACE)
            >> terminal(id_type::IDENTIFIER)
            >> block)
            ->*match_id_type::NAMESPACE_DEF;

        include_directive = (terminal(id_type::HASH)
            >> terminal(id_type::INCLUDE)
            >> +(terminal(id_type::IDENTIFIER)))
            ->*match_id_type::INCLUDE_DIRECTIVE;

        macro_define = (terminal(id_type::HASH)
            >> terminal(id_type::DEFINE)
            >> terminal(id_type::IDENTIFIER)
            >> skip_to_semicolon_rule)
            ->*match_id_type::MACRO_DEFINE;

        macro_if_block = (terminal(id_type::HASH)
            >> (terminal(id_type::IF) | terminal(id_type::IFDEF) | terminal(id_type::IFNDEF))
            >> +(terminal(id_type::IDENTIFIER))
            >> block_skip_rule)
            ->*match_id_type::MACRO_IF_BLOCK;

        comment = (terminal(id_type::COMMENT_SINGLE_LINE) | terminal(id_type::COMMENT_MULTI_LINE))
            ->*match_id_type::COMMENT;

        // Reuse your block and class definitions
        block = (terminal(id_type::LEFT_BRACE)
            >> *(func_def | class_def | struct_def | enum_def | func_decl
                 | var_decl | typedef_decl | alias_decl | control_flow_statement
                 | for_loop | return_stmt | break_stmt | continue_stmt
                 | try_catch | throw_stmt | expression_statement)
            >> terminal(id_type::RIGHT_BRACE))
            ->*match_id_type::BLOCK;

        class_def = (terminal(id_type::CLASS)
            >> (terminal(id_type::IDENTIFIER)->*match_id_type::CLASS_ID)
            >> block
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::CLASS_DEF;
                // --- Template-related rules (reintroduced) ---
        auto template_params = (terminal(id_type::TEMPLATE)
            >> (angle_bracket_skip_rule->*match_id_type::TEMPLATE_PARAMS));

        template_func_decl = (template_params
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::LEFT_PAREN)
            >> terminal(id_type::RIGHT_PAREN)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::TEMPLATE_FUNC_DECL;

        template_func_def = (template_params
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::LEFT_PAREN)
            >> terminal(id_type::RIGHT_PAREN)
            >> (block_skip_rule->*match_id_type::BLOCK_SKIP)
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::TEMPLATE_FUNC_DEF;

        template_class_def = (template_params
            >> terminal(id_type::CLASS)
            >> (terminal(id_type::IDENTIFIER)->*match_id_type::CLASS_ID)
            >> block
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::TEMPLATE_CLASS_DEF;


        // Top level
        top_level = +(include_directive | macro_define | macro_if_block
            | using_decl | namespace_def | class_def | struct_def
            | template_class_def | template_func_def | template_func_decl
            | func_def | func_decl | enum_def | var_decl
            | typedef_decl | alias_decl | control_flow_statement
            | for_loop | expression_statement | comment
            | error(error_id_type::INVALID_STATEMENT, skip_until_after(terminal(id_type::SEMICOLON))));
    }

    parse_result parse(ParseContext& pc) noexcept {
        return top_level.parse(pc);
    }

private:
rule<ParseContext>
    var_decl, func_decl, func_def, struct_def, class_def, block, block_skip_rule,
    paren_skip_rule, skip_to_semicolon_rule, angle_bracket_skip_rule,
    expression_statement, for_loop, control_flow_statement, enum_def,
    using_decl, typedef_decl, alias_decl, return_stmt, break_stmt, continue_stmt,
    throw_stmt, switch_stmt, case_label, default_label, try_catch,
    namespace_def, include_directive, macro_define, macro_if_block,
    comment, template_func_decl, template_func_def, template_class_def,
    top_level;

};


// Implement the parse method
parse_result cpp_parser_grammar::parse(parse_context_type& pc) const noexcept {
    instance inst;
    return inst.parse(pc);
}
