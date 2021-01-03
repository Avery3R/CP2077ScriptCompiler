%language "c++"

%define api.value.type variant
%define api.token.constructor
%define parse.error verbose
//%no-lines
%define parse.trace

%code top
{
#define YYDEBUG 1
#pragma warning(disable: 4065)
}

%code requires
{
	struct ParserDriver;

	#include <AST.hpp>
	using namespace Avery3R::RED4Scripts;
}

%code
{
	#include "driver.hpp"

	yy::parser::symbol_type yylex(ParserDriver &driver);
}

%param {ParserDriver &driver}

%%

%token OP_NOP;
%token OP_CONTEXT;

%token <std::pair<std::string, std::string>> OP_FINAL_FUNC;
%token OP_PARAM_END;

%token OP_ASSIGN;
%token OP_RETURN;
%token OP_INT32_ZERO;
%token OP_INT32_ONE;
%token OP_ARRAY_PUSH_BACK;
%token OP_ARRAY_ELEMENT;
%token OP_ARRAY_SIZE;
%token OP_TEST_NOT_EQUAL;

%token <std::string> OP_JUMP;
%token <std::string> OP_JUMP_IF_FALSE;

%token <std::string> OP_STRUCT_MEMBER;

%token <std::string> OP_CONSTRUCTOR;

%token <std::string> OP_NAME_CONST;

%token <std::string> TOK_IDENTIFIER;
%token <std::string> TOK_LABEL;
%token TOK_CTOR_PARAMEND;

%type <std::shared_ptr<AST::ASTFunctionCall>> function_call;

%type <std::shared_ptr<AST::ASTExpression>> expression;
%type <std::vector<std::shared_ptr<AST::ASTExpression>>> expression_list;
%type <std::vector<std::shared_ptr<AST::ASTExpression>>> ctor_expression_list;
%type <std::shared_ptr<AST::ASTStatement>> statement;
%type <std::vector<std::shared_ptr<AST::ASTStatement>>> statement_list;

function:
	statement_list
	{
		driver.m_statements = $1;
	}
;

statement_list:
	statement
	{
		if($1)
		{
			$$.push_back($1);
		}
	}
	| statement_list statement
	{
		$$ = $1;
		if($2)
		{
			$$.push_back($2);
		}
	}
;

statement:
	function_call
	{
		$$ = $1;
	}
	| OP_RETURN expression
	{
		$$ = std::make_shared<AST::ReturnStatement>($2);
	}
	| OP_JUMP
	{
		$$ = std::make_shared<AST::GotoStatement>($1);
	}
	| OP_JUMP_IF_FALSE expression
	{
		$$ = std::make_shared<AST::JumpZeroStatement>($1, $2);
	}
	| OP_ASSIGN expression expression
	{
		$$ = std::make_shared<AST::ASTAssignmentStatement>($2, $3);
	}
	| OP_ARRAY_PUSH_BACK expression expression
	{
		$$ = std::make_shared<AST::ASTOperatorAssignmentStatement>($2, AST::Operator::OP_ADD, $3);
	}
	| OP_NOP
	{
	}
	| TOK_LABEL
	{
		$$ = std::make_shared<AST::LabelStatement>($1);
	}
	//OP_ASSIGN expression expression
	//| function_call
	//| OP_RETURN expression
;

function_call:
	OP_FINAL_FUNC expression_list OP_PARAM_END
	{
		if($1.first.empty())
		{
			$$ = std::make_shared<AST::ASTFunctionCall>(std::make_shared<AST::ASTIdentifierExpression>($1.second), $2);
		}
		else
		{
			$$ = std::make_shared<AST::ASTFunctionCall>(std::make_shared<AST::ASTAccess>(std::make_shared<AST::ASTIdentifierExpression>($1.first), $1.second), $2);
		}
	}
	| OP_CONTEXT expression OP_FINAL_FUNC expression_list OP_PARAM_END
	{
		$$ = std::make_shared<AST::ASTFunctionCall>(std::make_shared<AST::ASTAccess>($2, $3.second), $4);
	}
;

expression_list :
	%empty
	{
	}
	| expression_list expression
	{
		$$ = $1;
		$$.push_back($2);
	}
;

ctor_expression_list :
	%empty
	{
	}
	| expression_list expression
	{
		$$ = $1;
		$$.push_back($2);
		driver.NotifyParsedCtorExpression();
	}
;

expression:
	OP_STRUCT_MEMBER expression
	{
		$$ = std::make_shared<AST::ASTAccess>($2, $1);
	}
	| TOK_IDENTIFIER
	{
		$$ = std::make_shared<AST::ASTIdentifierExpression>($1);
	}
	| function_call
	{
		$$ = $1;
	}
	| OP_CONSTRUCTOR ctor_expression_list TOK_CTOR_PARAMEND
	{
		$$ = std::make_shared<AST::ConstructExpression>($1, $2);
	}
	| OP_INT32_ZERO
	{
		$$ = std::make_shared<AST::ASTExpressionInteger>(0, AST::ASTExpressionInteger::Bits::BITS_32);
	}
	| OP_INT32_ONE
	{
		$$ = std::make_shared<AST::ASTExpressionInteger>(1, AST::ASTExpressionInteger::Bits::BITS_32);
	}
	| OP_ARRAY_ELEMENT expression expression
	{
		$$ = std::make_shared<AST::ArrayAccessExpression>($2, $3);
	}
	| OP_ARRAY_SIZE expression
	{
		$$ = std::make_shared<AST::ArrayLenExpression>($2);
	}
	| OP_TEST_NOT_EQUAL expression expression
	{
		$$ = std::make_shared<AST::BinaryExpression>($2, AST::Operator::OP_NE, $3);
	}
	| OP_NAME_CONST
	{
		$$ = std::make_shared<AST::NameExpression>($1);
	}
;

/*void example::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}*/

%%

void yy::parser::error(const std::string &msg)
{
	std::cout << "Parsing error: " << msg << std::endl;
}