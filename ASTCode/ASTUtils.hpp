#pragma once

#include <stdexcept>
#include "ASTNetTypes.hpp"

using namespace System::Runtime::InteropServices;

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			template<typename T>
			inline static T^ GetFromCache(CACHE_FILE cf, const std::string &name)
			{
				for each(auto def in cf->Definitions)
				{
					if(def->GetType() == T::typeid)
					{
						if(def->Name == gcnew String(name.c_str()))
						{
							return (T^)def;
						}
					}
				}

				return nullptr;
			}

			template<typename T>
			inline static T^ GetFromCache(CACHE_FILE cf, const String^ name)
			{
				for each(auto def in cf->Definitions)
				{
					if(def->GetType() == T::typeid)
					{
						if(def->Name->Equals((Object^)name))
						{
							return (T^)def;
						}
					}
				}

				return nullptr;
			}

			inline static Definition^ GetLocalOrParam(CACHE_FILE cf, FUNC_DEF func, const std::string &ident)
			{
				String^ typeName = nullptr;
				for each(auto local in func->Locals)
				{
					if(local->Name == gcnew String(ident.c_str()))
					{
						return local;
					}
				}

				if(!typeName)
				{
					for each(auto param in func->Parameters)
					{
						if(param->Name == gcnew String(ident.c_str()))
						{
							return param;
						}
					}
				}

				return nullptr;
			}

			inline static void EmitLocalOrParam(CODE_GEN cg, Definition ^def)
			{
				if(def->GetType() == Definitions::LocalDefinition::typeid)
				{
					cg->Emit(Opcode::LocalVar, (Definitions::LocalDefinition^)def);
				}
				else if(def->GetType() == Definitions::ParameterDefinition::typeid)
				{
					cg->Emit(Opcode::ParamVar, (Definitions::ParameterDefinition^)def);
				}
			}

			inline static bool IsClassSubclassOfClass(Definitions::ClassDefinition^ potentialChild, Definitions::ClassDefinition^ parent)
			{
				if(potentialChild == parent)
				{
					return true;
				}

				auto childParent = potentialChild->BaseClass;
				while(childParent != nullptr)
				{
					childParent = potentialChild->BaseClass;
					if(parent == childParent)
					{
						return true;
					}
				}

				return false;
			}

			inline static Definitions::NativeDefinition^ RefToParent(Definitions::NativeDefinition^ type)
			{
				if(type->NativeType == NativeType::Handle)
				{
					return (Definitions::NativeDefinition^)type->BaseType;
				}
				return type;
			}

			inline static bool IsTypeAType(CACHE_FILE cf, Definitions::NativeDefinition^ candidate, Definitions::NativeDefinition^ type)
			{
				if(candidate->NativeType == NativeType::Simple)
				{
					return candidate == type;
				}

				candidate = RefToParent(candidate);
				type = RefToParent(type);

				auto typeClass = GetFromCache<Definitions::ClassDefinition>(cf, type->Name);
				auto candidateClass = GetFromCache<Definitions::ClassDefinition>(cf, candidate->Name);

				if(!typeClass)
				{
					throw std::runtime_error("Could not get class of type");
				}
				if(!candidateClass)
				{
					throw std::runtime_error("Could not get class of candidate");
				}

				return IsClassSubclassOfClass(candidateClass, typeClass);
			}

			template<typename DEF_TYPE>
			inline static Definitions::FunctionDefinition^ FindFuncDefMatchingArgs(Collections::Generic::List<DEF_TYPE^> ^defs, CACHE_FILE cf, /*const std::string &funcName,*/ Collections::Generic::List<Definition^>^ argTypes)
			{
				for each(auto def in defs)
				{
					if(def->GetType() == Definitions::FunctionDefinition::typeid)
					{
						auto func = (Definitions::FunctionDefinition^)def;
						//if(func->ToName() == gcnew String(funcName.c_str()))
						{
							if(func->Parameters->Count == argTypes->Count)
							{
								bool bad = false;
								for(size_t i = 0; i < func->Parameters->Count; ++i)
								{
									if(argTypes[(int)i] != nullptr)
									{
										auto funcParamType = RefToParent(func->Parameters[i]->Type);
										auto argType = RefToParent((Definitions::NativeDefinition^)argTypes[(int)i]);

										if(!IsTypeAType(cf, argType, funcParamType))
										{
											bad = true;
											break;
										}
									}
								}

								if(!bad)
								{
									return func;
								}
							}
						}
					}
				}

				return nullptr;
			}

			inline static std::string NetToCppString(String^ string)
			{
				IntPtr p = Marshal::StringToHGlobalAnsi(string);
				char *cstr = static_cast<char*>(p.ToPointer());

				std::string ret = cstr;

				Marshal::FreeHGlobal(p);

				return ret;
			}

			inline static bool IsNativeDef(Definition ^def)
			{
				return Definitions::NativeDefinition::typeid->IsInstanceOfType(def);
			}
		}
	}
}