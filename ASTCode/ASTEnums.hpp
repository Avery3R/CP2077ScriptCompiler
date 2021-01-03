#pragma once

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			enum class Visibility
			{
				PUBLIC,
				PROTECTED,
				PRIVATE,
				INJECT
			};

			inline static const char* VisibilityToString(Visibility vis)
			{
				switch(vis)
				{
					case Visibility::PUBLIC:
						return "public";
					break;
					case Visibility::PROTECTED:
						return "protected";
					break;
					case Visibility::PRIVATE:
						return "private";
					break;
					case Visibility::INJECT:
						return "inject";
					break;
				}

				return "error_visibility";
			}

			enum class Operator
			{
				OP_MUL,
				OP_ADD,

				OP_NE,
			};

			inline static const char* ASTOperatorToString(Operator op)
			{
				switch(op)
				{
					case Operator::OP_MUL:
						return "*";
					case Operator::OP_ADD:
						return "+";
					case Operator::OP_NE:
						return "!=";
					break;
				}

				return "error_operator";
			}
		}
	}
}