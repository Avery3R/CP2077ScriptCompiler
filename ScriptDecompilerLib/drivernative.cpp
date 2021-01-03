#include "driver.hpp"
#include <unordered_map>
#include <algorithm>

void ParserDriver::NotifyParsedCtorExpression()
{
	--m_remainingConstructorExpressions;
}

struct BasicBlock
{
	std::vector<std::weak_ptr<BasicBlock>> parents;
	std::vector<std::shared_ptr<BasicBlock>> children;

	size_t num;
	std::vector<std::shared_ptr<AST::ASTStatement>> stmts;
	bool isLoopEnd;
};

std::vector<std::shared_ptr<BasicBlock>> BlocksThatReachBlockFromBlockRecurse(std::shared_ptr<BasicBlock> target, std::shared_ptr<BasicBlock> potentialParent, std::vector<bool> &visitedBlocks)
{
	std::vector<std::shared_ptr<BasicBlock>> ret;
	
	if(potentialParent->num+1 > visitedBlocks.size())
	{
		visitedBlocks.resize(potentialParent->num+1);
	}

	if(visitedBlocks[potentialParent->num])
	{
		return ret;
	}

	visitedBlocks[potentialParent->num] = true;

	for(auto &child : potentialParent->children)
	{
		if(child == target)
		{
			ret.emplace_back(potentialParent);
		}
		else
		{
			auto tmp = BlocksThatReachBlockFromBlockRecurse(target, child, visitedBlocks);
			ret.insert(ret.end(), tmp.begin(), tmp.end());
		}
	}

	return ret;

}

std::vector<std::shared_ptr<BasicBlock>> BlocksThatReachBlockFromBlock(std::shared_ptr<BasicBlock> target, std::shared_ptr<BasicBlock> potentialParent)
{
	std::vector<bool> visitedBlocks;
	return BlocksThatReachBlockFromBlockRecurse(target, potentialParent, visitedBlocks);
}

void FillBlock(ParserDriver &drv, std::shared_ptr<BasicBlock> block, size_t startingStmtNum, std::vector<std::shared_ptr<BasicBlock>> &stmt2BasicBlock,
	std::unordered_map<std::string, size_t> &label2stmtIdx)
{
	for(size_t i = startingStmtNum; i < drv.m_statements.size(); ++i)
	{
		if(stmt2BasicBlock[i])
		{
			return;
		}

		auto &stmt = drv.m_statements[i];
		if(typeid(*stmt) == typeid(AST::LabelStatement) && i != startingStmtNum)
		{
			auto lblStmt = std::dynamic_pointer_cast<AST::LabelStatement>(stmt);

			auto nextBlock = stmt2BasicBlock[i];
			if(!nextBlock)
			{
				nextBlock = std::make_shared<BasicBlock>();
				nextBlock->num = drv.GetNextBasicBlockNum();
				FillBlock(drv, nextBlock, i, stmt2BasicBlock, label2stmtIdx);
			}
			nextBlock->parents.emplace_back(block);

			block->children.emplace_back(nextBlock);

			return;
		}

		block->stmts.emplace_back(stmt);
		stmt2BasicBlock[i] = block;
		if(typeid(*stmt) == typeid(AST::JumpZeroStatement))
		{
			auto jzStmt = std::dynamic_pointer_cast<AST::JumpZeroStatement>(stmt);

			auto falseBlock = stmt2BasicBlock[label2stmtIdx[jzStmt->GetLabel()]];
			if(!falseBlock)
			{
				falseBlock = std::make_shared<BasicBlock>();
				falseBlock->num = drv.GetNextBasicBlockNum();
				FillBlock(drv, falseBlock, label2stmtIdx[jzStmt->GetLabel()], stmt2BasicBlock, label2stmtIdx);
			}
			falseBlock->parents.emplace_back(block);

			auto trueBlock = stmt2BasicBlock[i+1];
			if(!trueBlock)
			{
				trueBlock = std::make_shared<BasicBlock>();
				trueBlock->num = drv.GetNextBasicBlockNum();
				FillBlock(drv, trueBlock, i+1, stmt2BasicBlock, label2stmtIdx);
			}
			trueBlock->parents.emplace_back(block);

			//first child must always be the true
			block->children.emplace_back(trueBlock);
			block->children.emplace_back(falseBlock);

			return;
		}
		else if(typeid(*stmt) == typeid(AST::GotoStatement))
		{
			auto gotoStmt = std::dynamic_pointer_cast<AST::GotoStatement>(stmt);

			auto nextBlock = stmt2BasicBlock[label2stmtIdx[gotoStmt->GetLabel()]];
			if(!nextBlock)
			{
				nextBlock = std::make_shared<BasicBlock>();
				nextBlock->num = drv.GetNextBasicBlockNum();
				FillBlock(drv, nextBlock, label2stmtIdx[gotoStmt->GetLabel()], stmt2BasicBlock, label2stmtIdx);
			}
			nextBlock->parents.emplace_back(block);

			block->children.emplace_back(nextBlock);

			return;
		}
		else if(typeid(*stmt) == typeid(AST::ReturnStatement))
		{
			return;
		}
	}
}

size_t ParserDriver::GetNextBasicBlockNum()
{
	return m_nBasicBlocks++;
}

