#include "cpp_lexer_grammar.h"


using parse_ptr = parserlib::parse_node_ptr<cpp_lexer_grammar::parse_context_type>;


static auto digit() {
    return range('0', '9');
}


static auto letter() {
    return range('a', 'z') | range('A', 'Z') | terminal('_');
}


static auto newline() {
    return parserlib::newline(terminal("\r\n") | terminal('\n') | terminal('\r'));
}


static auto skip_element() {
    const auto whitespace             = terminal(' ') | terminal('\t') | newline();
    const auto single_line_comment    = terminal("//") >> *(any() - (newline() | end())) >> (newline() | end());
    const auto multi_line_comment     = terminal("/*") >> *((newline() | any()) - terminal("*/")) >> terminal("*/");
    const auto preprocessor_directive = terminal('#') >> *(any() - (newline() | end())) >> (newline() | end());
    return whitespace | single_line_comment | multi_line_comment | preprocessor_directive;
}


static auto string_literal() {
    const auto string_char = terminal("\\\"") | terminal("\\\t") | terminal("\\\r") | terminal("\\\n") | (any() - terminal('\"'));
    return (terminal('\"') >> *string_char >> terminal('\"'))->*cpp_lexer_grammar::match_id_type::STRING_LITERAL;
}


static auto number() {
    return (+digit() >> -('.' >> +digit()))->*cpp_lexer_grammar::match_id_type::NUMBER;
}


static auto three_char_token() {
    const auto left_shift_assign  = terminal("<<=")->*cpp_lexer_grammar::match_id_type::LEFT_SHIFT;
    const auto right_shift_assign = terminal(">>=")->*cpp_lexer_grammar::match_id_type::RIGHT_SHIFT;
    const auto ellipsis           = terminal("...")->*cpp_lexer_grammar::match_id_type::COMMA;
    return left_shift_assign | right_shift_assign | ellipsis;
}


static auto two_char_token() {
    const auto scope_res    = terminal("::")->*cpp_lexer_grammar::match_id_type::SCOPE_RES;
    const auto logical_or   = terminal("||")->*cpp_lexer_grammar::match_id_type::LOGICAL_OR;
    const auto logical_and  = terminal("&&")->*cpp_lexer_grammar::match_id_type::LOGICAL_AND;
    const auto eq           = terminal("==")->*cpp_lexer_grammar::match_id_type::EQ;
    const auto ne           = terminal("!=")->*cpp_lexer_grammar::match_id_type::NE;
    const auto le           = terminal("<=")->*cpp_lexer_grammar::match_id_type::LE;
    const auto ge           = terminal(">=")->*cpp_lexer_grammar::match_id_type::GE;
    const auto plus_assign  = terminal("+=")->*cpp_lexer_grammar::match_id_type::PLUS_ASSIGN;
    const auto minus_assign = terminal("-=")->*cpp_lexer_grammar::match_id_type::MINUS_ASSIGN;
    const auto mul_assign   = terminal("*=")->*cpp_lexer_grammar::match_id_type::MUL_ASSIGN;
    const auto div_assign   = terminal("/=")->*cpp_lexer_grammar::match_id_type::DIV_ASSIGN;
    const auto increment    = terminal("++")->*cpp_lexer_grammar::match_id_type::INCREMENT;
    const auto decrement    = terminal("--")->*cpp_lexer_grammar::match_id_type::DECREMENT;
    const auto right_shift  = terminal(">>")->*cpp_lexer_grammar::match_id_type::RIGHT_SHIFT;
    const auto left_shift   = terminal("<<")->*cpp_lexer_grammar::match_id_type::LEFT_SHIFT;
    const auto arrow        = terminal("->")->*cpp_lexer_grammar::match_id_type::ARROW;
    return scope_res | logical_or | logical_and | eq | ne | le | ge | plus_assign | minus_assign | mul_assign | div_assign | increment | decrement | right_shift | left_shift | arrow;
}


