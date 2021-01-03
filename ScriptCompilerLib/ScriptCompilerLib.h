#pragma once

#include <memory>

using namespace System;
using namespace System::Collections::Generic;
using namespace Gibbed::RED4::ScriptFormats;

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			class ASTImport;
			class ASTClass;
			class ASTFunction;
		}

		namespace Compiler
		{
			private ref class InternalTestClass
			{
				public:
					static void doTheThing();
			};

			template<typename A, typename T>
			public ref class ParsedASTWrapper
			{
				public:
					ParsedASTWrapper(std::shared_ptr<A> astClass);
					~ParsedASTWrapper();

					T^ GetDefinition(CacheFile ^cf);
					String^ PrettyPrint();

				private:
					std::shared_ptr<A> *m_astClass;
			};

			public ref class CompilerResult
			{
				public:
					property List<ParsedASTWrapper<AST::ASTImport, Definition>^>^ Imports;
					property List<ParsedASTWrapper<AST::ASTClass, Definitions::ClassDefinition>^>^ Classes;
					property List<ParsedASTWrapper<AST::ASTFunction, Definitions::FunctionDefinition>^>^ Functions;

					CompilerResult();
			};

			public ref class NetAPI
			{
				public:
					static CompilerResult^ ParseScript(String^ script);

					static void AddDefToCacheNoDupes(CacheFile ^cf, Definition ^def);
					static void AddFuncToCache(CacheFile ^cf, Definitions::FunctionDefinition ^def);
					static void AddClassToCache(CacheFile ^cf, Definitions::ClassDefinition ^def);
			};
		}
	}
}