std::vector<std::shared_ptr<AST::ASTStatement>> ReconstructFlow(std::shared_ptr<BasicBlock> block)
{
	std::vector<std::shared_ptr<AST::ASTStatement>> ret;

	//size_t numLoopParents = 0;

	std::vector<std::pair<std::shared_ptr<BasicBlock>, std::vector<std::shared_ptr<BasicBlock>>>> loopParents;

	if(block->parents.size() > 0)
	{
		for(auto &child : block->children)
		{
			auto tmp = BlocksThatReachBlockFromBlock(block, child);
			if(tmp.size() != 0)
			{
				loopParents.emplace_back(child, tmp);
			}
		}
	}

	if
	(
		loopParents.size() == 1 //we have only 1 child that loops back to us
		&& block->children.size() == 2 //we have two children
		&& block->stmts.size() == 2 //we have 2 statements
		&& typeid(*block->stmts[0]) == typeid(AST::LabelStatement) //our first statement is a label
		&& typeid(*block->stmts[1]) == typeid(AST::JumpZeroStatement) //our second statement is a __jz
	) //we're in a while loop
	{
		auto jzStmt = std::dynamic_pointer_cast<AST::JumpZeroStatement>(block->stmts[1]);

		for(size_t i = 0; i < loopParents[0].second.size(); ++i)
		{
			loopParents[0].second[i]->isLoopEnd = true;
		}

		auto loopStatements = ReconstructFlow(loopParents[0].first);

		ret.emplace_back(block->stmts[0]);
		auto whileStmt = std::make_shared<AST::WhileStatement>(jzStmt->GetCondition(), loopStatements);
		ret.emplace_back(whileStmt);

		for(size_t i = 0; i < 2; ++i)
		{
			if(block->children[i] != loopParents[0].first)
			{
				auto postLoopStatements = ReconstructFlow(block->children[i]);
				ret.insert(ret.end(), postLoopStatements.begin(), postLoopStatements.end());
				break;
			}
		}
	}
	else if
	(
		//loopParents.size() == 0 //we're not in a loop or we only have one parent
		block->children.size() == 2 //we have two children
		//&& block->children[0]->children.size() <= 1 //the first child has 0 or 1 children
		//&& block->children[1]->children.size() == block->children[0]->children.size() //both children have the same number of children
		&& typeid(*block->stmts[block->stmts.size()-1]) == typeid(AST::JumpZeroStatement) //the last statement in this block is a __jz
	)
	{ //then we're in an if(){}[else{}] statement
		for(size_t i = 0; i < block->stmts.size()-1; ++i)
		{
			ret.emplace_back(block->stmts[i]);
		}

		auto jzStmt = std::dynamic_pointer_cast<AST::JumpZeroStatement>(block->stmts[block->stmts.size()-1]);

		//first child must always be the true
		auto trueStmts = ReconstructFlow(block->children[0]);
		auto falseStmts = ReconstructFlow(block->children[1]);

		auto ifStmt = std::make_shared<AST::IfStatement>(jzStmt->GetCondition(), trueStmts, falseStmts);
		ret.emplace_back(ifStmt);
	}
	else
	{
		for(size_t i = 0; i < block->stmts.size(); ++i)
		{
			ret.emplace_back(block->stmts[i]);
		}

		if(!block->isLoopEnd)
		{
			//if(loopParents.size() == 0)
			{
				for(size_t i = 0; i < block->children.size(); ++i)
				{
					//this is going to break for loops
					auto childrenStmts = ReconstructFlow(block->children[i]);
					ret.insert(ret.end(), childrenStmts.begin(), childrenStmts.end());
				}
			}
			/*else
			{
				std::cout << "// Warning, default block handler on loop?? Block # " << block->num << std::endl;
			}*/
		}
	}

	return ret;
}

void ParserDriver::DoControlFlowAnalysis()
{
	if(m_statements.size() == 0)
	{
		return;
	}

	std::unordered_map<std::string, size_t> label2stmtIdx;

	for(size_t i = 0; i < m_statements.size(); ++i)
	{
		auto &stmt = m_statements[i];
		if(typeid(*stmt) == typeid(AST::LabelStatement))
		{
			auto labelStmt = std::dynamic_pointer_cast<AST::LabelStatement>(stmt);

			label2stmtIdx[labelStmt->GetLabel()] = i;
		}
	}

	auto rootBlock = std::make_shared<BasicBlock>();
	rootBlock->num = GetNextBasicBlockNum();

	std::vector<std::shared_ptr<BasicBlock>> stmt2BasicBlock(m_statements.size());

	FillBlock(*this, rootBlock, 0, stmt2BasicBlock, label2stmtIdx);
	std::vector<std::shared_ptr<BasicBlock>> blocks(m_nBasicBlocks);

	for(size_t i = 0; i < stmt2BasicBlock.size(); ++i)
	{
		if(stmt2BasicBlock[i])
		{
			blocks[stmt2BasicBlock[i]->num] = stmt2BasicBlock[i];
		}
	}

	for(size_t i = 0; i < blocks.size(); ++i)
	{
		std::cout << "// Basic block " << blocks[i]->num << " children: ";
		for(size_t j = 0; j < blocks[i]->children.size(); ++j)
		{
			if(j != 0)
			{
				std::cout << ", ";
			}
			std::cout << blocks[i]->children[j]->num;
		}
		std::cout << " parents: ";
		for(size_t j = 0; j < blocks[i]->parents.size(); ++j)
		{
			if(j != 0)
			{
				std::cout << ", ";
			}
			std::cout << blocks[i]->parents[j].lock()->num;
		}
		std::cout << std::endl;

		for(size_t j = 0; j < blocks[i]->stmts.size(); ++j)
		{
			std::cout << blocks[i]->stmts[j]->PrettyPrint() << '\n';
		}
		std::cout << std::endl;
	}

	m_statements = ReconstructFlow(rootBlock);

	return;
}