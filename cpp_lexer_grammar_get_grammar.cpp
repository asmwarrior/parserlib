#include "cpp_lexer_grammar.h"


using namespace parserlib;


static auto digit() {
    return range('0', '9');
}


static auto letter() {
    return range('a', 'z') | range('A', 'Z') | terminal('_');
}


static auto newline() {
    return terminal("\r\n") | terminal('\n') | terminal('\r');
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
    return scope_res | logical_or | logical_and | eq | ne | le | ge | plus_assign | minus_assign | mul_assign | div_assign | increment | right_shift | left_shift | arrow;
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


static auto identifier() {
    return (letter() >> *(letter() | digit()))->*cpp_lexer_grammar::match_id_type::IDENTIFIER;
}


static auto token() {
    return cpp_lexer_grammar::get_keyword_grammar()
         | string_literal()
         | number()
         | three_char_token()
         | two_char_token()
         | one_char_token()
         | identifier()
         ;
}


cpp_lexer_grammar::grammar_type cpp_lexer_grammar::get_grammar() {
    return *(skip_element() | token());
}

