#include "clrpch.hpp"

#include "ASTBaseTypes.hpp"

#include <stdexcept>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			void ASTEmittable::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				throw std::logic_error("EmitCode() Unimplemented");
			}

			GENERIC_DEF_LIST ASTResolvable::ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				ASTPrintable *test = dynamic_cast<ASTPrintable*>(this);
				if(test)
				{
					throw std::logic_error(std::string("ResolveDef() Unimplemented: ") + test->PrettyPrint() + typeid(*this).name());
				}
				else
				{
					throw std::logic_error(std::string("ResolveDef() Unimplemented ") + typeid(*this).name());
				}
			}

			ASTExpression::ASTExpression() : ASTPrintable(), ASTEmittable()
			{
			}

			std::shared_ptr<ASTExpression> ASTExpression::Simplify()
			{
				return shared_from_this();
			}

			GENERIC_DEF ASTExpression::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				throw std::logic_error("GetTypeDef() Unimplemented");
			}

			ASTStatement::ASTStatement() : std::enable_shared_from_this<ASTStatement>(), ASTPrintable(), ASTEmittable()
			{
			}

			std::shared_ptr<ASTStatement> ASTStatement::Simplify()
			{
				return shared_from_this();
			}
		}
	}
}