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
%token <ast::token::NameToken> NAME
%token <ast::token::NumberToken> NUMBER
%token EndOfFile 0 "end of file"


%type <std::shared_ptr<ast::StringNode>> String
%type <std::shared_ptr<ast::VariableRefNode>> Name
%type <std::shared_ptr<ast::NumberNode>> Number
%type <ast::token::BinOPToken> Binop
%type <ast::token::UnOPToken> Unop

%type <std::shared_ptr<ast::RootNode>> root
%type <ast::NodePtr> block stat laststat lastChunkStatement chunkStatement funcnamePart funcname varlist var namelist explist exp prefixexp functioncall args function funcbody parlist tableconstructor fieldlistParts fieldlist field fieldsep
%type <std::shared_ptr<ast::ChunkNode>> chunk
%type <std::vector<ast::NodePtr>> chunkStatements

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

stat : varlist EQUALS explist { $$ = make<ast::AssignValueNode>($1, $3); }
| functioncall { $$ = $1; }
| FUNCTION funcname funcbody { $$ = make<ast::AssignValueNode>($2, $3); }
| LOCAL FUNCTION Name funcbody {
		auto cn = make<ast::ChunkNode>();
		cn->children.push_back(make<ast::LocalAssignValueNode>($3, make<ast::ValueNode>(ast::token::NilToken())));
		cn->children.push_back(make<ast::LocalAssignValueNode>($3, $4));
		cn->addScope = false;
		$$ = cn;
 }
| LOCAL namelist { $$ = make<ast::LocalAssignValueNode>($2, make<ast::ValueNode>(ast::token::NilToken())); }
| LOCAL namelist EQUALS explist { $$ = make<ast::LocalAssignValueNode>($2, $4); }
;

laststat : RETURN { $$ = make<ast::ReturnNode>(); }
| RETURN explist { $$ = make<ast::ReturnNode>($2); }
| BREAK { $$ = make<ast::BreakNode>(); }
;

funcnamePart : Name { $$ = make<ast::VariableRefNode>($1); }
| funcnamePart DOT Name { $$ = make<ast::IndexOfNode>($1, $3); }
;

funcname : funcnamePart { $$ = make<ast::VariableRefNode>($1); }
;

varlist : var { $$ = make<ast::VariableRefListNode>(); $$->children.push_back($1); }
| varlist COMMA var { $$ = $1; $$->children.push_back($3); }
;

var : Name { $$ = $1; }
| prefixexp SQUARE_OPEN exp SQUARE_CLOSE { $$ = make<ast::IndexOfNode>($1, $3);}
| prefixexp DOT Name { $$ = make<ast::IndexOfNode>($1, $3); }
;

namelist : Name { $$ = make<ast::VariableRefListNode>(); $$->children.push_back($1); }
| namelist COMMA Name { $$ = $1; $$->children.push_back($3);}
;

explist : exp { $$ = make<ast::VariableRefListNode>(); $$->children.push_back($1); }
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
| exp Binop exp { $$ = make<ast::BinOPNode>($1, $2, $3); }
| Unop exp { $$ = make<ast::UnOPNode>($1, $2); }
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

args : PARENTHESIS_OPEN PARENTHESIS_CLOSE { $$ = make<ast::VariableRefListNode>(); }
| PARENTHESIS_OPEN explist PARENTHESIS_CLOSE { $$ = $2; }
| tableconstructor { $$ = $1; }
| String { $$ = $1; }
;

function : FUNCTION funcbody { $$ = $2; }
;

funcbody : PARENTHESIS_OPEN PARENTHESIS_CLOSE block END { $$ = make<ast::FunctionNode>(make<ast::ValueNode>(ast::token::NilToken()), $3); }
| PARENTHESIS_OPEN parlist PARENTHESIS_CLOSE block END { $$ = make<ast::FunctionNode>($2, $4); }
;

parlist : namelist { $$ = $1; }
| namelist COMMA VARIABLE_LIST { $$ = $1; $$->children.push_back(make<ast::ValueNode>($3)); }
| VARIABLE_LIST { $$ = make<ast::VariableRefListNode>(); $$->children.push_back(make<ast::ValueNode>($1)); }
;

tableconstructor : LIST_OPEN LIST_CLOSE { $$ = make<ast::TableNode>(); }
| LIST_OPEN fieldlist LIST_CLOSE { $$ = make<ast::TableNode>($2); }
;

fieldlistParts : field { $$ = make<ast::VariableRefListNode>(); $$->children.push_back($1); }
| fieldlistParts fieldsep field { $$ = $1; $$->children.push_back($3); }
;

fieldlist : fieldlistParts { $$ = $1; }
| fieldlistParts fieldsep { $$ = $1; }
;

field : SQUARE_OPEN exp SQUARE_CLOSE EQUALS exp { $$ = make<ast::AssignValueNode>(make<ast::IndexOfNode>(make<ast::ValueNode>(ast::token::NilToken()), $2), $5); }
| Name EQUALS exp { $$ = make<ast::AssignValueNode>($1, $3); }
| exp { $$ = $1; }
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

Binop : BINOP { $$ = $1; }
| MINUSOP { $$ = {ast::token::BinOPToken::OP::minus}; }
;
Unop : UNOP { $$ = $1; }
| MINUSOP { $$ = {ast::token::UnOPToken::OP::minus}; }
;
