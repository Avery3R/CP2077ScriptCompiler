#pragma once

#include "ASTBaseTypes.hpp"

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			class ASTExpressionInteger : public ASTExpression
			{
				public:
					enum class Bits
					{
						BITS_64,
						BITS_32,
						BITS_16,
						BITS_8
					};

					ASTExpressionInteger(int64_t num, Bits bits);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					int64_t GetNum();
					Bits GetBits();

					static Bits GetBiggerBits(Bits a, Bits b);
				private:
					int64_t m_num;
					Bits m_bits;
			};

			class ASTNullExpression : public ASTExpression
			{
				public:
					ASTNullExpression();

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;
			};

			class ASTExpressionFloat : public ASTExpression
			{
				public:
					ASTExpressionFloat(float num);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;

					float GetNum();
				private:
					float m_num;
			};

			class ASTThisExpression : public ASTExpression
			{
				public:
					ASTThisExpression();

					std::string PrettyPrint() override;

					GENERIC_DEF_LIST ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;
					GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;
			};

			class NameExpression : public ASTExpression
			{
				public:
					NameExpression(const std::string &name);

					std::string PrettyPrint() override;
				
				private:
					std::string m_name;
			};
		}
	}
}