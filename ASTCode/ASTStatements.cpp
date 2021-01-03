#include "clrpch.hpp"

#include "ASTStatements.hpp"
#include "ASTUtils.hpp"
#include "AST.hpp" //for ASTVarDecl

#include <sstream>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			ASTAssignmentStatement::ASTAssignmentStatement(std::shared_ptr<ASTExpression> lhs, std::shared_ptr<ASTExpression> rhs) : ASTStatement(),
				m_lhs(lhs), m_rhs(rhs)
			{
			}

			std::string ASTAssignmentStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_lhs->PrettyPrint() << " = " << m_rhs ->PrettyPrint();
				return ss.str();
			}

			void ASTAssignmentStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				cg->Emit(Opcode::Assign);

				m_lhs->EmitCode(cf, cg, func, cd);
				m_rhs->EmitCode(cf, cg, func, cd);
			}

			ASTStmtVarDecl::ASTStmtVarDecl(std::shared_ptr<ASTVarDecl> varDecl) : ASTStatement(),
				m_varDecl(varDecl)
			{
			}

			std::string ASTStmtVarDecl::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_varDecl->PrettyPrint() << ';';
				return ss.str();
			}

			void ASTStmtVarDecl::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				auto var = gcnew Definitions::LocalDefinition();
				var->Name = gcnew String(m_varDecl->GetIdent().c_str());
				var->Type = m_varDecl->GetType()->GetNativeType(cf);
				var->Parent = func;

				func->Locals->Add(var);
			}

			ASTOperatorAssignmentStatement::ASTOperatorAssignmentStatement(std::shared_ptr<ASTExpression> lhs, Operator op, std::shared_ptr<ASTExpression> rhs) : ASTStatement(),
				m_lhs(lhs), m_op(op), m_rhs(rhs)
			{
			}

			std::string ASTOperatorAssignmentStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_lhs->PrettyPrint() << ' ' << ASTOperatorToString(m_op) << "= " << m_rhs->PrettyPrint() << ';';
				return ss.str();
			}

			void ASTOperatorAssignmentStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				ASTStatement::EmitCode(cf, cg, func, cd);
			}

			std::shared_ptr<ASTStatement> ASTOperatorAssignmentStatement::Simplify()
			{
				switch(m_op)
				{
					case Operator::OP_MUL:
					{
						std::vector<std::shared_ptr<ASTExpression>> funcArgs = {m_lhs, m_rhs};
						auto opCall = std::make_shared<ASTFunctionCall>(std::make_shared<ASTIdentifierExpression>("OperatorAssignMultiply"), funcArgs);
						return opCall;
					}
					break;
				}

				return ASTStatement::Simplify();
			}

			ReturnStatement::ReturnStatement(std::shared_ptr<ASTExpression> expr) : ASTStatement(),
				m_expr(expr)
			{
			}

			std::string ReturnStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << "return " << m_expr->PrettyPrint() << ';';
				return ss.str();
			}

			void ReturnStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				cg->Emit(Opcode::Return);
				m_expr->EmitCode(cf, cg, func, cd);
			}

			IfStatement::IfStatement(std::shared_ptr<ASTExpression> condition, std::vector<std::shared_ptr<ASTStatement>> trueStatements, std::vector<std::shared_ptr<ASTStatement>> falseStatements) : ASTStatement(),
				m_condition(condition), m_trueStatements(trueStatements), m_falseStatements(falseStatements)
			{
			}

			std::string IfStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << "if(" << m_condition->PrettyPrint() << ")\n{\n";
				for(const auto &stmt : m_trueStatements)
				{
					ss << stmt->PrettyPrint() << '\n';
				}
				ss << "}\nelse\n{\n";
				for(const auto &stmt : m_falseStatements)
				{
					ss << stmt->PrettyPrint() << '\n';
				}
				ss << '}';
				return ss.str();
			}

			void IfStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				ASTStatement::EmitCode(cf, cg, func, cd);
			}

			GotoStatement::GotoStatement(const std::string &labelName) : ASTStatement(),
				m_label(labelName)
			{
			}

			std::string GotoStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << "goto " << m_label << ';';
				return ss.str();
			}

			void GotoStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				ASTStatement::EmitCode(cf, cg, func, cd);
			}

			std::string GotoStatement::GetLabel()
			{
				return m_label;
			}

			JumpZeroStatement::JumpZeroStatement(const std::string &labelName, std::shared_ptr<ASTExpression> expr) : ASTStatement(),
				m_label(labelName), m_expr(expr)
			{
			}

			std::string JumpZeroStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << "__jz(" << m_label << ", " << m_expr->PrettyPrint() << ");";
				return ss.str();
			}

			void JumpZeroStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				ASTStatement::EmitCode(cf, cg, func, cd);
			}

			std::string JumpZeroStatement::GetLabel()
			{
				return m_label;
			}

			std::shared_ptr<ASTExpression> JumpZeroStatement::GetCondition()
			{
				return m_expr;
			}

			LabelStatement::LabelStatement(const std::string &labelName) : ASTStatement(),
				m_label(labelName)
			{
			}

			std::string LabelStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_label << ":";
				return ss.str();
			}

			void LabelStatement::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				ASTStatement::EmitCode(cf, cg, func, cd);
			}

			std::string LabelStatement::GetLabel()
			{
				return m_label;
			}

			WhileStatement::WhileStatement(std::shared_ptr<ASTExpression> condition, std::vector<std::shared_ptr<ASTStatement>> loopStatements) : ASTStatement(),
				m_condition(condition), m_loopStatements(loopStatements)
			{
			}

			std::string WhileStatement::PrettyPrint()
			{
				std::stringstream ss;
				ss << "while(" << m_condition->PrettyPrint() << ")\n{\n";
				for(auto &stmt : m_loopStatements)
				{
					ss << stmt->PrettyPrint() << '\n';
				}
				ss << '}';
				return ss.str();
			}
		}
	}
}