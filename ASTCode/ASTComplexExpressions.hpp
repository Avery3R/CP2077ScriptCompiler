#pragma once

#include "ASTBaseTypes.hpp"
#include "ASTNetTypes.hpp"
#include "ASTEnums.hpp"
#include <vector>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			class ASTNewExpression : public ASTExpression
			{
				public:
					ASTNewExpression(const std::string &className);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

				private:
					std::string m_className;
			};

			class ASTIdentifierExpression : public ASTExpression
			{
				public:
					ASTIdentifierExpression(const std::string &ident);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;
					GENERIC_DEF_LIST ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;

				private:
					std::string m_ident;
			};

			class ASTAccess : public ASTExpression
			{
				public:
					ASTAccess(const std::shared_ptr<ASTExpression> &lhs, const std::string &rhs);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;
					GENERIC_DEF_LIST ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;
					GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;

					std::shared_ptr<ASTExpression> GetLhs();
		
				private:
					std::shared_ptr<ASTExpression> m_lhs;
					std::string m_rhs;
			};

			class ConstructExpression : public ASTExpression
			{
				public:
					ConstructExpression(const std::string &className, std::vector<std::shared_ptr<ASTExpression>> ctorArgs);

					std::string PrettyPrint() override;

				private:
					std::string m_className;
					std::vector<std::shared_ptr<ASTExpression>> m_ctorArgs;
			};

			class ArrayAccessExpression : public ASTExpression
			{
				public:
					ArrayAccessExpression(const std::shared_ptr<ASTExpression> &array, const std::shared_ptr<ASTExpression> &index);

					std::string PrettyPrint() override;

				private:
					std::shared_ptr<ASTExpression> m_array;
					std::shared_ptr<ASTExpression> m_index;
			};

			class ArrayLenExpression : public ASTExpression
			{
				public:
					ArrayLenExpression(const std::shared_ptr<ASTExpression> &array);

					std::string PrettyPrint() override;

				private:
					std::shared_ptr<ASTExpression> m_array;
			};

			class BinaryExpression : public ASTExpression
			{
				public:
					BinaryExpression(std::shared_ptr<ASTExpression> lhs, Operator op, std::shared_ptr<ASTExpression> rhs);

					std::string PrettyPrint() override;

				private:
					std::shared_ptr<ASTExpression> m_lhs;
					Operator m_op;
					std::shared_ptr<ASTExpression> m_rhs;
			};
		}
	}
}