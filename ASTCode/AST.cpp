#include "clrpch.hpp"
#include "AST.hpp"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "ASTUtils.hpp"

using namespace System;

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			ASTClassImport::ASTClassImport(const std::string &className) : ASTImport(),
				m_className(className)
			{
			}

			std::string ASTClassImport::PrettyPrint()
			{
				std::stringstream ss;
				ss << "import " << m_className << ';';
				return ss.str();
			}

			GENERIC_DEF ASTClassImport::Emit(CACHE_FILE cf)
			{
				auto classDef = GetFromCache<Definitions::ClassDefinition>(cf, m_className.c_str());
				if(!classDef)
				{
					std::cout << "Warning ASTClassImport::AddDefToCache() m_className does not exist in cache as a ClassDefinition: " << m_className << std::endl;
				}

				auto newType = gcnew Definitions::NativeDefinition;
				newType->Name = gcnew String(m_className.c_str());
				newType->NativeType = NativeType::Complex;

				return newType;
			}

			ASTVarDecl::ASTVarDecl(const std::string &identifier, const std::shared_ptr<ASTType> &type) : ASTPrintable(),
				m_ident(identifier), m_type(type)
			{
			}

			std::string ASTVarDecl::PrettyPrint()
			{
				std::stringstream ss;
				ss << "var " << m_ident << " : " << m_type->PrettyPrint();
				return ss.str();
			}

			std::string ASTVarDecl::GetIdent()
			{
				return m_ident;
			}

			std::shared_ptr<ASTType> ASTVarDecl::GetType()
			{
				return m_type;
			}

			ASTFunction::ASTFunction(const std::string &name, FunctionFlags flags, const std::vector<std::pair<std::shared_ptr<ASTType>, std::string>> &params, const std::vector<std::shared_ptr<ASTStatement>> statements) : ASTPrintable(), ASTEmittable(),
				m_name(name), m_statements(statements), m_flags(flags), m_params(params)
			{
			}

			std::string ASTFunction::PrettyPrint()
			{
				std::stringstream ss;
				ss << "function " << m_name << " Flags(" << int(m_flags) << ") (";
				for(size_t i = 0; i < m_params.size(); ++i)
				{
					if(i != 0)
					{
						ss << ", ";
					}
					ss << m_params[i].first->PrettyPrint() << ' ' << m_params[i].second;
				}
				ss << ")\n{\n";

				for(const auto &stmt : m_statements)
				{
					ss << '\t' << stmt->PrettyPrint() << '\n';
				}

				ss << '}';

				return ss.str();
			}

			FUNC_DEF ASTFunction::Emit(CACHE_FILE cf, CLASS_DEF cd)
			{
				auto ret = gcnew Definitions::FunctionDefinition;
				ret->Name =  gcnew String(m_name.c_str());
				ret->Flags = (Definitions::FunctionFlags)m_flags;

				for(const auto param : m_params)
				{
					auto pd = gcnew Definitions::ParameterDefinition();
					pd->Name = gcnew String(param.second.c_str());
					pd->Type = param.first->GetNativeType(cf);
					pd->Parent = ret;

					ret->Parameters->Add(pd);
					//cf->Definitions->Add(pd);
				}

				auto cg = gcnew Emit::CodeGenerator();

				for(size_t i = 0; i < m_statements.size(); ++i)
				{
					//FIXME: hack, to fix seperate ASTFunctionCall into ASTFunctionCallExpression and ASTFunctionCallStatment
					if(!std::dynamic_pointer_cast<ASTFunctionCall>(m_statements[i]))
					{
						m_statements[i] = m_statements[i]->Simplify();
					}
				}

				for(const auto &stmt : m_statements)
				{
					stmt->EmitCode(cf, cg, ret, cd);
				}

				cg->Emit(Opcode::Nop);

				ret->Code->AddRange(cg->GetCode());

				if(ret->Code->Count != 0)
				{
					ret->Flags |= (Definitions::FunctionFlags)FF_HasCode;
				}

				if(ret->Locals->Count != 0)
				{
					ret->Flags |= (Definitions::FunctionFlags)FF_HasLocals;
				}

				if(ret->Parameters->Count != 0)
				{
					ret->Flags |= (Definitions::FunctionFlags)FF_HasParameters;
				}

				for each(auto local in ret->Locals)
				{
					//cf->Definitions->Add(local);
				}

				return ret;
			}

			ASTType::ASTType(const std::string &typeName, bool isRef) : ASTPrintable(),
				m_typeName(typeName), m_isRef(isRef)
			{
			}

			std::string ASTType::PrettyPrint()
			{
				std::stringstream ss;
				if(m_isRef)
				{
					ss << "ref<";
				}
				ss << m_typeName;
				if(m_isRef)
				{
					ss << '>';
				}

				return ss.str();
			}


			NATIVE_DEF ASTType::GetNativeType(CACHE_FILE cf)
			{
				if(m_isRef)
				{
					std::string searchStr = "ref:";
					searchStr += m_typeName;
		
					auto foundRefType = GetFromCache<Definitions::NativeDefinition>(cf, searchStr);
		
					if(foundRefType)
					{
						return foundRefType;
					}
				}

				Definitions::NativeDefinition ^baseType = GetFromCache<Definitions::NativeDefinition>(cf, m_typeName);

				if(!baseType)
				{
					/*std::cout << "Warning: could not find type " << m_typeName << " assuming it's NativeType::Complex" << std::endl;
		
					baseType = gcnew Definitions::NativeDefinition;
					baseType->Name = gcnew String(m_typeName.c_str());
					baseType->NativeType = NativeType::Complex;

					cf->Definitions->Add(baseType);*/

					throw std::runtime_error(std::string("ASTType::GetNativeType() could not find type, maybe you need an import statement?: ") + m_typeName);
				}

				//The actual ref wasn't already in the definitions so we make our own
				if(m_isRef)
				{
					auto refType = gcnew Definitions::NativeDefinition;
					refType->Name = gcnew String((std::string("ref:")+m_typeName).c_str());
					refType->NativeType = NativeType::Handle;
					refType->BaseType = baseType;

					//cf->Definitions->Add(refType);

					return refType;
				}

				return baseType;
			}

			ASTFunctionCall::ASTFunctionCall(const std::shared_ptr<ASTExpression> &lhs, const std::vector<std::shared_ptr<ASTExpression>> &args) : ASTStatement(), ASTExpression(),
				m_lhs(lhs), m_args(args)
			{
			}

			std::string ASTFunctionCall::PrettyPrint()
			{
				std::stringstream ss;
				ss << m_lhs->PrettyPrint() << '(';
				for(size_t i = 0; i < m_args.size(); ++i)
				{
					if(i != 0)
					{
						ss << ", ";
					}
					ss << m_args[i]->PrettyPrint();
				}
				ss << ')';
				return ss.str();
			}

			void ASTFunctionCall::EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd)
			{
				auto argTypes = gcnew Collections::Generic::List<Definition^>;
				for(const auto &arg : m_args)
				{
					argTypes->Add(arg->GetTypeDef(cf, func, cd));
				}

				auto resolvedDefs = m_lhs->ResolveDef(cf, func, cd);

				Definitions::FunctionDefinition^ funcDef = FindFuncDefMatchingArgs(resolvedDefs, cf, argTypes);
	
				if(!funcDef)
				{
					throw std::runtime_error("Could not find function matching name and args");
				}

				auto contextEnd = cg->DefineLabel();

				if(!funcDef->Flags.HasFlag(Definitions::FunctionFlags::IsStatic))
				{
					auto accessExpr = std::dynamic_pointer_cast<ASTAccess>(m_lhs);
					if(!accessExpr)
					{
						std::stringstream ss;
						ss << "Trying to call non-static function without an access on the lhs: " << PrettyPrint();
						throw std::runtime_error(ss.str());
					}

					cg->Emit(Opcode::Context, contextEnd);
					accessExpr->GetLhs()->EmitCode(cf, cg, func, cd);
				}

				cg->Emit(Opcode::FinalFunc, contextEnd, 0, funcDef);

				for(const auto &arg : m_args)
				{
					arg->EmitCode(cf, cg, func, cd);
				}

				cg->Emit(Opcode::ParamEnd);
				cg->MarkLabel(contextEnd);

				/*auto identType = GetFromCache<Definitions::ClassDefinition>(cf, identDef->Type);

				if(!localType)
				{
					throw std::runtime_error(std::string("Could not find type for local ") + m_ident);
				}*/
			}

			GENERIC_DEF ASTFunctionCall::GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto argTypes = gcnew Collections::Generic::List<Definition^>;
				for(const auto &arg : m_args)
				{
					argTypes->Add(arg->GetTypeDef(cf, func, cd));
				}

				auto resolvedDefs = m_lhs->ResolveDef(cf, func, cd);

				return FindFuncDefMatchingArgs(resolvedDefs, cf, argTypes)->ReturnType;
			}

			GENERIC_DEF_LIST ASTFunctionCall::ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd)
			{
				auto ret = gcnew System::Collections::Generic::List<Definition^>;

				auto retType = (Definitions::NativeDefinition^)GetTypeDef(cf, func, cd);
				if(retType->NativeType == NativeType::Handle)
				{
					retType = (Definitions::NativeDefinition^)retType->BaseType;
				}

				auto retClass = GetFromCache<Definitions::ClassDefinition>(cf, retType->Name);
				if(!retClass)
				{
					throw std::runtime_error(std::string("ASTFunctionCall::ResolveDef() could not find class: ") + PrettyPrint());
				}
				ret->Add(retClass);

				return ret;
			}

			ASTClass::ASTClass(Visibility visibility, const std::string &name, std::shared_ptr<ASTClassBody> body) : ASTPrintable(),
				m_visibility(visibility), m_name(name), m_body(body)
			{
			}

			std::string ASTClass::PrettyPrint()
			{
				std::stringstream ss;
				ss << VisibilityToString(m_visibility) << " class " << m_name << m_body->PrettyPrint();
				return ss.str();
			}

			CLASS_DEF ASTClass::Emit(CACHE_FILE cf)
			{
				Definitions::ClassDefinition ^cd = nullptr;

				switch(m_visibility)
				{
					case Visibility::INJECT:
					{
						std::cout << "Warning: injected classes currently always modify cache" << std::endl;
						cd = GetFromCache<Definitions::ClassDefinition>(cf, m_name);

						if(!cd)
						{
							throw std::runtime_error(std::string("Could not find class ") + m_name + " to inject into");
						}
					}
					break;
					default:
						throw std::runtime_error("ASTClass::EmitToCache not yet implemented for new classes, inject only");
				}

				for(const auto &varDecl : m_body->m_varDecls)
				{
					auto propDef = gcnew Definitions::PropertyDefinition;
					propDef->Name = gcnew String(varDecl.decl->GetIdent().c_str());
					propDef->Flags = (Definitions::PropertyFlags)varDecl.flag;
					propDef->Flags = (Definitions::PropertyFlags)propDef->Flags|Definitions::PropertyFlags::Unknown10;
					propDef->Type = varDecl.decl->GetType()->GetNativeType(cf);

					switch(varDecl.vis)
					{
						case Visibility::PUBLIC:
							propDef->Visibility = gsf::Visibility::Public;
						break;
						case Visibility::PROTECTED:
							propDef->Visibility = gsf::Visibility::Protected;
						break;
						case Visibility::PRIVATE:
							propDef->Visibility = gsf::Visibility::Private;
						break;
						default:
							throw std::runtime_error("Invalid visibility for class property");
					}

					propDef->Parent = cd;
					cd->Properties->Add(propDef);
					//cf->Definitions->Add(propDef);
				}

				for(const auto &func : m_body->m_funcs)
				{
					switch(func.first)
					{
						case Visibility::INJECT:
						{
							auto genFuncDef = func.second->Emit(cf, cd);

							Definitions::FunctionDefinition ^classFunc = nullptr;
							for each(auto def in cd->Functions)
							{
								if(def->Name == genFuncDef->Name)
								{
									classFunc = def;
									break;
								}
							}

							if(!classFunc)
							{
								throw std::runtime_error("Could not find function in class");
							}

							classFunc->Code->RemoveAt(classFunc->Code->Count-1);
							classFunc->Code->AddRange(genFuncDef->Code);
						}
						break;
						default:
							throw std::runtime_error("Can not yet insert new functions into classes, only inject code into old");
					}
				}

				return cd;
			}

			ASTClassBody::ASTClassBody() : ASTPrintable(),
				m_varDecls(), m_funcs()
			{
			}

			std::string ASTClassBody::PrettyPrint()
			{
				std::stringstream ss;
				ss << "\n{\n";
				for(const auto &decl : m_varDecls)
				{
					ss << '\t' << VisibilityToString(decl.vis) << " flag(" << uint16_t(decl.flag) << ')' << decl.decl->PrettyPrint() << ";\n";
				}

				for(const auto &func : m_funcs)
				{
					ss << '\t' << VisibilityToString(func.first) << ' ' << func.second->PrettyPrint() << '\n';
				}
				ss << "}\n";
				return ss.str();
			}

			void ASTClassBody::AddVarDecl(Visibility vis, PropertyFlag flag, std::shared_ptr<ASTVarDecl> decl)
			{
				ClassVarDecl cvd{vis, flag, decl};
				m_varDecls.emplace_back(cvd);
			}

			void ASTClassBody::AddFunc(Visibility vis, std::shared_ptr<ASTFunction> func)
			{
				m_funcs.emplace_back(vis, func);
			}
		}
	}
}