#include "cpp_parser_grammar.h"

const char* cpp_parser_grammar::match_id_to_string(match_id_type id) {
    switch (id) {
        case match_id_type::VAR_DECL:             return "VAR_DECL";
        case match_id_type::FUNC_DECL:            return "FUNC_DECL";
        case match_id_type::FUNC_DEF:             return "FUNC_DEF";
        case match_id_type::BLOCK:                return "BLOCK";
        case match_id_type::BLOCK_SKIP:           return "BLOCK_SKIP";
        case match_id_type::CLASS_DEF:            return "CLASS_DEF";
        case match_id_type::CLASS_ID:             return "CLASS_ID";
        case match_id_type::TOP_LEVEL:            return "TOP_LEVEL";
        case match_id_type::EXPRESSION_STATEMENT: return "EXPRESSION_STATEMENT";
        case match_id_type::FOR_LOOP:             return "FOR_LOOP";
        case match_id_type::CONTROL_FLOW:         return "CONTROL_FLOW";
        case match_id_type::PARAM_LIST:           return "PARAM_LIST";
        case match_id_type::ENUM_DEF:             return "ENUM_DEF";
        case match_id_type::TEMPLATE_PARAMS:      return "TEMPLATE_PARAMS";
        case match_id_type::TEMPLATE_FUNC_DECL:   return "TEMPLATE_FUNC_DECL";
        case match_id_type::TEMPLATE_FUNC_DEF:    return "TEMPLATE_FUNC_DEF";
        case match_id_type::TEMPLATE_CLASS_DEF:   return "TEMPLATE_CLASS_DEF";
        case match_id_type::USING_DECL:           return "USING_DECL";
    }
    return "UNKNOWN";
}

// Define the instance class in the .cpp file
class cpp_parser_grammar::instance {
public:
    using ParseContext = cpp_parser_grammar::parse_context_type;

