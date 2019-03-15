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

	template <typename T, typename... Args>
	inline std::shared_ptr<T> make(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

}
%code {
	extern std::shared_ptr<ast::RootNode> root;
#define YY_DECL yy::parser::symbol_type yylex()
YY_DECL;
}

%token <ast::token::BinOPToken> PLUS MUL DIV POW MOD DOTDOT LESS LESSOREQUAL MORE MOREOREQUAL EQUAL NOTEQUAL AND OR
%token <ast::token::MinusOPToken> MINUS
%token <ast::token::UnOPToken> NOT POUND
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
%token <ast::token::NameToken> NAME
%token <ast::token::NumberToken> NUMBER
%token EndOfFile 0 "end of file"


%type <std::shared_ptr<ast::StringNode>> String
%type <std::shared_ptr<ast::VariableRefNode>> Name
%type <std::shared_ptr<ast::NumberNode>> Number

%type <std::shared_ptr<ast::RootNode>> root
%type <ast::NodePtr> block stat laststat lastChunkStatement chunkStatement funcnamePart funcname var namelist explist exp prefixexp functioncall args function funcbody parlist tableconstructor fieldlistParts fieldlist field fieldsep expBinOPexp unopExp varlist
%type <std::shared_ptr<ast::ChunkNode>> chunk
%type <std::vector<ast::NodePtr>> chunkStatements



%left OR
%left AND
%left LESS MORE LESSOREQUAL MOREOREQUAL NOTEQUAL EQUAL
%left DOTDOT
%left PLUS MINUS
%left MUL DIV MOD
%precedence NOT POUND
%right POW

%%

root : block { $$ = root = make<ast::RootNode>(); root->children.push_back($1); }
| root block { $$ = $1; $1->children.push_back($2); }
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


stat : varlist EQUALS explist { $$ = make<ast::AssignValuesNode>($1, $3); }
| functioncall { $$ = $1; }
| FUNCTION funcname funcbody {
		auto var = make<ast::VariableListNode>();
		var->children.push_back($2);
		auto valExp = make<ast::ExpressionListNode>();
		valExp->children.push_back($3);

		$$ = make<ast::AssignValuesNode>(var, valExp);
 }
| LOCAL FUNCTION Name funcbody {
		$$ = make<ast::LocalAssignValueNode>($3, $4);
 }
| LOCAL Name { $$ = make<ast::LocalAssignValueNode>($2, make<ast::ValueNode>(ast::token::NilToken())); }
| LOCAL Name EQUALS exp { $$ = make<ast::LocalAssignValueNode>($2, $4); }
| FOR Name EQUALS exp COMMA exp DO block END { $$ = make<ast::ForLoopNode>($2, $4, $6, $8); }
| IF exp THEN block END { $$ = make<ast::IfNode>($2, $4, make<ast::ChunkNode>()); }
| IF exp THEN block ELSE block END { $$ = make<ast::IfNode>($2, $4, $6); }
| REPEAT block UNTIL exp { $$ = make<ast::RepeatNode>($2, $4); }
;

varlist : var { $$ = make<ast::VariableListNode>(); $$->children.push_back($1); }
| varlist COMMA var { $$ = $1; $$->children.push_back($3); }
;

laststat : RETURN { $$ = make<ast::ReturnNode>(); }
| RETURN exp { $$ = make<ast::ReturnNode>($2); }
| BREAK { $$ = make<ast::BreakNode>(); }
;

funcnamePart : Name { $$ = $1; }
| funcnamePart DOT Name { $$ = make<ast::IndexOfNode>($1, $3); }
;

funcname : funcnamePart { $$ = $1; }
;

var : Name { $$ = $1; }
| prefixexp SQUARE_OPEN exp SQUARE_CLOSE { $$ = make<ast::IndexOfNode>($1, $3);}
| prefixexp DOT Name { $$ = make<ast::IndexOfNode>($1, $3); }
;

