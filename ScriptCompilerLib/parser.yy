%language "c++"

%define api.value.type variant
%define api.token.constructor
%define parse.error verbose
//%no-lines
//%define parse.trace

%locations

%code top
{
//#define YYDEBUG 1
#pragma warning(disable: 4065)
}

%code requires
{
#include <iostream>
#include <cstdint>
#include <utility>
	#include "AST.hpp"

	using namespace Avery3R::RED4Scripts;
	class ParserDriver;
}

%code
{
#include "driver.hpp"

	yy::parser::symbol_type yylex(ParserDriver &driver);
}

%param {ParserDriver &driver}

%%

%token TOKEN_VAR;
%token TOKEN_REF;
%token TOKEN_NEW;
%token TOKEN_NULL;

%token TOKEN_FUNCTION;
%token TOKEN_EXEC;
%token TOKEN_EVENT;

%token TOKEN_CLASS;
%token TOKEN_IMPORT;

%token TOKEN_PUBLIC;
%token TOKEN_PROTECTED;
%token TOKEN_PRIVATE;
%token TOKEN_INJECT;

%token TOKEN_STATIC;

%token TOKEN_NATIVE;

%token TOKEN_OP_ASSIGN_MULT;

%token TOKEN_THIS;

%token <int64_t> TOKEN_INT64;
%token <int32_t> TOKEN_INT32;
%token <int16_t> TOKEN_INT16;
%token <int8_t> TOKEN_INT8;
%token <float> TOKEN_FLOAT;
%token <std::string> TOKEN_IDENTIFIER;

%type <std::shared_ptr<AST::ASTExpression>> expression;
%type <std::vector<std::shared_ptr<AST::ASTExpression>>> expression_list;
%type <std::shared_ptr<AST::ASTStatement>> statement;
%type <std::vector<std::shared_ptr<AST::ASTStatement>>> statement_list;
%type <std::shared_ptr<AST::ASTType>> type;
%type <AST::ASTFunction::FunctionFlags> function_flags;
%type <AST::ASTFunction::FunctionFlags> function_type;
%type <std::vector<std::pair<std::shared_ptr<AST::ASTType>, std::string>>> function_params;
%type <std::shared_ptr<AST::ASTFunction>> function;
//%type <std::shared_ptr<AST::ASTMethodCall>> method_call;
%type <std::shared_ptr<AST::ASTFunctionCall>> function_call;
%type <std::shared_ptr<AST::ASTAccess>> access;
%type <std::shared_ptr<AST::ASTImport>> import;
%type <AST::Visibility> visibility;
%type <std::shared_ptr<AST::ASTVarDecl>> var_decl;
%type <std::shared_ptr<AST::ASTClass>> class;
%type <std::shared_ptr<AST::ASTClassBody>> class_body;
%type <AST::ASTClassBody::PropertyFlag> class_property_flag;

//%precedence NUMBER

file :
	%empty
	| file function
	{
		driver.m_functions.push_back($2);
	}
	| file import
	{
		driver.m_imports.push_back($2);
	}
	| file class
	{
		driver.m_classes.push_back($2);
	}
;

import :
	TOKEN_IMPORT TOKEN_CLASS TOKEN_IDENTIFIER ';'
	{
		$$ = std::make_shared<AST::ASTClassImport>($3);
	}
;

class:
	visibility TOKEN_CLASS TOKEN_IDENTIFIER '{' class_body '}'
	{
		$$ = std::make_shared<AST::ASTClass>($1, $3, $5);
	}
;

class_property_flag:
	%empty
	{
		$$ = (AST::ASTClassBody::PropertyFlag)0;
	}
	| TOKEN_NATIVE
	{
		$$ = AST::ASTClassBody::PF_IsNative;
	}
;

class_body:
	%empty
	{
		$$ = std::make_shared<AST::ASTClassBody>();
	}
	| class_body visibility class_property_flag var_decl ';'
	{
		$$ = $1;
		$$->AddVarDecl($2, $3, $4);
	}
	| class_body visibility function
	{
		$$ = $1;
		$$->AddFunc($2, $3);
	}
;

function :
	function_flags function_type TOKEN_IDENTIFIER '(' function_params ')' '{' statement_list '}'
	{
		$$ = std::make_shared<AST::ASTFunction>($3, AST::ASTFunction::FunctionFlags($1 | $2), $5, $8);
	}
	| function_flags function_type TOKEN_IDENTIFIER '(' function_params ')' ':' type '{' statement_list '}'
	{
		$$ = std::make_shared<AST::ASTFunction>($3, AST::ASTFunction::FunctionFlags($1 | $2), $5, $10);
	}
;

function_type:
	TOKEN_FUNCTION
	{
		$$ = AST::ASTFunction::FF_None;
	}
	| TOKEN_EXEC
	{
		$$ = AST::ASTFunction::FF_IsExec;
	}
	| TOKEN_EVENT
	{
		$$ = AST::ASTFunction::FF_IsEvent;
	}
;

function_params :
	%empty
	{
	}
	| function_params ',' TOKEN_IDENTIFIER ':' type
	{
		$$ = $1;
		$$.push_back(std::make_pair($5, $3));
	}
	| TOKEN_IDENTIFIER ':' type
	{
		$$.push_back(std::make_pair($3, $1));
	}
