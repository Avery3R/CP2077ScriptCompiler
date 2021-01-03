#include "clrpch.hpp"

#include "ScriptCompilerLib.h"

#include "driver.hpp"
#include <vcclr.h>
#include <string>
#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <ASTUtils.hpp>

//#using "C:\Users\avery\source\repos\CP2077ScriptCompiler\TestDisassemblerLib\bin\Debug\net5.0\TestDisassemblerLib.dll"

using namespace Gibbed::RED4::ScriptFormats;
using namespace System::Text;
using namespace System::IO;
//using namespace System::Runtime::InteropServices;
//using namespace Msclr::interop;

namespace ScriptCompilerLib
{
}

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace Compiler
		{
			void InternalTestClass::doTheThing()
			{
				ParserDriver drv;
				std::fstream inputFile("compileTestScript.script", std::ios_base::in | std::ios_base::binary);
				inputFile.seekg(0, std::ios_base::end);
				size_t inputFileSize = inputFile.tellg();
				inputFile.seekg(0);

				char *inputBuf = new char[inputFileSize+1];
				inputBuf[inputFileSize] = 0;
				inputFile.read(inputBuf, inputFileSize);
				inputFile.close();
				std::string testFnCode(inputBuf);
				delete [] inputBuf;

				std::cout << "Testing compile of: \n" << testFnCode << std::endl;
				std::cout << std::endl;

				drv.Go(
					testFnCode
				);

				std::cout << "Parsed " << drv.m_imports.size() << " imports." << std::endl;
				std::cout << "Parsed " << drv.m_functions.size() << " functions." << std::endl;
				std::cout << "Parsed " << drv.m_classes.size() << " classes." << std::endl;
		
				std::cout << std::endl;

				for(size_t i = 0; i < drv.m_classes.size(); ++i)
				{
					std::cout << "PrettyPrint drv.m_classes[" << i << "]:\n" << drv.m_classes[i]->PrettyPrint() << std::endl;
				}
				std::cout << std::endl;

				for(size_t i = 0; i < drv.m_functions.size(); ++i)
				{
					std::cout << "PrettyPrint drv.m_functions[" << i << "]:\n" << drv.m_functions[i]->PrettyPrint() << std::endl;
				}
				std::cout << std::endl;

				/*for(size_t i = 0; i < drv.m_statements.size(); ++i)
				{
					std::cout << i << ' ' << drv.m_statements[i]->PrettyPrint() << std::endl;
					drv.m_statements[i]->Emit(cg, func);
				}*/
				std::cout << "Loading cache file..." << std::endl;
				auto cacheFileBytes = File::ReadAllBytes("D:\\Games\\Cyberpunk 2077\\r6\\cache\\final.redscripts");
				auto cacheFileStream = gcnew MemoryStream(cacheFileBytes, false);
				auto cacheFile = CacheFile::Load(cacheFileStream, true);

				/*sstd::fstream funcDump("funcDump.txt", std::ios_base::out);

				size_t defNum = 0;

				for each(auto def in cacheFile->Definitions)
				{
					if(def->GetType() == Definitions::ClassDefinition::typeid)
					{
						auto classDef = (Definitions::ClassDefinition^)def;

						funcDump << defNum << " class " << NetToCppString(classDef->Name);
						if(classDef->BaseClass != nullptr)
						{
							funcDump << " : class " << NetToCppString(classDef->BaseClass->Name);
						}
						funcDump << "\n{\n";
						for each(auto propertyDef in classDef->Properties)
						{
							funcDump << '\t' << NetToCppString(propertyDef->Visibility.ToString())
								<< " var " << NetToCppString(propertyDef->Name)
								<< " : " << NetToCppString(propertyDef->Type->ToString())
								<< "; //" << NetToCppString(propertyDef->Flags.ToString()) << '\n';
							if(!propertyDef->Flags.HasFlag(Definitions::PropertyFlags::Unknown10))
							{
								__debugbreak();
							}
						}
						funcDump << '\n';
						for each(auto funcDef in classDef->Functions)
						{
							funcDump << "\tfunction ";
							if(funcDef->ReturnType)
							{
								funcDump << NetToCppString(funcDef->ReturnType->ToName());
							}
							else
							{
								funcDump << "void";
							}
							funcDump << ' ' << NetToCppString(funcDef->Name) << '(';
							for(size_t i = 0; i < funcDef->Parameters->Count; ++i)
							{
								if(i != 0)
								{
									funcDump << ", ";
								}

								auto param = funcDef->Parameters[i];
								funcDump << NetToCppString(param->Type->ToString()) << ' ' << NetToCppString(param->Name);
							}
							funcDump << "); //" << NetToCppString(funcDef->Flags.ToString()) << '\n';
						}
						funcDump << "};\n\n";
					}
					else if(def->GetType() == Definitions::FunctionDefinition::typeid)
					{
						auto funcDef = (Definitions::FunctionDefinition^)def;
						if(funcDef->Parent == nullptr)
						{
							funcDump << defNum << " function ";
							if(funcDef->ReturnType)
							{
								funcDump << NetToCppString(funcDef->ReturnType->ToName());
							}
							else
							{
								funcDump << "void";
							}
							funcDump << ' ' << NetToCppString(funcDef->Name) << '(';
							for(size_t i = 0; i < funcDef->Parameters->Count; ++i)
							{
								if(i != 0)
								{
									funcDump << ", ";
								}

								auto param = funcDef->Parameters[i];
								funcDump << NetToCppString(param->Type->ToString()) << ' ' << NetToCppString(param->Name);
							}
							funcDump << "); //" << NetToCppString(funcDef->Flags.ToString()) << '\n';
						}
					}

					++defNum;
				}

				funcDump.close();*/

				for(const auto &import : drv.m_imports)
				{
					auto def = import->Emit(cacheFile);
					cacheFile->Definitions->Add(def);
				}

				for(const auto &astClass : drv.m_classes)
				{
					try
					{
						auto cd = astClass->Emit(cacheFile);

						auto sb = gcnew StringBuilder();

						std::cout << "Hacked in ScriptCacheDumpTest::Program::DumpFunction() thx gibbed: " << std::endl;

						TestDisassemblerLib::TestDisassemblerLib::DumpFunction(cacheFile, cd->Functions[0], sb, false);

						pin_ptr<const wchar_t> wch = PtrToStringChars(sb->ToString());
						std::wstring sbTest = wch;

						_setmode(_fileno(stdout), _O_U16TEXT);
						std::wcout << sbTest << std::endl;
						_setmode(_fileno(stdout), _O_TEXT);
					}
					catch(std::exception &e)
					{
						std::cout << "Caught exception " << e.what() << std::endl;
					}
				}

				for(const auto &func : drv.m_functions)
				{
					Definitions::FunctionDefinition^ funcDef = nullptr;
					try
					{
						funcDef = drv.m_functions[0]->Emit(cacheFile, nullptr);
					}
					catch(std::exception &e)
					{
						std::cout << "Caught exception " << e.what() << std::endl;
					}

					auto sb = gcnew StringBuilder();

					std::cout << "Hacked in ScriptCacheDumpTest::Program::DumpFunction() thx gibbed: " << std::endl;

					TestDisassemblerLib::TestDisassemblerLib::DumpFunction(cacheFile, funcDef, sb, false);

					pin_ptr<const wchar_t> wch = PtrToStringChars(sb->ToString());
					std::wstring sbTest = wch;

					_setmode(_fileno(stdout), _O_U16TEXT);
					std::wcout << sbTest << std::endl;
					_setmode(_fileno(stdout), _O_TEXT);
				}
			}

			template<typename A, typename T>
			ParsedASTWrapper<A,T>::ParsedASTWrapper(std::shared_ptr<A> astClass)
			{
				m_astClass = new std::shared_ptr<A>(astClass);
			}

			template<typename A, typename T>
			ParsedASTWrapper<A,T>::~ParsedASTWrapper()
			{
				delete m_astClass;
			}

			template<typename A, typename T>
			T^ ParsedASTWrapper<A,T>::GetDefinition(CacheFile ^cf)
			{
				return (*m_astClass)->Emit(cf);
			}

			template<typename A, typename T>
			String^ ParsedASTWrapper<A,T>::PrettyPrint()
			{
				return gcnew String((*m_astClass)->PrettyPrint().c_str());
			}

			template ParsedASTWrapper<AST::ASTImport, Definition>;
			template ParsedASTWrapper<AST::ASTClass, Definitions::ClassDefinition>;
			template ParsedASTWrapper<AST::ASTFunction, Definitions::FunctionDefinition>;

			CompilerResult::CompilerResult()
			{
				Imports = gcnew List<ParsedASTWrapper<AST::ASTImport, Definition>^>;
				Classes = gcnew List<ParsedASTWrapper<AST::ASTClass, Definitions::ClassDefinition>^>;
				Functions = gcnew List<ParsedASTWrapper<AST::ASTFunction, Definitions::FunctionDefinition>^>;
			}

			CompilerResult^ NetAPI::ParseScript(String^ script)
			{
				ParserDriver drv;

				int parseResult = 0;

				try
				{
					parseResult = drv.Go(AST::NetToCppString(script));
				}
				catch(std::exception &e)
				{
					std::cout << "Caught exception while parsing script. e.what(): " << e.what() << std::endl;
					parseResult = 2;
				}

				if(parseResult)
				{
					std::cout << "Parsing did not complete successfully :(" << std::endl;
					return nullptr;
				}

				auto ret = gcnew CompilerResult();
				for(auto &import : drv.m_imports)
				{
					ret->Imports->Add(gcnew ParsedASTWrapper<AST::ASTImport, Definition>(import));
				}

				for(auto &astClass : drv.m_classes)
				{
					ret->Classes->Add(gcnew ParsedASTWrapper<AST::ASTClass, Definitions::ClassDefinition>(astClass));
				}

				for(auto &func : drv.m_functions)
				{
					ret->Functions->Add(gcnew ParsedASTWrapper<AST::ASTFunction, Definitions::FunctionDefinition>(func));
				}

				return ret;
			}

			void NetAPI::AddDefToCacheNoDupes(CacheFile ^cf, Definition ^def)
			{
				if(!cf->Definitions->Contains(def))
				{
					cf->Definitions->Add(def);
				}
			}

			void NetAPI::AddFuncToCache(CacheFile ^cf, Definitions::FunctionDefinition ^def)
			{
				AddDefToCacheNoDupes(cf, def);
				
				if(def->ReturnType)
				{
					AddDefToCacheNoDupes(cf, def->ReturnType);
				}

				for each(auto local in def->Locals)
				{
					AddDefToCacheNoDupes(cf, local);
					if(local->Type)
					{
						AddDefToCacheNoDupes(cf, local->Type);
					}
				}

				for each(auto param in def->Parameters)
				{
					AddDefToCacheNoDupes(cf, param);
					if(param->Type)
					{
						AddDefToCacheNoDupes(cf, param->Type);
					}
				}
			}

			void NetAPI::AddClassToCache(CacheFile ^cf, Definitions::ClassDefinition ^def)
			{
				AddDefToCacheNoDupes(cf, def);

				for each(auto prop in def->Properties)
				{
					AddDefToCacheNoDupes(cf, prop);
					if(prop->Type)
					{
						AddDefToCacheNoDupes(cf, prop->Type);
					}
				}

				for each(auto func in def->Functions)
				{
					AddFuncToCache(cf, func);
				}
			}
		}
	}
}