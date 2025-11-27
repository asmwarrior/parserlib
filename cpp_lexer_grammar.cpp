#include "cpp_lexer_grammar.h"


// --- Helper to convert token ID to string ---
const char* cpp_lexer_grammar::match_id_to_string(match_id_type id)
{
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

        default:
            return "UNKNOWN";

    }
    // This is necessary to avoid a compiler warning about non-void function
    // not returning a value on all control paths, even though all IDs are listed.
    throw std::logic_error("Unknown match_id_type passed to match_id_to_string");
}

    // --- Keyword map (string -> match_id_type) ---
const std::unordered_map<std::string, cpp_lexer_grammar::match_id_type>& cpp_lexer_grammar::keyword_map()
{
    static const std::unordered_map<std::string, match_id_type> map{
        {"if", match_id_type::IF},
        {"else", match_id_type::ELSE},
        {"while", match_id_type::WHILE},
        {"for", match_id_type::FOR},
        {"return", match_id_type::RETURN},
        {"class", match_id_type::CLASS},
        {"auto", match_id_type::AUTO},
        {"bool", match_id_type::BOOL},
        {"break", match_id_type::BREAK},
        {"case", match_id_type::CASE},
        {"catch", match_id_type::CATCH},
        {"char", match_id_type::CHAR},
        {"const", match_id_type::CONST},
        {"const_cast", match_id_type::CONST_CAST},
        {"continue", match_id_type::CONTINUE},
        {"default", match_id_type::DEFAULT},
        {"delete", match_id_type::DELETE},
        {"do", match_id_type::DO},
        {"double", match_id_type::DOUBLE},
        {"dynamic_cast", match_id_type::DYNAMIC_CAST},
        {"extern", match_id_type::EXTERN},
        {"float", match_id_type::FLOAT},
        {"goto", match_id_type::GOTO},
        {"int", match_id_type::INT},
        {"long", match_id_type::LONG},
        {"namespace", match_id_type::NAMESPACE},
        {"new", match_id_type::NEW},
        {"operator", match_id_type::OPERATOR},
        {"private", match_id_type::PRIVATE},
        {"protected", match_id_type::PROTECTED},
        {"public", match_id_type::PUBLIC},
        {"register", match_id_type::REGISTER},
        {"reinterpret_cast", match_id_type::REINTERPRET_CAST},
        {"short", match_id_type::SHORT},
        {"sizeof", match_id_type::SIZEOF},
        {"static", match_id_type::STATIC},
        {"static_cast", match_id_type::STATIC_CAST},
        {"struct", match_id_type::STRUCT},
        {"switch", match_id_type::SWITCH},
        {"template", match_id_type::TEMPLATE},
        {"this", match_id_type::THIS},
        {"throw", match_id_type::THROW},
        {"try", match_id_type::TRY},
        {"typedef", match_id_type::TYPEDEF},
        {"typeid", match_id_type::TYPEID},
        {"union", match_id_type::UNION},
        {"unsigned", match_id_type::UNSIGNED},
        {"using", match_id_type::USING},
        {"virtual", match_id_type::VIRTUAL},
        {"void", match_id_type::VOID},
        {"volatile", match_id_type::VOLATILE},
        {"wchar_t", match_id_type::WCHAR_T},
        {"enum", match_id_type::ENUM}
    };
    return map;
}


bool cpp_lexer_grammar::parse(parse_context_type& pc) const noexcept
{
    auto& grammar = get_grammar();
    auto result = grammar.parse(pc);

    for (auto& m : pc.matches()) {
        if (m.id() == match_id_type::IDENTIFIER) {
            std::string text{ m.source() };
            auto it = keyword_map().find(text);
            if (it != keyword_map().end())
                m.set_id(it->second);
        }
    }
    return result;
}