;

function_flags:
	%empty
	{
		$$ = AST::ASTFunction::FF_None;
	}
	| function_flags TOKEN_STATIC
	{
		$$ = AST::ASTFunction::FunctionFlags($1 | AST::ASTFunction::FF_IsStatic);
	}
;

visibility:
	%empty
	{
		$$ = AST::Visibility::PUBLIC;
	}
	| TOKEN_PUBLIC
	{
		$$ = AST::Visibility::PUBLIC;
	}
	| TOKEN_PROTECTED
	{
		$$ = AST::Visibility::PROTECTED;
	}
	| TOKEN_PRIVATE
	{
		$$ = AST::Visibility::PRIVATE;
	}
	| TOKEN_INJECT
	{
		$$ = AST::Visibility::INJECT;
	}
;

statement_list :
	statement
	{
		//driver.m_statements.push_back($1);
		$$.push_back($1);
	}
	| statement_list statement
	{
		$1.push_back($2);
		$$ = $1;
	}
;

var_decl:
	TOKEN_VAR TOKEN_IDENTIFIER ':' type
	{
		$$ = std::make_shared<AST::ASTVarDecl>($2, $4);
	}
;

statement :
	/*TOKEN_IDENTIFIER '=' expression ';'
	{
		//std::cout << "Hit assignment statement" << std::endl;
		$$ = std::make_shared<AST::ASTAssignmentStatement>($1, $3);
	}*/
	expression '=' expression ';'
	{
		//std::cout << "Hit assignment statement" << std::endl;
		$$ = std::make_shared<AST::ASTAssignmentStatement>($1, $3);
	}
	| var_decl ';'
	{
		//std::cout << "Hit declaration statement" << std::endl;
		$$ = std::make_shared<AST::ASTStmtVarDecl>($1);
	}
	| function_call ';'
	{
		$$ = $1;
	}
	/*| method_call ';'
	{
		$$ = $1;
	}*/
	| expression TOKEN_OP_ASSIGN_MULT expression ';'
	{
		$$ = std::make_shared<AST::ASTOperatorAssignmentStatement>($1, AST::Operator::OP_MUL, $3);
	}
;

expression_list :
	%empty
	{
	}
	| expression
	{
		//driver.m_statements.push_back($1);
		$$.push_back($1);
	}
	| expression_list ',' expression
	{
		$$ = $1;
		$$.push_back($3);
	}
;

expression :
	TOKEN_INT64
	{
		//std::cout << "Hit expression NUMBER" << std::endl;
		$$ = std::make_shared<AST::ASTExpressionInteger>($1, AST::ASTExpressionInteger::Bits::BITS_64);
	}
	| TOKEN_INT32
	{
		//std::cout << "Hit expression NUMBER" << std::endl;
		$$ = std::make_shared<AST::ASTExpressionInteger>($1, AST::ASTExpressionInteger::Bits::BITS_32);
	}
	| TOKEN_INT16
	{
		//std::cout << "Hit expression NUMBER" << std::endl;
		$$ = std::make_shared<AST::ASTExpressionInteger>($1, AST::ASTExpressionInteger::Bits::BITS_16);
	}
	| TOKEN_INT8
	{
		//std::cout << "Hit expression NUMBER" << std::endl;
		$$ = std::make_shared<AST::ASTExpressionInteger>($1, AST::ASTExpressionInteger::Bits::BITS_8);
	}
	| TOKEN_FLOAT
	{
		$$ = std::make_shared<AST::ASTExpressionFloat>($1);
	}
	| TOKEN_NEW TOKEN_IDENTIFIER
	{
		$$ = std::make_shared<AST::ASTNewExpression>($2);
	}
	| TOKEN_IDENTIFIER
	{
		$$ = std::make_shared<AST::ASTIdentifierExpression>($1);
	}
	| TOKEN_NULL
	{
		$$ = std::make_shared<AST::ASTNullExpression>();
	}
	| TOKEN_THIS
	{
		$$ = std::make_shared<AST::ASTThisExpression>();
	}
	| function_call
	{
		$$ = $1;
	}
	/*| method_call
	{
		$$ = $1;
	}*/
	| access
	{
		$$ = $1;
	}
;

access :
	expression '.' TOKEN_IDENTIFIER
	{
		$$ = std::make_shared<AST::ASTAccess>($1, $3);
	}
;

/*method_call :
	TOKEN_IDENTIFIER '.' TOKEN_IDENTIFIER '(' expression_list ')'
	{
		$$ = std::make_shared<AST::ASTMethodCall>($1, $3, $5);
	}
;*/

function_call :
	expression '(' expression_list ')'
	{
		$$ = std::make_shared<AST::ASTFunctionCall>($1, $3);
	}
;

type :
	TOKEN_REF '<' TOKEN_IDENTIFIER '>'
	{
		$$ = std::make_shared<AST::ASTType>($3, true);
	}
	| TOKEN_IDENTIFIER
	{
		$$ = std::make_shared<AST::ASTType>($1, false);
	}
;

%%

/*void example::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}*/

void yy::parser::error(const yy::parser::location_type &loc, const std::string &msg)
{
	std::cout << "Error at location " << loc.begin.line << ',' << loc.begin.column << ':' << msg << std::endl;
}