static auto one_char_token() {
    const auto assign        = terminal('=')->*cpp_lexer_grammar::match_id_type::ASSIGN;
    const auto logical_not   = terminal('!')->*cpp_lexer_grammar::match_id_type::LOGICAL_NOT;
    const auto lt            = terminal('<')->*cpp_lexer_grammar::match_id_type::LT;
    const auto gt            = terminal('>')->*cpp_lexer_grammar::match_id_type::GT;
    const auto plus          = terminal('+')->*cpp_lexer_grammar::match_id_type::PLUS;
    const auto minus         = terminal('-')->*cpp_lexer_grammar::match_id_type::MINUS;
    const auto mul           = terminal('*')->*cpp_lexer_grammar::match_id_type::MUL;
    const auto div           = terminal('/')->*cpp_lexer_grammar::match_id_type::DIV;
    const auto bit_and       = terminal('&')->*cpp_lexer_grammar::match_id_type::BIT_AND;
    const auto bit_or        = terminal('|')->*cpp_lexer_grammar::match_id_type::BIT_OR;
    const auto bit_xor       = terminal('^')->*cpp_lexer_grammar::match_id_type::BIT_XOR;
    const auto bit_not       = terminal('~')->*cpp_lexer_grammar::match_id_type::BIT_NOT;
    const auto lparen        = terminal('(')->*cpp_lexer_grammar::match_id_type::LEFT_PAREN;
    const auto rparen        = terminal(')')->*cpp_lexer_grammar::match_id_type::RIGHT_PAREN;
    const auto lbrace        = terminal('{')->*cpp_lexer_grammar::match_id_type::LEFT_BRACE;
    const auto rbrace        = terminal('}')->*cpp_lexer_grammar::match_id_type::RIGHT_BRACE;
    const auto semicolon     = terminal(';')->*cpp_lexer_grammar::match_id_type::SEMICOLON;
    const auto comma         = terminal(',')->*cpp_lexer_grammar::match_id_type::COMMA;
    const auto colon         = terminal(':')->*cpp_lexer_grammar::match_id_type::COLON;
    const auto dot           = terminal('.')->*cpp_lexer_grammar::match_id_type::DOT;
    const auto question_mark = terminal('?')->*cpp_lexer_grammar::match_id_type::QUESTION_MARK;
    const auto lbracket      = terminal('[')->*cpp_lexer_grammar::match_id_type::LEFT_BRACKET;
    const auto rbracket      = terminal(']')->*cpp_lexer_grammar::match_id_type::RIGHT_BRACKET;
    return assign | logical_not | lt | gt | plus | minus | mul | div | bit_and | bit_or | bit_xor | bit_not | lparen | rparen | lbrace | rbrace | semicolon | comma | colon | dot | question_mark | lbracket | rbracket;
}


static parse_ptr char_token
    = three_char_token()
    | two_char_token()
    | one_char_token()
    ;


static auto identifier() {
    return (letter() >> *(letter() | digit()))->*cpp_lexer_grammar::match_id_type::IDENTIFIER;
}


static auto keyword_10_char_or_longer() {
    const auto keyword_reinterpret_cast = terminal("reinterpret_cast")->*cpp_lexer_grammar::match_id_type::REINTERPRET_CAST;
    const auto keyword_dynamic_cast     = terminal("dynamic_cast")->*cpp_lexer_grammar::match_id_type::DYNAMIC_CAST;
    const auto keyword_static_cast      = terminal("static_cast")->*cpp_lexer_grammar::match_id_type::STATIC_CAST;
    const auto keyword_const_cast       = terminal("const_cast")->*cpp_lexer_grammar::match_id_type::CONST_CAST;
    return keyword_reinterpret_cast | keyword_dynamic_cast | keyword_static_cast | keyword_const_cast;
}


static auto keyword_9_char() {
    const auto keyword_namespace = terminal("namespace")->*cpp_lexer_grammar::match_id_type::NAMESPACE;
    const auto keyword_protected = terminal("protected")->*cpp_lexer_grammar::match_id_type::PROTECTED;
    return keyword_namespace | keyword_protected;
}


static auto keyword_8_char() {
    const auto keyword_continue = terminal("continue")->*cpp_lexer_grammar::match_id_type::CONTINUE;
    const auto keyword_operator = terminal("operator")->*cpp_lexer_grammar::match_id_type::OPERATOR;
    const auto keyword_register = terminal("register")->*cpp_lexer_grammar::match_id_type::REGISTER;
    const auto keyword_template = terminal("template")->*cpp_lexer_grammar::match_id_type::TEMPLATE;
    const auto keyword_unsigned = terminal("unsigned")->*cpp_lexer_grammar::match_id_type::UNSIGNED;
    const auto keyword_volatile = terminal("volatile")->*cpp_lexer_grammar::match_id_type::VOLATILE;
    return keyword_continue | keyword_operator | keyword_register | keyword_template | keyword_unsigned | keyword_volatile;
}


static auto keyword_7_char() {
    const auto keyword_default = terminal("default")->*cpp_lexer_grammar::match_id_type::DEFAULT;
    const auto keyword_private = terminal("private")->*cpp_lexer_grammar::match_id_type::PRIVATE;
    const auto keyword_typedef = terminal("typedef")->*cpp_lexer_grammar::match_id_type::TYPEDEF;
    const auto keyword_virtual = terminal("virtual")->*cpp_lexer_grammar::match_id_type::VIRTUAL;
    const auto keyword_wchar_t = terminal("wchar_t")->*cpp_lexer_grammar::match_id_type::WCHAR_T;
    return keyword_default | keyword_private | keyword_typedef | keyword_virtual | keyword_wchar_t;
}


