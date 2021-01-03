#pragma once
#include <queue>
#include <list>
#include <utility>
#include "parserimpl.hpp"

//class yy::parser::symbol_type;

struct ParserDriver
{
	void* m_code;
	size_t m_idx;
	std::queue<yy::parser::symbol_type> m_fakeSymbols;
	std::vector<std::shared_ptr<AST::ASTStatement>> m_statements;
	std::list<std::pair<size_t, std::string>> m_labelPositions;
	bool m_inConstructor;
	size_t m_remainingConstructorExpressions;

	void Go();
	void NotifyParsedCtorExpression();

	size_t m_nBasicBlocks;
	size_t GetNextBasicBlockNum();
	void DoControlFlowAnalysis();
};