cpp_lexer_grammar::grammar_type& cpp_lexer_grammar::get_grammar()
{
    static grammar_type  grammar = [](){
        const auto newline = terminal("\r\n") | terminal('\n') | terminal('\r');
        const auto whitespace = terminal(' ') | terminal('\t') | newline;

        const auto digit = range('0', '9');
        const auto letter = range('a', 'z') | range('A', 'Z') | terminal('_');

        // --- Comment Rules (Skipped) ---
        // Single-line comment: // up to the end of the line (but not including the newline)
        // const auto single_line_comment = (terminal("//") >> *(any() - newline))->*match_id_type::COMMENT_SINGLE_LINE;
        const auto single_line_comment = terminal("//") >> *(any() - newline);

        // Multi-line comment: /* followed by anything non-greedily, ending with */
        // NOTE: A true regex for C-style comments is complex due to greediness and nesting.
        // We use a simplified pattern that matches "/*" followed by any characters until "*/".
        // A robust lexer would often use state management for this.
        const auto multi_line_comment_content = *(any() - terminal("*/"));
        // const auto multi_line_comment = (terminal("/*") >> multi_line_comment_content >> terminal("*/"))->*match_id_type::COMMENT_MULTI_LINE;
        const auto multi_line_comment = terminal("/*") >> multi_line_comment_content >> terminal("*/");

        // --- ADDED: Preprocessor Directive Rule ---
        // Matches '#' followed by everything until the newline. Since it has NO ->*match_id_type, it's skipped.
        const auto preprocessor_directive = terminal('#') >> *(any() - newline);

        // The 'skippable' element: whitespace OR a comment. We use '+' to match one or more.
        const auto skip_element = *(whitespace | single_line_comment | multi_line_comment | preprocessor_directive);


        // --- Token Definitions ---

        // Number token
        const auto number = (+digit >> -('.' >> +digit))->*match_id_type::NUMBER;

        // String Literal
        // Matches '"' followed by any character that is not '"' or a newline, or an escaped character.
        const auto string_char = terminal("\\\"") | terminal("\\\t") | terminal("\\\r") | terminal("\\\n") | (any() - terminal('\"'));
        const auto string_literal = (terminal('\"') >> *string_char >> terminal('\"'))->*match_id_type::STRING_LITERAL;

        // Identifier token
        const auto identifier = (+letter >> *(letter | digit))->*match_id_type::IDENTIFIER;

        // --- Operators and Symbols (Longest match first) ---

        // 3-char operators/symbols
        const auto left_shift_assign  = terminal("<<=") ->* match_id_type::LEFT_SHIFT;
        const auto right_shift_assign = terminal(">>=") ->* match_id_type::RIGHT_SHIFT;
        const auto ellipsis           = terminal("...") ->* match_id_type::COMMA; // Represents '...' variadic operator

        // 2-char operators/symbols
        const auto scope_res    = terminal("::") ->* match_id_type::SCOPE_RES;
        const auto logical_or   = terminal("||") ->* match_id_type::LOGICAL_OR;
        const auto logical_and  = terminal("&&") ->* match_id_type::LOGICAL_AND;
        const auto eq           = terminal("==") ->* match_id_type::EQ;
        const auto ne           = terminal("!=") ->* match_id_type::NE;
        const auto le           = terminal("<=") ->* match_id_type::LE;
        const auto ge           = terminal(">=") ->* match_id_type::GE;
        const auto plus_assign  = terminal("+=") ->* match_id_type::PLUS_ASSIGN;
        const auto minus_assign = terminal("-=") ->* match_id_type::MINUS_ASSIGN;
        const auto mul_assign   = terminal("*=") ->* match_id_type::MUL_ASSIGN;
        const auto div_assign   = terminal("/=") ->* match_id_type::DIV_ASSIGN;
        const auto increment    = terminal("++") ->* match_id_type::INCREMENT;
        const auto decrement    = terminal("--") ->* match_id_type::DECREMENT;
        const auto right_shift  = terminal(">>") ->* match_id_type::RIGHT_SHIFT;
        const auto left_shift   = terminal("<<") ->* match_id_type::LEFT_SHIFT;
        const auto arrow        = terminal("->") ->* match_id_type::ARROW;

        // 1-char operators/symbols
        const auto assign        = terminal('=')->*match_id_type::ASSIGN;
        const auto logical_not   = terminal('!')->*match_id_type::LOGICAL_NOT;
        const auto lt            = terminal('<')->*match_id_type::LT;
        const auto gt            = terminal('>')->*match_id_type::GT;
        const auto plus          = terminal('+')->*match_id_type::PLUS;
        const auto minus         = terminal('-')->*match_id_type::MINUS;
        const auto mul           = terminal('*')->*match_id_type::MUL;
        const auto div           = terminal('/')->*match_id_type::DIV;
        const auto bit_and       = terminal('&')->*match_id_type::BIT_AND;
        const auto bit_or        = terminal('|')->*match_id_type::BIT_OR;
        const auto bit_xor       = terminal('^')->*match_id_type::BIT_XOR;
        const auto bit_not       = terminal('~')->*match_id_type::BIT_NOT;
        const auto lparen        = terminal('(')->*match_id_type::LEFT_PAREN;
        const auto rparen        = terminal(')')->*match_id_type::RIGHT_PAREN;
        const auto lbrace        = terminal('{')->*match_id_type::LEFT_BRACE;
        const auto rbrace        = terminal('}')->*match_id_type::RIGHT_BRACE;
        const auto semicolon     = terminal(';')->*match_id_type::SEMICOLON;
        const auto comma         = terminal(',')->*match_id_type::COMMA;
        const auto colon         = terminal(':')->*match_id_type::COLON;
        const auto dot           = terminal('.')->*match_id_type::DOT;
        const auto question_mark = terminal('?')->*match_id_type::QUESTION_MARK;
        const auto lbracket      = terminal('[')->*match_id_type::LEFT_BRACKET;
        const auto rbracket      = terminal(']')->*match_id_type::RIGHT_BRACKET;

        // Token: Ordered from longest/most specific to shortest/least specific
        const auto token =
            string_literal | number |
            // 3-char ops
            left_shift_assign | right_shift_assign | ellipsis |
            // 2-char ops
            scope_res | logical_or | logical_and | eq | ne | le | ge |
            plus_assign | minus_assign | mul_assign | div_assign |
            increment | decrement | right_shift | left_shift | arrow |
            // 1-char ops/symbols
            assign | logical_not | lt | gt | plus | minus | mul | div |
            bit_and | bit_or | bit_xor | bit_not |
            lparen | rparen | lbrace | rbrace | semicolon | comma |
            colon | dot | question_mark |
            // Identifier comes last to prevent consuming keywords
            identifier;

        // Grammar: zero or more 'skip_element' OR 'token'
        return *( skip_element >> token >> skip_element );
    }();
    return grammar;
}

