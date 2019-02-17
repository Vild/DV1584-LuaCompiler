/* -*- mode: bison; c-set-style: cc-mode -*- */
%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%define api.token.constructor
%define parse.error verbose
%define parse.assert
%locations
%code requires {
#include <string>
#include <vector>
#include <iostream>

// Fix need for older bison version! source: http://www.jonathanbeard.io/tutorials/FlexBisonC++ 
// The following definitions is missing when %locations isn't used
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif
#include <token.hpp>
#include <ast.hpp>
}
%code {
#define YY_DECL yy::parser::symbol_type yylex()
YY_DECL;
}

%type <ast::Node> stream

%token <ast::token::BinOPToken> BINOP
%token <ast::token::MinusOPToken> MINUSOP
%token <ast::token::UnOPToken> UNOP
%token SEMICOLON EQUALS COMMA DOT COLON SQUARE_OPEN SQUARE_CLOSE VARIABLE_LIST PARENTHESIS_OPEN PARENTHESIS_CLOSE LIST_OPEN LIST_CLOSE
%token DO END WHILE REPEAT UNTIL IF THEN ELSEIF ELSE FOR IN FUNCTION LOCAL RETURN BREAK NIL TRUE FALSE
%token <std::string> QUOTED DBLQUOTED NAME NUMBER
%token EndOfFile 0 "end of file"
%%

stream : token {}
| stream token {}
;

token : BINOP {}
| MINUSOP {}
| UNOP {}
| SEMICOLON {}
| EQUALS {}
| COMMA {}
| DOT {}
| COLON {}
| SQUARE_OPEN {}
| SQUARE_CLOSE {}
| VARIABLE_LIST {}
| PARENTHESIS_OPEN {}
| PARENTHESIS_CLOSE {}
| LIST_OPEN {}
| LIST_CLOSE {}
| DO {}
| END {}
| WHILE {}
| REPEAT {}
| UNTIL {}
| IF {}
| THEN {}
| ELSEIF {}
| ELSE {}
| FOR {}
| IN {}
| FUNCTION {}
| LOCAL {}
| RETURN {}
| BREAK {}
| NIL {}
| TRUE {}
| FALSE {}
| QUOTED {}
| DBLQUOTED {}
| NAME {}
| NUMBER {}
;
