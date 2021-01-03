#pragma once

#pragma warning(push)
#pragma warning(disable: 4065)

#include "parserimpl.hpp"
#include "AST.hpp"

#pragma warning(pop)

class ParserDriver
{
	public:
		ParserDriver();

		int Go(const std::string &data);

		yy::location m_loc;
		std::vector<std::shared_ptr<AST::ASTFunction>> m_functions;
		std::vector<std::shared_ptr<AST::ASTImport>> m_imports;
		std::vector<std::shared_ptr<AST::ASTClass>> m_classes;
};
