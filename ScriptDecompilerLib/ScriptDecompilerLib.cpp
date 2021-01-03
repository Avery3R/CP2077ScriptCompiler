#include "clrpch.hpp"

#include "ScriptDecompilerLib.h"

#include "driver.hpp"
#include <vcclr.h>
#include <string>
#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <ASTUtils.hpp>

using namespace Gibbed::RED4::ScriptFormats;
using namespace System::Collections::Generic;
using namespace ScriptCacheDumpTest;
using namespace System::Text;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
//using namespace Msclr::interop;

namespace ScriptDecompilerLib
{
	void Class1::doTheThing()
	{
		std::cout << "Loading cache file..." << std::endl;
		auto cacheFileBytes = File::ReadAllBytes("D:\\Games\\Cyberpunk 2077\\r6\\cache\\final.redscripts");
		auto cacheFileStream = gcnew MemoryStream(cacheFileBytes, false);
		auto cacheFile = CacheFile::Load(cacheFileStream, true);

		for each(auto def in cacheFile->Definitions)
		{
			if(Definitions::FunctionDefinition::typeid->IsInstanceOfType(def))
			{
				auto funcDef = (Definitions::FunctionDefinition^)def;
				if(funcDef->Flags.HasFlag(Definitions::FunctionFlags::HasCode) && funcDef->Flags.HasFlag(Definitions::FunctionFlags::IsStatic)
					//&& funcDef->ToName() == "ManyCNamesIntoSingleString"
				)
				{
					ParserDriver driver = {};
					GCHandle codeHandle = GCHandle::Alloc(funcDef->Code);
					driver.m_code = GCHandle::ToIntPtr(codeHandle).ToPointer();

					std::cout << "function " << AST::NetToCppString(funcDef->ToName()) << '(';
					for(size_t i = 0; i < funcDef->Parameters->Count; ++i)
					{
						if(i != 0)
						{
							std::cout << ", ";
						}
						std::cout << AST::NetToCppString(funcDef->Parameters[i]->Name) << " : " << AST::NetToCppString(funcDef->Parameters[i]->Type->ToString());
					}
					std::cout << ")\n{\n";

					driver.Go();
					driver.DoControlFlowAnalysis();
					
					for(size_t i = 0; i < funcDef->Locals->Count; ++i)
					{
						std::cout << "var " << AST::NetToCppString(funcDef->Locals[i]->Name) << " : " << AST::NetToCppString(funcDef->Locals[i]->Type->ToString())
							<< ";\n";
					}
					if(funcDef->Locals->Count != 0)
					{
						std::cout << '\n';
					}

					for(const auto &stmt : driver.m_statements)
					{
						if(stmt)
						{
							std::cout << stmt->PrettyPrint() << std::endl;
						}
					}
					std::cout << "}\n" << std::endl;
				}
			}
		}
	}
}