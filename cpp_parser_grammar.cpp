#include "cpp_parser_grammar.h"

// --- Helper to convert enum to string ---
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
    }
    return "UNKNOWN";
}
