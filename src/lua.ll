/* -*- mode: flex; c-set-style: cc-mode -*- */
%top{
#include <lua.tab.hpp>
#include <cstdio>
#define YY_DECL yy::parser::symbol_type yylex()
#define YY_USER_ACTION {loc.columns(yyleng); printf("Token: '%s'\n", yytext);}
extern yy::location loc;
}
%x QUOTE
%option yylineno
%option noyywrap nounput batch noinput
%%
%{
loc.step();
%}

"+"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::plus}, loc); }
"*"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::mul}, loc); }
"/"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::div}, loc); }
"^"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::pow}, loc); }
"%"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::mod}, loc); }
".."  { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::dotdot}, loc); }
"<"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::less}, loc); }
"<="  { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::lessOrEqual}, loc); }
">"   { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::more}, loc); }
">="  { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::moreOrEqual}, loc); }
"=="  { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::equal}, loc); }
"~="  { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::notEqual}, loc); }
"and" { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::and_}, loc); }
"or"  { return yy::parser::make_BINOP({ast::token::BinOPToken::OP::or_}, loc); }

"-"   { return yy::parser::make_MINUSOP({}, loc); }

"not" { return yy::parser::make_UNOP({ast::token::UnOPToken::OP::not_}, loc); }
"#"   { return yy::parser::make_UNOP({ast::token::UnOPToken::OP::pound}, loc); }

";"    { return yy::parser::make_SEMICOLON(loc); }
"="    { return yy::parser::make_EQUALS(loc); }
","    { return yy::parser::make_COMMA(loc); }
"."    { return yy::parser::make_DOT(loc); }
":"    { return yy::parser::make_COLON(loc); }
"["    { return yy::parser::make_SQUARE_OPEN(loc); }
"]"    { return yy::parser::make_SQUARE_CLOSE(loc); }
"..."  { return yy::parser::make_VARIABLE_LIST(loc); }
"("    { return yy::parser::make_PARENTHESIS_OPEN(loc); }
")"    { return yy::parser::make_PARENTHESIS_CLOSE(loc); }
"{"    { return yy::parser::make_LIST_OPEN(loc); }
"}"    { return yy::parser::make_LIST_CLOSE(loc); }

"do"       { return yy::parser::make_DO(loc); }
"end"      { return yy::parser::make_END(loc); }
"while"    { return yy::parser::make_WHILE(loc); }
"repeat"   { return yy::parser::make_REPEAT(loc); }
"until"    { return yy::parser::make_UNTIL(loc); }
"if"       { return yy::parser::make_IF(loc); }
"then"     { return yy::parser::make_THEN(loc); }
"elseif"   { return yy::parser::make_ELSEIF(loc); }
"else"     { return yy::parser::make_ELSE(loc); }
"for"      { return yy::parser::make_FOR(loc); }
"in"       { return yy::parser::make_IN(loc); }
"function" { return yy::parser::make_FUNCTION(loc); }
"local"    { return yy::parser::make_LOCAL(loc); }
"return"   { return yy::parser::make_RETURN(loc); }
"break"    { return yy::parser::make_BREAK(loc); }
"nil"      { return yy::parser::make_NIL(loc); }
"true"     { return yy::parser::make_TRUE(loc); }
"false"    { return yy::parser::make_FALSE(loc); }




'[^']*'                    { return yy::parser::make_QUOTED(yytext, loc); }
<INITIAL>\"                { BEGIN(QUOTE); }
<QUOTE>\"                  { BEGIN(INITIAL); }
[[:blank:]\r\n]+           { loc.step(); }
<INITIAL>([[:digit:]]+)    { return yy::parser::make_NUMBER(yytext, loc); }
<INITIAL>([[:alpha:]_][[:alnum:]_]+)     { return yy::parser::make_NAME(yytext, loc); }
<QUOTE>(\\.|[^\"\\$=])+    { return yy::parser::make_DBLQUOTED(yytext, loc); }
<<EOF>>                    { return yy::parser::make_END(loc); }
.                          { throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }
%%
