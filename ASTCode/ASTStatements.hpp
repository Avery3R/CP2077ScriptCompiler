#pragma once

#include "ASTBaseTypes.hpp"
#include "ASTEnums.hpp"
#include "ASTNetTypes.hpp"

#include <vector>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			class ASTAssignmentStatement : public ASTStatement
			{
				public:
					ASTAssignmentStatement(std::shared_ptr<ASTExpression> lhs, std::shared_ptr<ASTExpression> rhs);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

				private:
					std::shared_ptr<ASTExpression> m_lhs;
					std::shared_ptr<ASTExpression> m_rhs;
			};

			class ASTVarDecl;

			class ASTStmtVarDecl : public ASTStatement
			{
				public:
					ASTStmtVarDecl(std::shared_ptr<ASTVarDecl> varDecl);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

				private:
					std::shared_ptr<ASTVarDecl> m_varDecl;
			};

			class ASTOperatorAssignmentStatement : public ASTStatement
			{
				public:
					ASTOperatorAssignmentStatement(std::shared_ptr<ASTExpression> lhs, Operator op, std::shared_ptr<ASTExpression> rhs);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					std::shared_ptr<ASTStatement> Simplify() override;

				private:
					std::shared_ptr<ASTExpression> m_lhs;
					Operator m_op;
					std::shared_ptr<ASTExpression> m_rhs;
			};

			class ReturnStatement : public ASTStatement
			{
				public:
					ReturnStatement(std::shared_ptr<ASTExpression> expr);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

				private:
					std::shared_ptr<ASTExpression> m_expr;
			};

			class IfStatement : public ASTStatement
			{
				public:
					IfStatement(std::shared_ptr<ASTExpression> condition, std::vector<std::shared_ptr<ASTStatement>> trueStatements, std::vector<std::shared_ptr<ASTStatement>> falseStatements);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

				private:
					std::shared_ptr<ASTExpression> m_condition;
					std::vector<std::shared_ptr<ASTStatement>> m_trueStatements;
					std::vector<std::shared_ptr<ASTStatement>> m_falseStatements;
			};

			class GotoStatement : public ASTStatement
			{
				public:
					GotoStatement(const std::string &labelName);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;
					
					std::string GetLabel();
				private:
					std::string m_label;
			};

			class JumpZeroStatement : public ASTStatement
			{
				public:
					JumpZeroStatement(const std::string &labelName, std::shared_ptr<ASTExpression> expr);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					std::string GetLabel();
					std::shared_ptr<ASTExpression> GetCondition();
				private:
					std::string m_label;
					std::shared_ptr<ASTExpression> m_expr;
			};

			class LabelStatement : public ASTStatement
			{
				public:
					LabelStatement(const std::string &labelName);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					std::string GetLabel();
				private:
					std::string m_label;
			};

			class WhileStatement : public ASTStatement
			{
				public:
					WhileStatement(std::shared_ptr<ASTExpression> condition, std::vector<std::shared_ptr<ASTStatement>> loopStatements);

					std::string PrettyPrint() override;

				private:
					std::shared_ptr<ASTExpression> m_condition;
					std::vector<std::shared_ptr<ASTStatement>> m_loopStatements;
			};
		}
	}
}