static auto keyword_6_char() {
    const auto keyword_delete = terminal("delete")->*cpp_lexer_grammar::match_id_type::DELETE;
    const auto keyword_double = terminal("double")->*cpp_lexer_grammar::match_id_type::DOUBLE;
    const auto keyword_extern = terminal("extern")->*cpp_lexer_grammar::match_id_type::EXTERN;
    const auto keyword_public = terminal("public")->*cpp_lexer_grammar::match_id_type::PUBLIC;
    const auto keyword_return = terminal("return")->*cpp_lexer_grammar::match_id_type::RETURN;
    const auto keyword_sizeof = terminal("sizeof")->*cpp_lexer_grammar::match_id_type::SIZEOF;
    const auto keyword_static = terminal("static")->*cpp_lexer_grammar::match_id_type::STATIC;
    const auto keyword_struct = terminal("struct")->*cpp_lexer_grammar::match_id_type::STRUCT;
    const auto keyword_switch = terminal("switch")->*cpp_lexer_grammar::match_id_type::SWITCH;
    const auto keyword_typeid = terminal("typeid")->*cpp_lexer_grammar::match_id_type::TYPEID;
    return keyword_delete | keyword_double | keyword_extern | keyword_public | keyword_return | keyword_sizeof | keyword_static | keyword_struct | keyword_switch | keyword_typeid;
}


static auto keyword_5_char() {
    const auto keyword_class = terminal("class")->*cpp_lexer_grammar::match_id_type::CLASS;
    const auto keyword_break = terminal("break")->*cpp_lexer_grammar::match_id_type::BREAK;
    const auto keyword_catch = terminal("catch")->*cpp_lexer_grammar::match_id_type::CATCH;
    const auto keyword_const = terminal("const")->*cpp_lexer_grammar::match_id_type::CONST;
    const auto keyword_float = terminal("float")->*cpp_lexer_grammar::match_id_type::FLOAT;
    const auto keyword_short = terminal("short")->*cpp_lexer_grammar::match_id_type::SHORT;
    const auto keyword_throw = terminal("throw")->*cpp_lexer_grammar::match_id_type::THROW;
    const auto keyword_union = terminal("union")->*cpp_lexer_grammar::match_id_type::UNION;
    const auto keyword_using = terminal("using")->*cpp_lexer_grammar::match_id_type::USING;
    const auto keyword_while = terminal("while")->*cpp_lexer_grammar::match_id_type::WHILE;
    return keyword_class | keyword_break | keyword_catch | keyword_const | keyword_float | keyword_short | keyword_throw | keyword_union | keyword_using | keyword_while;
}


static auto keyword_4_char() {
    const auto keyword_auto = terminal("auto")->*cpp_lexer_grammar::match_id_type::AUTO;
    const auto keyword_bool = terminal("bool")->*cpp_lexer_grammar::match_id_type::BOOL;
    const auto keyword_case = terminal("case")->*cpp_lexer_grammar::match_id_type::CASE;
    const auto keyword_char = terminal("char")->*cpp_lexer_grammar::match_id_type::CHAR;
    const auto keyword_else = terminal("else")->*cpp_lexer_grammar::match_id_type::ELSE;
    const auto keyword_goto = terminal("goto")->*cpp_lexer_grammar::match_id_type::GOTO;
    const auto keyword_long = terminal("long")->*cpp_lexer_grammar::match_id_type::LONG;
    const auto keyword_this = terminal("this")->*cpp_lexer_grammar::match_id_type::THIS;
    const auto keyword_void = terminal("void")->*cpp_lexer_grammar::match_id_type::VOID;
    const auto keyword_enum = terminal("enum")->*cpp_lexer_grammar::match_id_type::ENUM;
    return keyword_auto | keyword_bool | keyword_case | keyword_char | keyword_else | keyword_goto | keyword_long | keyword_this | keyword_void | keyword_enum;
}


static auto keyword_3_char() {
    const auto keyword_for = terminal("for")->*cpp_lexer_grammar::match_id_type::FOR;
    const auto keyword_int = terminal("int")->*cpp_lexer_grammar::match_id_type::INT;
    const auto keyword_new = terminal("new")->*cpp_lexer_grammar::match_id_type::NEW;
    const auto keyword_try = terminal("try")->*cpp_lexer_grammar::match_id_type::TRY;
    return keyword_for | keyword_int | keyword_new | keyword_try;
}


static auto keyword_2_char() {
    const auto keyword_do = terminal("do")->*cpp_lexer_grammar::match_id_type::DO;
    const auto keyword_if = terminal("if")->*cpp_lexer_grammar::match_id_type::IF;
    return keyword_do | keyword_if;
}


static parse_ptr get_keyword_grammar() {
    return keyword_10_char_or_longer()
         | keyword_9_char()
         | keyword_8_char()
         | keyword_7_char()
         | keyword_6_char()
         | keyword_5_char()
         | keyword_4_char()
         | keyword_3_char()
         | keyword_2_char()
         ;
}


static auto token() {
    return get_keyword_grammar()
         | string_literal()
         | number()
         | char_token
         | identifier()
         ;
}


cpp_lexer_grammar::grammar_type cpp_lexer_grammar::get_grammar() {
    return *(skip_element() | token());
}


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
