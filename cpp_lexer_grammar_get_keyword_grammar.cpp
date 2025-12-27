#include "cpp_lexer_grammar.h"


using namespace parserlib;


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


cpp_lexer_grammar::grammar_type cpp_lexer_grammar::get_keyword_grammar() {
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
