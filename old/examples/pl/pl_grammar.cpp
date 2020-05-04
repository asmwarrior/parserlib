#include "pl_grammar.hpp"
using namespace parserlib;


/******************************************************************************
    GRAMMAR
 ******************************************************************************/


/**** TERMINALS ****/


//newline
#define NEWLINE              nl(expr("\r\n") | "\n\r" | '\n' | '\r')


//any character
#define ANY_CHAR             range(0, 255)


//block comments
#define BLOCK_COMMENTS       ("/*" >> *(!expr("*/") >> (NEWLINE | ANY_CHAR)) >> "*/")


//line comments
#define LINE_COMMENTS        ("//" >> *(!(NEWLINE | eof()) >> ANY_CHAR) >> (NEWLINE | eof()))


//letter
#define LETTER               (range('a', 'z') | range('A', 'Z'))


//digit
#define DIGIT                range('0', '9')


//string character
#define STRING_CHAR          (('\'' >> set("\"'nr0")) | (!set("\"'") >> ANY_CHAR))


//whitespace
rule whitespace = *(BLOCK_COMMENTS | LINE_COMMENTS | NEWLINE | range(0, 32));


//identifier
rule identifier = term((LETTER | '_') >> *(LETTER | DIGIT | '_'));


//float literal
rule float_literal = term(+DIGIT >> '.' >> +DIGIT >> -(set("eE") >> -set("+-") >> +DIGIT));


//integer literal
rule int_literal = term(+DIGIT);


//string literal
rule string_literal = term('"' >> *STRING_CHAR >> '"');


//char literal
rule char_literal = term('\'' >> STRING_CHAR >> '\'');


//boolean literal
rule bool_literal = term(expr("true") | "false");


/**** TYPES ****/


//void type
rule void_type = "void";


//integer type
rule int_type = "int";


//float type
rule float_type = "float";


//boolean type
rule bool_type = "bool";


//string type
rule string_type = "string";


//character type
rule char_type = "char";


//id type.
rule id_type = identifier;


//type
rule type = int_type
          | float_type
          | bool_type
          | string_type
          | char_type
          | id_type;


/**** EXPRESSIONS ****/


//function call
extern rule expression;
rule begin_paren = '(';
rule end_paren = ')';
rule function_call = identifier >> begin_paren >> -(expression >> *(',' >> expression)) >> end_paren;


//member access
rule member_access = identifier >> *('.' >> identifier);


//literal value
rule literal_value = float_literal
                   | int_literal
                   | string_literal
                   | char_literal
                   | bool_literal;


//value
rule value = function_call
           | member_access
           | literal_value
           | '(' >> expression >> ')';


//logical not
extern rule unary_expr;
rule logical_not_expr = '!' >> unary_expr;


//positive
rule positive_expr = '+' >> unary_expr;


//negative
rule negative_expr = '-' >> unary_expr;


//unary
rule unary_expr = logical_not_expr
                | positive_expr
                | negative_expr
                | value;


//multiplication
rule mul_op = '*' >> unary_expr;
rule div_op = '/' >> unary_expr;
rule mul_expr = unary_expr >> *(mul_op | div_op);


//add
rule add_op = '+' >> mul_expr;
rule sub_op = '-' >> mul_expr;
rule add_expr = mul_expr >> *(add_op | sub_op);


//compare expression
rule lt_op  = "<"  >> add_expr;
rule lte_op = "<=" >> add_expr;
rule gt_op  = ">"  >> add_expr;
rule gte_op = ">=" >> add_expr;
rule cmp_expr = add_expr >> *(lt_op | lte_op | gt_op | gte_op);


//equal expression
rule eq_op   = "==" >> cmp_expr;
rule diff_op = "!=" >> cmp_expr;
rule eq_expr = cmp_expr >> *(eq_op | diff_op);


//logical
rule log_and_op = "&&" >> eq_expr;
rule log_or_op  = "||" >> eq_expr;
rule logical_expr = eq_expr >> *(log_and_op | log_or_op);


//conditional
extern rule cond_expr;
rule cond_op = '?' >> cond_expr >> ':' >> cond_expr;
rule cond_expr = logical_expr >> -cond_op;


//expression
rule expression = cond_expr;


/**** STATEMENTS ****/


//block statement
extern rule statement;
rule block_begin = '{';
rule block_end = '}';
rule block_stm = block_begin >> *statement >> block_end;


//local variable statement
extern rule var_def;
rule var_stm = var_def;


//for statement
extern rule assignment_stm;
rule for_init = var_stm | assignment_stm;
rule for_cond = expression;
rule for_step = assignment_stm;
rule for_stm = expr("for") >> '(' >> -for_init >> ';' >> -for_cond >> ';' >> -for_step >> ')' >> block_stm;


//while statement
rule while_stm = expr("while") >> '(' >> expression >> ')' >> block_stm;


//if statement
extern rule if_stm;
rule else_if_stm = if_stm;
rule else_block_stm = block_stm;
rule if_stm = expr("if") >> '(' >> expression >> ')' >> block_stm >> -("else" >> (else_if_stm | else_block_stm));


//print statement
rule print_stm = "print" >> expression >> *(',' >> expression);


//reutrn statement
rule return_stm = "return" >> -expression;


//assignment statement
rule assignment_stm = expression >> '=' >> expression;


//expression statement
rule expression_stm = expression;


//statement
rule statement = block_stm
               | var_stm
               | for_stm
               | while_stm
               | if_stm
               | print_stm
               | return_stm
               | assignment_stm
               | expression_stm
               | ';';


/**** DECLARATIONS ****/


//variable instance
rule var_inst = identifier >> ':' >> type;


//variable initializer
rule var_init = '=' >> expression;


//variable definition
rule var_def = "var" >> var_inst >> -var_init;


//variable declaration
rule var_decl = var_def;


//struct member
rule struct_member = var_inst
                   | ';';


//struct declaration
rule struct_decl = "struct" >> identifier >> '{' >> *struct_member >> '}';


//function declaration
rule func_decl = "func" >> identifier >> '(' >> -(var_inst >> *(',' >> var_inst)) >> ')' >> ':' >> type >> block_stm;


//declaration
rule declaration = struct_decl
                 | var_decl
                 | func_decl
                 |  ';';


//translation unit
rule translation_unit = *declaration;
