#pragma once

#include "ASTNetTypes.hpp"
#include <string>
#include <memory>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			class ASTPrintable
			{
				public:
					virtual std::string PrettyPrint() = 0;
			};

			class ASTEmittable
			{
				public:
					virtual void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd);
			};

			class ASTResolvable
			{
				public:
					virtual GENERIC_DEF_LIST ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd);
			};

			class ASTImport : public ASTPrintable
			{
				public:
					virtual GENERIC_DEF Emit(CACHE_FILE cf) = 0;
			};

			class ASTExpression : public std::enable_shared_from_this<ASTExpression>, public ASTPrintable, public ASTEmittable, public ASTResolvable
			{
				public:
					virtual std::shared_ptr<ASTExpression> Simplify();
					virtual GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd);
				protected:
					ASTExpression();
			};

			class ASTStatement : public std::enable_shared_from_this<ASTStatement>, public ASTPrintable, public ASTEmittable
			{
				public:
					virtual std::shared_ptr<ASTStatement> Simplify();
				protected:
					ASTStatement();
			};
		}
	}
}