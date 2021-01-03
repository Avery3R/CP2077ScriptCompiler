#pragma once

#include "ASTEnums.hpp"
#include "ASTNetTypes.hpp"
#include "ASTBaseTypes.hpp"
#include "ASTBasicExpressions.hpp"
#include "ASTComplexExpressions.hpp"
#include "ASTStatements.hpp"

#include <memory>
#include <cstdint>
#include <string>
#include <vector>

namespace Avery3R
{
	namespace RED4Scripts
	{
		namespace AST
		{
			class ASTClassImport : public ASTImport
			{
				public:
					ASTClassImport(const std::string &className);

					std::string PrettyPrint() override;

					GENERIC_DEF Emit(CACHE_FILE cf) override;

				private:
					std::string m_className;
			};

			class ASTType : public ASTPrintable
			{
				public:
					ASTType(const std::string &typeName, bool isRef);

					std::string PrettyPrint() override;

					NATIVE_DEF GetNativeType(CACHE_FILE cf);
				private:
					std::string m_typeName;
					bool m_isRef;
			};

			class ASTVarDecl : public ASTPrintable
			{
				public:
					ASTVarDecl(const std::string &identifier, const std::shared_ptr<ASTType> &type);

					std::string PrettyPrint() override;

					std::string GetIdent();
					std::shared_ptr<ASTType> GetType();

				private:
					std::string m_ident;
					std::shared_ptr<ASTType> m_type;
			};

			class ASTFunction : public ASTPrintable, public ASTEmittable
			{
				public:
					enum FunctionFlags : uint32_t
					{
						FF_None = 0,
						FF_IsStatic = 1u << 0,
						FF_IsExec = 1u << 1,
						FF_Unknown2 = 1u << 2,
						FF_Unknown3 = 1u << 3,
						FF_IsNative = 1u << 4,
						FF_IsEvent = 1u << 5,
						FF_Unknown6 = 1u << 6,
						FF_HasReturnValue = 1u << 7,
						FF_Unknown8 = 1u << 8,
						FF_HasParameters = 1 << 9,
						FF_HasLocals = 1 << 10,
						FF_HasCode = 1u << 11,
						FF_Unknown12 = 1u << 12,
						FF_Unknown13 = 1u << 13,
						FF_IsConstant = 1u << 18,
						FF_Unknown19 = 1u << 19,
						FF_Unknown20 = 1u << 20,
						FF_Unknown21 = 1u << 21,
					};

					ASTFunction(const std::string &name, FunctionFlags flags, const std::vector<std::pair<std::shared_ptr<ASTType>, std::string>> &params, const std::vector<std::shared_ptr<ASTStatement>> statements);

					std::string PrettyPrint() override;

					FUNC_DEF Emit(CACHE_FILE cf, CLASS_DEF cd = nullptr);

				private:
					std::string m_name;
					std::vector<std::shared_ptr<ASTStatement>> m_statements;
					std::vector<std::pair<std::shared_ptr<ASTType>, std::string>> m_params;
					FunctionFlags m_flags;
			};

			class ASTFunctionCall : public ASTStatement, public ASTExpression
			{
				public:
					ASTFunctionCall(const std::shared_ptr<ASTExpression> &lhs, const std::vector<std::shared_ptr<ASTExpression>> &args);

					std::string PrettyPrint() override;

					void EmitCode(CACHE_FILE cf, CODE_GEN cg, FUNC_DEF func, CLASS_DEF cd) override;

					GENERIC_DEF GetTypeDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;

					GENERIC_DEF_LIST ResolveDef(CACHE_FILE cf, FUNC_DEF func, CLASS_DEF cd) override;
		
				private:
					std::shared_ptr<ASTExpression> m_lhs;
					std::vector<std::shared_ptr<ASTExpression>> m_args;
			};

			class ASTClassBody : public ASTPrintable
			{
				public:
					enum PropertyFlag : uint16_t
					{
						PF_IsNative = 1 << 0,
					};

					ASTClassBody();

					std::string PrettyPrint() override;

					void AddVarDecl(Visibility vis, PropertyFlag flag, std::shared_ptr<ASTVarDecl> decl);
					void AddFunc(Visibility vis, std::shared_ptr<ASTFunction> func);

				private:
					struct ClassVarDecl
					{
						Visibility vis;
						PropertyFlag flag;
						std::shared_ptr<ASTVarDecl> decl;
					};

					std::vector<ClassVarDecl> m_varDecls;
					std::vector<std::pair<Visibility,std::shared_ptr<ASTFunction>>> m_funcs;

					friend class ASTClass;
			};

			class ASTClass : public ASTPrintable
			{
				public:
					ASTClass(Visibility visibility, const std::string &name, std::shared_ptr<ASTClassBody> body);
		
					std::string PrettyPrint() override;

					CLASS_DEF Emit(CACHE_FILE cf);

				private:
					Visibility m_visibility;
					std::string m_name;
					std::shared_ptr<ASTClassBody> m_body;
			};
		}
	}
}