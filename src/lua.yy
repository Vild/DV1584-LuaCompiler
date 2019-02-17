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

template <class T, Args...>
inline auto make(Args args) { return std::make_unique<T>(std::forward(args)); }

}
%code {
#define YY_DECL yy::parser::symbol_type yylex()
YY_DECL;
}

%token <ast::token::BinOPToken> BINOP
%token <ast::token::MinusOPToken> MINUSOP
%token <ast::token::UnOPToken> UNOP
%token <ast::token::SemicolonToken> SEMICOLON
%token <ast::token::EqualsToken> EQUALS
%token <ast::token::CommaToken> COMMA
%token <ast::token::DotToken> DOT
%token <ast::token::ColonToken> COLON
%token <ast::token::SquareOpenToken> SQUARE_OPEN
%token <ast::token::SquareCloseToken> SQUARE_CLOSE
%token <ast::token::VariableListToken> VARIABLE_LIST
%token <ast::token::ParenthesisOpenToken> PARENTHESIS_OPEN
%token <ast::token::ParenthesisCloseToken> PARENTHESIS_CLOSE
%token <ast::token::ListOpenToken> LIST_OPEN
%token <ast::token::ListCloseToken> LIST_CLOSE
%token <ast::token::DoToken> DO
%token <ast::token::EndToken> END
%token <ast::token::WhileToken> WHILE
%token <ast::token::RepeatToken> REPEAT
%token <ast::token::UntilToken> UNTIL
%token <ast::token::IfToken> IF
%token <ast::token::ThenToken> THEN
%token <ast::token::ElseIfToken> ELSEIF
%token <ast::token::ElseToken> ELSE
%token <ast::token::ForToken> FOR
%token <ast::token::InToken> IN
%token <ast::token::FunctionToken> FUNCTION
%token <ast::token::LocalToken> LOCAL
%token <ast::token::ReturnToken> RETURN
%token <ast::token::BreakToken> BREAK
%token <ast::token::NilToken> NIL
%token <ast::token::TrueToken> TRUE
%token <ast::token::FalseToken> FALSE
%token <ast::token::QuotedToken> QUOTED
%token <ast::token::DblQuotedToken> DBLQUOTED
%token <ast::token::NameToken> NAME
%token <ast::token::NumberToken> NUMBER
%token EndOfFile 0 "end of file"


%type <ast::token::NameToken> Name
%type <ast::token::NumberToken> Number
%type <ast::token::BinOPToken> Binop
%type <ast::token::UnOPToken> Unop

%type <ast::RootNodePtr&> root
%type <ast::NodePtr> block stat laststat lastChunkStatement chunkStatement
%type <ast::ChunkNodePtr> chunk
%type <std::vector<ast::NodePtr>> chunkStatements

%%

root : block { root = make<ast::RootNode>(); root->children.push_back(token); $$ = root; }
| root block { root->children.push_back($2); $$ = root; }
;

block : chunk { $$ = $1; }
;

chunk : chunkStatements lastChunkStatement {
		$$ = make<ast::ChunkNode>();
		$$->children = $1;
		$$->children.push_back($2);
}
| chunkStatements {
		$$ = make<ast::ChunkNode>();
		$$->children = $1;
	}
| lastChunkStatement {
		$$ = make<ast::ChunkNode>();
		$$->children.push_back($1);
}
;

lastChunkStatement : laststat { $$ = $1; }
| lastChunkStatement SEMICOLON { $$ = $1; }
;

chunkStatements : chunkStatement { $$ = std::vector<ast::NodePtr>(); $$.push_back($1); }
| chunkStatements chunkStatement { $1.push_back($2); $$ = $1; }
;

chunkStatement : stat { $$ = $1; }
| chunkStatement SEMICOLON { $$ = $1; }
;

stat : varlist EQUALS explist { $$ = make<AssignValueNode>($1, $2); }
| functioncall { $$ = make<FunctionCall>($1); }
| FUNCTION funcname funcbody { $$ = make<FunctionDefinitionNode>($2, $3); }
| LOCAL FUNCTION Name funcbody { $$ = make<AssignValueNode>($2, $3); }
| LOCAL namelist { $$ = make<ConstructLocalVariable>($2); }
| LOCAL namelist EQUALS explist { $$ = make<ConstructLocalVariable>($2, $4); }
;

laststat : RETURN { $$ = make<ReturnNode>(); }
| RETURN explist { $$ = make<ReturnNode>($2); }
| BREAK { $$ = make<BreakNode>(); }
;

funcnamePart : Name {}
| funcnamePart DOT Name {}
;

funcname : funcnamePart {}
| funcnamePart COLON Name {}
;

varlist : var {}
| varlist COMMA var {}
;

var : Name {}
| prefixexp SQUARE_OPEN exp SQUARE_CLOSE {}
| prefixexp DOT Name {}
;

namelist : Name {}
| namelist COMMA Name {}
;

explist : exp {}
| explist COMMA exp {}
;

exp : NIL {}
| FALSE {}
| TRUE {}
| Number {}
| String {}
| VARIABLE_LIST {}
| function {}
| prefixexp {}
| tableconstructor {}
| exp Binop exp {}
| Unop exp {}
;

prefixexp : var {}
| functioncall {}
| PARENTHESIS_OPEN exp PARENTHESIS_CLOSE {}
;

functioncall : prefixexp args {}
| prefixexp COLON Name args {}
;

args : PARENTHESIS_OPEN PARENTHESIS_CLOSE {}
| PARENTHESIS_OPEN explist PARENTHESIS_CLOSE {}
| tableconstructor {}
| String {}
;

function : FUNCTION funcbody {}
;

funcbody : PARENTHESIS_OPEN PARENTHESIS_CLOSE block END {}
| PARENTHESIS_OPEN parlist PARENTHESIS_CLOSE block END {}
;

parlist : namelist
| namelist COMMA VARIABLE_LIST {}
| VARIABLE_LIST {}
;

tableconstructor : LIST_OPEN LIST_CLOSE {}
| LIST_OPEN fieldlist LIST_CLOSE {}
;

fieldlistParts : field {}
| fieldlistParts fieldsep field {}
;

fieldlist : fieldlistParts {}
| fieldlistParts fieldsep {}
;

field : SQUARE_OPEN exp SQUARE_CLOSE EQUALS exp {}
| Name EQUALS exp {}
| exp {}
;

fieldsep : COMMA {}
| SEMICOLON {}
;


String : DBLQUOTED {}
| QUOTED {}
;
Name : NAME { $$ = $1; }
;
Number : NUMBER { $$ = $1; }
;

Binop : BINOP { $$ = $1; }
| MINUSOP { $$ = {ast::token::BinOPToken::OP::minus}; }
;
Unop : UNOP { $$ = $1; }
| MINUSOP { $$ = {ast::token::UnOPToken::OP::minus}; }
;