namelist : Name { $$ = make<ast::NameListNode>(); $$->children.push_back($1); }
| namelist COMMA Name { $$ = $1; $$->children.push_back($3);}
;

explist : exp { $$ = make<ast::ExpressionListNode>(); $$->children.push_back($1); }
| explist COMMA exp { $$ = $1; $$->children.push_back($3); }
;

exp : NIL { $$ = make<ast::NilNode>(); }
| FALSE { $$ = make<ast::BoolNode>(false); }
| TRUE { $$ = make<ast::BoolNode>(true); }
| Number { $$ = $1; }
| String { $$ = $1; }
| VARIABLE_LIST { $$ = make<ast::ValueNode>($1); }
| function { $$ = $1; }
| prefixexp { $$ = $1; }
| tableconstructor { $$ = $1; }
| expBinOPexp { $$ = $1; }
| unopExp { $$ = $1; }
;

expBinOPexp: exp PLUS exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp MINUS exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp MUL exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp DIV exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp POW exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp MOD exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp DOTDOT exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp LESS exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp LESSOREQUAL exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp MORE exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp MOREOREQUAL exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp EQUAL exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp NOTEQUAL exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp AND exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| exp OR exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
;

unopExp : MINUS exp { $$ = make<ast::UnOPNode>($1, $2); }
| NOT exp { $$ = make<ast::UnOPNode>($1, $2); }
| POUND exp { $$ = make<ast::UnOPNode>($1, $2); }
;

prefixexp : var { $$ = $1; }
| functioncall { $$ = $1; }
| PARENTHESIS_OPEN exp PARENTHESIS_CLOSE { $$ = $2; }
;

functioncall : prefixexp args { $$ = make<ast::FunctionCallNode>($1, $2); }
| prefixexp COLON Name args {
		$4->children.insert($4->children.begin(), $1);
		$$ = make<ast::FunctionCallNode>(make<ast::IndexOfNode>($1, $3), $4);
 }
;

args : PARENTHESIS_OPEN PARENTHESIS_CLOSE { $$ = make<ast::ExpressionListNode>(); }
| PARENTHESIS_OPEN explist PARENTHESIS_CLOSE { $$ = $2; }
| tableconstructor { $$ = make<ast::ExpressionListNode>(); $$->children.push_back($1); }
| String { $$ = make<ast::ExpressionListNode>(); $$->children.push_back($1); }
;

function : FUNCTION funcbody { $$ = $2; }
;

funcbody : PARENTHESIS_OPEN PARENTHESIS_CLOSE block END { $$ = make<ast::FunctionNode>(make<ast::ValueNode>(ast::token::NilToken()), $3); }
| PARENTHESIS_OPEN parlist PARENTHESIS_CLOSE block END { $$ = make<ast::FunctionNode>($2, $4); }
;

parlist : namelist { $$ = $1; }
| namelist COMMA VARIABLE_LIST { $$ = $1; $$->children.push_back(make<ast::ValueNode>($3)); }
| VARIABLE_LIST { $$ = make<ast::NameListNode>(); $$->children.push_back(make<ast::ValueNode>($1)); }
;

tableconstructor : LIST_OPEN LIST_CLOSE { $$ = make<ast::TableNode>(); }
| LIST_OPEN fieldlist LIST_CLOSE { $$ = $2; }
;

fieldlistParts : field { $$ = make<ast::TableNode>(); $$->children.push_back($1); }
| fieldlistParts fieldsep field { $$ = $1; $$->children.push_back($3); }
;

fieldlist : fieldlistParts { $$ = $1; }
| fieldlistParts fieldsep { $$ = $1; }
;

field : exp { $$ = $1; }
;

fieldsep : COMMA {}
| SEMICOLON {}
;

String : QUOTED { $$ = make<ast::StringNode>($1); }
;
Name : NAME { $$ = make<ast::VariableRefNode>($1); }
;
Number : NUMBER { $$ = make<ast::NumberNode>($1); }
;
