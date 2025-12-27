#include "cpp_lexer_grammar.h"


// --- Helper to convert token ID to string ---
const char* cpp_lexer_grammar::match_id_to_string(match_id_type id) {
    switch (id) {
        case match_id_type::NUMBER:                 return "NUMBER";
        case match_id_type::IDENTIFIER:             return "IDENTIFIER";
        case match_id_type::STRING_LITERAL:         return "STRING_LITERAL";
        case match_id_type::LEFT_PAREN:             return "LEFT_PAREN";
        case match_id_type::RIGHT_PAREN:            return "RIGHT_PAREN";
        case match_id_type::LEFT_BRACE:             return "LEFT_BRACE";
        case match_id_type::RIGHT_BRACE:            return "RIGHT_BRACE";
        case match_id_type::SEMICOLON:              return "SEMICOLON";
        case match_id_type::COMMA:                  return "COMMA";
        case match_id_type::COLON:                  return "COLON";
        case match_id_type::DOT:                    return "DOT";
        case match_id_type::PLUS:                   return "PLUS";
        case match_id_type::MINUS:                  return "MINUS";
        case match_id_type::MUL:                    return "MUL";
        case match_id_type::DIV:                    return "DIV";
        case match_id_type::ASSIGN:                 return "ASSIGN";
        case match_id_type::LT:                     return "LT";
        case match_id_type::GT:                     return "GT";
        case match_id_type::BIT_AND:                return "BIT_AND";
        case match_id_type::BIT_OR:                 return "BIT_OR";
        case match_id_type::BIT_XOR:                return "BIT_XOR";
        case match_id_type::BIT_NOT:                return "BIT_NOT";
        case match_id_type::LOGICAL_NOT:            return "LOGICAL_NOT";
        case match_id_type::PLUS_ASSIGN:            return "PLUS_ASSIGN";
        case match_id_type::MINUS_ASSIGN:           return "MINUS_ASSIGN";
        case match_id_type::MUL_ASSIGN:             return "MUL_ASSIGN";
        case match_id_type::DIV_ASSIGN:             return "DIV_ASSIGN";
        case match_id_type::EQ:                     return "EQ";
        case match_id_type::NE:                     return "NE";
        case match_id_type::LE:                     return "LE";
        case match_id_type::GE:                     return "GE";
        case match_id_type::INCREMENT:              return "INCREMENT";
        case match_id_type::DECREMENT:              return "DECREMENT";
        case match_id_type::LOGICAL_AND:            return "LOGICAL_AND";
        case match_id_type::LOGICAL_OR:             return "LOGICAL_OR";
        case match_id_type::RIGHT_SHIFT:            return "RIGHT_SHIFT";
        case match_id_type::LEFT_SHIFT:             return "LEFT_SHIFT";
        case match_id_type::ARROW:                  return "ARROW";
        case match_id_type::SCOPE_RES:              return "SCOPE_RES";
        case match_id_type::AUTO:                   return "AUTO";
        case match_id_type::BOOL:                   return "BOOL";
        case match_id_type::BREAK:                  return "BREAK";
        case match_id_type::CASE:                   return "CASE";
        case match_id_type::CATCH:                  return "CATCH";
        case match_id_type::CHAR:                   return "CHAR";
        case match_id_type::CLASS:                  return "CLASS";
        case match_id_type::CONST:                  return "CONST";
        case match_id_type::CONST_CAST:             return "CONST_CAST";
        case match_id_type::CONTINUE:               return "CONTINUE";
        case match_id_type::DEFAULT:                return "DEFAULT";
        case match_id_type::DELETE:                 return "DELETE";
        case match_id_type::DO:                     return "DO";
        case match_id_type::DOUBLE:                 return "DOUBLE";
        case match_id_type::DYNAMIC_CAST:           return "DYNAMIC_CAST";
        case match_id_type::ELSE:                   return "ELSE";
        case match_id_type::EXTERN:                 return "EXTERN";
        case match_id_type::FLOAT:                  return "FLOAT";
        case match_id_type::FOR:                    return "FOR";
        case match_id_type::GOTO:                   return "GOTO";
        case match_id_type::IF:                     return "IF";
        case match_id_type::INT:                    return "INT";
        case match_id_type::LONG:                   return "LONG";
        case match_id_type::NAMESPACE:              return "NAMESPACE";
        case match_id_type::NEW:                    return "NEW";
        case match_id_type::OPERATOR:               return "OPERATOR";
        case match_id_type::PRIVATE:                return "PRIVATE";
        case match_id_type::PROTECTED:              return "PROTECTED";
        case match_id_type::PUBLIC:                 return "PUBLIC";
        case match_id_type::REGISTER:               return "REGISTER";
        case match_id_type::REINTERPRET_CAST:       return "REINTERPRET_CAST";
        case match_id_type::RETURN:                 return "RETURN";
        case match_id_type::SHORT:                  return "SHORT";
        case match_id_type::SIZEOF:                 return "SIZEOF";
        case match_id_type::STATIC:                 return "STATIC";
        case match_id_type::STATIC_CAST:            return "STATIC_CAST";
        case match_id_type::STRUCT:                 return "STRUCT";
        case match_id_type::SWITCH:                 return "SWITCH";
        case match_id_type::TEMPLATE:               return "TEMPLATE";
        case match_id_type::THIS:                   return "THIS";
        case match_id_type::THROW:                  return "THROW";
        case match_id_type::TRY:                    return "TRY";
        case match_id_type::TYPEDEF:                return "TYPEDEF";
        case match_id_type::TYPEID:                 return "TYPEID";
        case match_id_type::UNION:                  return "UNION";
        case match_id_type::UNSIGNED:               return "UNSIGNED";
        case match_id_type::USING:                  return "USING";
        case match_id_type::VIRTUAL:                return "VIRTUAL";
        case match_id_type::VOID:                   return "VOID";
        case match_id_type::VOLATILE:               return "VOLATILE";
        case match_id_type::WCHAR_T:                return "WCHAR_T";
        case match_id_type::WHILE:                  return "WHILE";
        case match_id_type::COMMENT_SINGLE_LINE:    return "COMMENT_SINGLE_LINE";
        case match_id_type::COMMENT_MULTI_LINE:     return "COMMENT_MULTI_LINE";
        case match_id_type::HASH:                   return "HASH";
        case match_id_type::PREPROCESSOR_DIRECTIVE: return "PREPROCESSOR_DIRECTIVE";
        case match_id_type::QUESTION_MARK:          return "QUESTION_MARK";
        case match_id_type::LEFT_BRACKET:           return "LEFT_BRACKET";
        case match_id_type::RIGHT_BRACKET:          return "RIGHT_BRACKET";
        case match_id_type::ENUM:                   return "ENUM";
        case match_id_type::INCLUDE:               return "INCLUDE";
        case match_id_type::DEFINE:                return "DEFINE";
        case match_id_type::IFDEF:                 return "IFDEF";
        case match_id_type::IFNDEF:                return "IFNDEF";
        case match_id_type::ENDIF:                 return "ENDIF";
        default:                                   return "UNKNOWN";
    }

    // This is necessary to avoid a compiler warning about non-void function
    // not returning a value on all control paths, even though all IDs are listed.
    throw std::logic_error("Unknown match_id_type passed to match_id_to_string");
}

bool cpp_lexer_grammar::parse(parse_context_type& pc) {
    auto grammar = get_grammar();
    auto result = grammar.parse(pc);
    return result;
}
