#pragma once

//This macro hackyiness is needed to make it compile under both clr and native code. lexer/parser native, clr everything else
#ifdef __cplusplus_cli
	namespace gsf = Gibbed::RED4::ScriptFormats; //For ambiguities
	using namespace Gibbed::RED4::ScriptFormats;
	using namespace System;

	#define CACHE_FILE gsf::CacheFile^
	#define CODE_GEN gsf::Emit::CodeGenerator^
	#define FUNC_DEF gsf::Definitions::FunctionDefinition^
	#define NATIVE_DEF gsf::Definitions::NativeDefinition^
	#define CLASS_DEF gsf::Definitions::ClassDefinition^
	#define GENERIC_DEF gsf::Definition^
	#define GENERIC_DEF_LIST System::Collections::Generic::List<gsf::Definition^>^
#else
	#define CACHE_FILE void*
	#define CODE_GEN void*
	#define FUNC_DEF void*
	#define NATIVE_DEF void*
	#define CLASS_DEF void*
	#define GENERIC_DEF void*
	#define GENERIC_DEF_LIST void*
#endif