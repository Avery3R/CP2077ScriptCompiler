#include "clrpch.hpp"

#include "ASTBasicExpressions.hpp"
#include "ASTUtils.hpp"

#include <sstream>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			ASTExpressionInteger::ASTExpressionInteger(int64_t num, ASTExpressionInteger::Bits bits) : ASTExpression(),
				m_num(num), m_bits(bits)
			{

			}

			std::string ASTExpressionInteger::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_num;
				return ss.str();
			}

			void ASTExpressionInteger::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				switch(m_bits)
				{
					case ASTExpressionInteger::Bits::BITS_8:
						cg->Emit(Opcode::Int8Const, m_num);
					break;
					case ASTExpressionInteger::Bits::BITS_16:
						cg->Emit(Opcode::Int16Const, m_num);
					break;
					case ASTExpressionInteger::Bits::BITS_32:
						cg->Emit(Opcode::Int32Const, m_num);
					break;
					case ASTExpressionInteger::Bits::BITS_64:
						cg->Emit(Opcode::Int64Const, m_num);
					break;
				}
			}

			int64_t ASTExpressionInteger::GetNum()
			{
				return m_num;
			}

			ASTExpressionInteger::Bits ASTExpressionInteger::GetBits()
			{
				return m_bits;
			}

			ASTExpressionInteger::Bits ASTExpressionInteger::GetBiggerBits(Bits a, Bits b)
			{
				Bits ret;
				ret = Bits::BITS_8;
	
				if(a == Bits::BITS_16 || b == Bits::BITS_16)
				{
					ret = Bits::BITS_16;
				}

				if(a == Bits::BITS_32 || b == Bits::BITS_32)
				{
					ret = Bits::BITS_32;
				}

				if(a == Bits::BITS_64 || b == Bits::BITS_64)
				{
					ret = Bits::BITS_64;
				}

				return ret;
			}

			ASTNullExpression::ASTNullExpression() : ASTExpression()
			{
			}

			std::string ASTNullExpression::PrettyPrint()
			{
				return "null";
			}

			void ASTNullExpression::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				cg->Emit(Opcode::Nop);
			}

			GENERIC_DEF ASTNullExpression::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				return nullptr;
			}

			ASTExpressionFloat::ASTExpressionFloat(float num) : ASTExpression(),
				m_num(num)
			{
			}

			std::string ASTExpressionFloat::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_num;
				return ss.str();
			}

			void ASTExpressionFloat::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				cg->Emit(Opcode::FloatConst, m_num);
			}

			GENERIC_DEF ASTExpressionFloat::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				return GetFromCache<Definitions::NativeDefinition>(cf, "Float");
			}

			float ASTExpressionFloat::GetNum()
			{
				return m_num;
			}

			ASTThisExpression::ASTThisExpression() : ASTExpression()
			{
			}

			std::string ASTThisExpression::PrettyPrint()
			{
				return "this";
			}

			GENERIC_DEF_LIST ASTThisExpression::ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto ret = gcnew System::Collections::Generic::List<Definition^>;
				ret->Add(cd);
				return ret;
			}

			GENERIC_DEF ASTThisExpression::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				return cd;
			}

			NameExpression::NameExpression(const std::string &name) : ASTExpression(),
				m_name(name)
			{
			}

			std::string NameExpression::PrettyPrint()
			{
				std::stringstream ss;
				ss << "n\"" << m_name << '\"';
				return ss.str();
			}
		}
	}
}