#include "clrpch.hpp"

#include "ASTComplexExpressions.hpp"
#include "ASTUtils.hpp"

#include <sstream>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			ASTNewExpression::ASTNewExpression(const std::string &className) : ASTExpression(),
				m_className(className)
			{

			}

			std::string ASTNewExpression::PrettyPrint()
			{
				std::stringstream ss;
				ss << "new " << m_className;

				return ss.str();
			}

			void ASTNewExpression::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				Definitions::ClassDefinition ^classDef = GetFromCache<Definitions::ClassDefinition>(cf, m_className);

				if(!classDef)
				{
					throw std::runtime_error(std::string("Could not find class: ")+m_className);
				}

				cg->Emit(Opcode::New, classDef);
			}

			ASTIdentifierExpression::ASTIdentifierExpression(const std::string &ident) : ASTExpression(),
				m_ident(ident)
			{
			}

			std::string ASTIdentifierExpression::PrettyPrint()
			{
				return m_ident;
			}

			void ASTIdentifierExpression::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				auto identDef = GetLocalOrParam(cf, func, m_ident);
				if(!identDef)
				{
					throw std::runtime_error(std::string("identifier does not exist: ") + m_ident);
				}

				EmitLocalOrParam(cg, identDef);
			}

			GENERIC_DEF ASTIdentifierExpression::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto identDef = GetLocalOrParam(cf, func, m_ident);
				if(!identDef)
				{
					throw std::runtime_error(std::string("identifier does not exist: ") + m_ident);
				}

				Definitions::NativeDefinition^ type = nullptr;
				if(identDef->GetType() == Definitions::LocalDefinition::typeid)
				{
					type = ((Definitions::LocalDefinition^)identDef)->Type;
				}
				else if(identDef->GetType() == Definitions::ParameterDefinition::typeid)
				{
					type = (Definitions::NativeDefinition^)((Definitions::ParameterDefinition^)identDef)->Type;
				}
				else
				{
					throw std::runtime_error(std::string("Identifier is not a param or local: ") + m_ident);
				}

				return type;
			}

			GENERIC_DEF_LIST ASTIdentifierExpression::ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto ret = gcnew System::Collections::Generic::List<Definition^>;

				String^ typeName = nullptr;
				for each(auto local in func->Locals)
				{
					if(local->Name == gcnew String(m_ident.c_str()))
					{
						ret->Add(local);
					}
				}

				for each(auto param in func->Parameters)
				{
					if(param->Name == gcnew String(m_ident.c_str()))
					{
						ret->Add(param);
					}
				}

				for each(auto def in cf->Definitions)
				{
					if(def->ToName() == gcnew String(m_ident.c_str()))
					{
						ret->Add(def);
					}
				}

				return ret;
			}

			ASTAccess::ASTAccess(const std::shared_ptr<ASTExpression> &lhs, const std::string &rhs) : ASTExpression(),
				m_lhs(lhs), m_rhs(rhs)
			{

			}

			std::string ASTAccess::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_lhs->PrettyPrint() << '.' << m_rhs;
				return ss.str();
			}

			void ASTAccess::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				auto resolvedDefs = ResolveDef(cf, func, cd);
				for each(auto resolvedDef in resolvedDefs)
				{
					auto resolvedPropDef = dynamic_cast<Definitions::PropertyDefinition^>(resolvedDef);
	
					if(resolvedPropDef)
					{
						cg->Emit(Opcode::ObjectVar, resolvedPropDef);
						return;
					}
				}

				ASTExpression::EmitCode(cf, cg, func, cd);
			}

			GENERIC_DEF_LIST ASTAccess::ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto ret = gcnew System::Collections::Generic::List<Definition^>;

				auto lhsDefs = m_lhs->ResolveDef(cf, func, cd);

				lhsDefs->RemoveAll(gcnew Predicate<Definition^>(&IsNativeDef));

				if(lhsDefs->Count != 1)
				{
					std::stringstream ss;
					ss << "Did not get exactly 1 candidate when trying to resolve ASTAccess. Candidates: ";
					for each(auto def in lhsDefs)
					{
						ss << NetToCppString(def->GetType()->Name) << ':' << NetToCppString(def->Name) << ", ";
					}
					throw std::runtime_error(ss.str());
				}

				auto lhsDef = lhsDefs[0];

				if(Definitions::FunctionVarDefinition::typeid->IsInstanceOfType(lhsDef))
				{
					auto funcVarDef = (Definitions::FunctionVarDefinition^)(lhsDef);
					auto funcVarTypeDef = funcVarDef->Type;

					if(funcVarTypeDef->NativeType == NativeType::Handle)
					{
						funcVarTypeDef = (Definitions::NativeDefinition^)funcVarTypeDef->BaseType;
					}

					auto funcVarTypeClassDef = GetFromCache<Definitions::ClassDefinition>(cf, funcVarTypeDef->Name);

					if(!funcVarTypeClassDef)
					{
						std::stringstream ss;
						ss << "Could not find class of lhs in cache when trying to resolve. Candidates: ";
						for each(auto def in lhsDefs)
						{
							ss << NetToCppString(def->GetType()->Name) << ':' << NetToCppString(def->Name) << ", ";
						}
						throw std::runtime_error(ss.str());
					}

					for each(auto def in  funcVarTypeClassDef->Functions)
					{
						if(def->Name == gcnew String(m_rhs.c_str()))
						{
							ret->Add(def);
						}
					}

					//TODO: Implement properties
				}
				else if(Definitions::ClassDefinition::typeid->IsInstanceOfType(lhsDef))
				{
					auto classDef = (Definitions::ClassDefinition^)(lhsDef);

					for each(auto def in classDef->Functions)
					{
						if(def->Name == gcnew String(m_rhs.c_str()))
						{
							ret->Add(def);
						}
					}

					for each(auto def in classDef->Properties)
					{
						if(def->Name == gcnew String(m_rhs.c_str()))
						{
							ret->Add(def);
						}
					}
				}
				else
				{
					std::stringstream ss;
					ss << "Lhs of ASTAccess is not FunctionVarDefinition or ClassDefinition. Candidates: ";
					for each(auto def in lhsDefs)
					{
						ss << NetToCppString(def->GetType()->Name) << ':' << NetToCppString(def->Name) << ", ";
					}
				}

				return ret;
			}

			GENERIC_DEF ASTAccess::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto lhsDef = m_lhs->GetTypeDef(cf, func, cd);

				auto lhsClassDef = dynamic_cast<Definitions::ClassDefinition^>(lhsDef);

				if(!lhsClassDef)
				{
					throw std::runtime_error("Expecting lhs to resolve to a class type");
				}

				for each(auto propDef in lhsClassDef->Properties)
				{
					if(propDef->Name == gcnew String(m_rhs.c_str()))
					{
						return propDef->Type;
					}
				}

				throw std::runtime_error("Could not find property in class");
			}

			std::shared_ptr<ASTExpression> ASTAccess::GetLhs()
			{
				return m_lhs;
			}

			ConstructExpression::ConstructExpression(const std::string &className, std::vector<std::shared_ptr<ASTExpression>> ctorArgs) : ASTExpression(),
				m_className(className), m_ctorArgs(ctorArgs)
			{
			}

			std::string ConstructExpression::PrettyPrint()
			{
				std::stringstream ss;
				ss << "ctor(" << m_className;
				for(size_t i = 0; i < m_ctorArgs.size(); ++i)
				{
					ss << ", " << m_ctorArgs[i]->PrettyPrint();
				}
				ss << ')';
				return ss.str();
			}

			ArrayAccessExpression::ArrayAccessExpression(const std::shared_ptr<ASTExpression> &array, const std::shared_ptr<ASTExpression> &index) : ASTExpression(),
				m_array(array), m_index(index)
			{
			}

			std::string ArrayAccessExpression::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_array->PrettyPrint() << '[' << m_index->PrettyPrint() << ']';
				return ss.str();
			}

			ArrayLenExpression::ArrayLenExpression(const std::shared_ptr<ASTExpression> &array) : ASTExpression(),
				m_array(array)
			{
			}

			std::string ArrayLenExpression::PrettyPrint()
			{
				std::stringstream ss;
				ss << "arraylen(" << m_array->PrettyPrint() << ')';
				return ss.str();
			}

			BinaryExpression::BinaryExpression(std::shared_ptr<ASTExpression> lhs, Operator op, std::shared_ptr<ASTExpression> rhs) : ASTExpression(),
				m_lhs(lhs), m_op(op), m_rhs(rhs)
			{
			}

			std::string BinaryExpression::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_lhs->PrettyPrint() << ' ' << ASTOperatorToString(m_op) << ' ' << m_rhs->PrettyPrint();
				return ss.str();
			}
		}
	}
}