    instance() {
        using id_type = typename lexer_grammar::match_id_type;


        // Add this in your instance() constructor
        angle_bracket_skip_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            if (!pc.is_valid_parse_position() ||
                pc.parse_position()->id() != id_type::LT) {
                return false;
            }

            pc.increment_parse_position();
            int angle_level = 1;

            while (angle_level > 0 && pc.is_valid_parse_position()) {
                const auto token_id = pc.parse_position()->id();
                if (token_id == id_type::LT) {
                    angle_level++;
                } else if (token_id == id_type::GT) {
                    angle_level--;
                }
                pc.increment_parse_position();
            }

            return angle_level == 0;
        });

        // --- Custom rule to skip a block based on nested brace levels ---
        block_skip_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;

            if (!pc.is_valid_parse_position() ||
                pc.parse_position()->id() != id_type::LEFT_BRACE) {
                return false;
            }

            pc.increment_parse_position();
            int brace_level = 1;

            while (brace_level > 0 && pc.is_valid_parse_position()) {
                const auto token_id = pc.parse_position()->id();
                if (token_id == id_type::LEFT_BRACE) {
                    brace_level++;
                } else if (token_id == id_type::RIGHT_BRACE) {
                    brace_level--;
                }
                pc.increment_parse_position();
            }

            return brace_level == 0;
        });

        // --- Custom rule to skip to semicolon ---
        skip_to_semicolon_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            int paren_level = 0, brace_level = 0, bracket_level = 0;

            if (!pc.is_valid_parse_position()) return false;

            while (pc.is_valid_parse_position()) {
                const auto token_id = pc.parse_position()->id();

                if (token_id == id_type::SEMICOLON) {
                    if (paren_level == 0 && brace_level == 0 && bracket_level == 0) {
                        pc.increment_parse_position();
                        return true;
                    }
                }

                if (token_id == id_type::LEFT_PAREN)          paren_level++;
                else if (token_id == id_type::RIGHT_PAREN)    paren_level--;
                else if (token_id == id_type::LEFT_BRACE)     brace_level++;
                else if (token_id == id_type::RIGHT_BRACE)    brace_level--;
                else if (token_id == id_type::LEFT_BRACKET)   bracket_level++;
                else if (token_id == id_type::RIGHT_BRACKET)  bracket_level--;

                pc.increment_parse_position();
            }
            return false;
        });

        // --- Paren skip rule (you need to define this) ---
        paren_skip_rule = function([](auto& pc) -> parse_result {
            using id_type = typename cpp_lexer_grammar::match_id_type;
            if (!pc.is_valid_parse_position() ||
                pc.parse_position()->id() != id_type::LEFT_PAREN) {
                return false;
            }

            pc.increment_parse_position();
            int paren_level = 1;

            while (paren_level > 0 && pc.is_valid_parse_position()) {
                const auto token_id = pc.parse_position()->id();
                if (token_id == id_type::LEFT_PAREN) {
                    paren_level++;
                } else if (token_id == id_type::RIGHT_PAREN) {
                    paren_level--;
                }
                pc.increment_parse_position();
            }

            return paren_level == 0;
        });

        // All your grammar rules here
        expression_statement = (skip_to_semicolon_rule)
            ->*match_id_type::EXPRESSION_STATEMENT;

        for_loop = (terminal(id_type::FOR)
            >> (paren_skip_rule->*match_id_type::PARAM_LIST)
            >> (block_skip_rule | expression_statement))
            ->*match_id_type::FOR_LOOP;

        control_flow_statement = (
            (terminal(id_type::IF) | terminal(id_type::WHILE) | terminal(id_type::SWITCH))
            >> (paren_skip_rule->*match_id_type::PARAM_LIST)
            >> (block_skip_rule | expression_statement))
            ->*match_id_type::CONTROL_FLOW;

        enum_def = (terminal(id_type::ENUM)
            >> -(terminal(id_type::CLASS) | terminal(id_type::STRUCT))
            >> terminal(id_type::IDENTIFIER)
            >> -(terminal(id_type::COLON) >> terminal(id_type::IDENTIFIER))
            >> (block_skip_rule->*match_id_type::BLOCK_SKIP)
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::ENUM_DEF;

        var_decl = (terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::VAR_DECL;

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

        block = (terminal(id_type::LEFT_BRACE)
            >> *(func_def | class_def | enum_def | func_decl
                 | control_flow_statement | for_loop | expression_statement)
            >> terminal(id_type::RIGHT_BRACE))
            ->*match_id_type::BLOCK;

        class_def = (terminal(id_type::CLASS)
            >> (terminal(id_type::IDENTIFIER)->*match_id_type::CLASS_ID)
            >> block
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::CLASS_DEF;

        // Template parameter list
        auto template_params = (terminal(id_type::TEMPLATE)
            >> (angle_bracket_skip_rule->*match_id_type::TEMPLATE_PARAMS));

        // Template function declaration
        auto template_func_decl = (template_params
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::LEFT_PAREN)
            >> terminal(id_type::RIGHT_PAREN)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::TEMPLATE_FUNC_DECL;

        // Template function definition
        auto template_func_def = (template_params
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::LEFT_PAREN)
            >> terminal(id_type::RIGHT_PAREN)
            >> (block_skip_rule->*match_id_type::BLOCK_SKIP)
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::TEMPLATE_FUNC_DEF;

        // Template class definition
        auto template_class_def = (template_params
            >> terminal(id_type::CLASS)
            >> (terminal(id_type::IDENTIFIER)->*match_id_type::CLASS_ID)
            >> block
            >> -terminal(id_type::SEMICOLON))
            ->*match_id_type::TEMPLATE_CLASS_DEF;

        using_decl = (terminal(id_type::USING)
            >> terminal(id_type::NAMESPACE)
            >> terminal(id_type::IDENTIFIER)
            >> terminal(id_type::SEMICOLON))
            ->*match_id_type::USING_DECL;

        // Update top_level to include template rules
        top_level = +(var_decl
                       | template_class_def
                       | template_func_def
                       | template_func_decl
                       | class_def
                       | enum_def
                       | func_def
                       | func_decl
                       | using_decl
                       | error(error_id_type::INVALID_STATEMENT, skip_until_after(terminal(id_type::SEMICOLON))));
    }

    parse_result parse(ParseContext& pc) noexcept {
        return top_level.parse(pc);
    }

private:

        private:
    rule<ParseContext> var_decl, func_decl, func_def, block, class_def, top_level,
        block_skip_rule, paren_skip_rule, skip_to_semicolon_rule,
        angle_bracket_skip_rule,  // Add this
        expression_statement, for_loop, control_flow_statement, enum_def, using_decl;
};

// Implement the parse method
parse_result cpp_parser_grammar::parse(parse_context_type& pc) const noexcept {
    instance inst;
    return inst.parse(pc);
}
