#include "clrpch.hpp"
#include "driver.hpp"
#include "parserimpl.hpp"
#include <ASTUtils.hpp>
#include <sstream>

using namespace System::Runtime::InteropServices;
using namespace Gibbed::RED4::ScriptFormats;
using namespace System::Collections::Generic;
using namespace Avery3R::RED4Scripts;

yy::parser::symbol_type yylex(ParserDriver &driver)
{
	if(!driver.m_fakeSymbols.empty())
	{
		auto ret = driver.m_fakeSymbols.front();
		driver.m_fakeSymbols.pop();
		return ret;
	}

	if(driver.m_inConstructor)
	{
		if(driver.m_remainingConstructorExpressions == 0)
		{
			driver.m_inConstructor = false;
			return yy::parser::make_TOK_CTOR_PARAMEND();
		}
	}

	for(auto it = driver.m_labelPositions.begin(); it != driver.m_labelPositions.end(); ++it)
	{
		if(driver.m_idx == it->first)
		{
			auto ret = yy::parser::make_TOK_LABEL(it->second);
			driver.m_labelPositions.erase(it);
			return ret;
		}
	}

	auto codeHandle = GCHandle::FromIntPtr(System::IntPtr(driver.m_code));

	auto code = (List<Definitions::Instruction>^)codeHandle.Target;

	if(driver.m_idx == code->Count)
	{
		return yy::parser::make_YYEOF();
	}

	auto instr = code[int(driver.m_idx++)];

	switch(instr.Opcode)
	{
		case Opcode::Assign:
			return yy::parser::make_OP_ASSIGN();
		case Opcode::FinalFunc:
		{
			auto sinstr = (Instructions::FinalFunc^)instr.Argument;
			auto func = sinstr->Function;

			std::string parentName = "";

			if(func->Parent != nullptr && func->Flags.HasFlag(Definitions::FunctionFlags::IsStatic))
			{
				parentName = AST::NetToCppString(func->Parent->ToName());
			}
			return yy::parser::make_OP_FINAL_FUNC(std::make_pair(parentName, AST::NetToCppString(func->ToName())));
		}
		case Opcode::LocalVar:
		case Opcode::ParamVar:
		{
			auto def = (Definitions::FunctionVarDefinition^)instr.Argument;
			return yy::parser::make_TOK_IDENTIFIER(AST::NetToCppString(def->Name));
		}
		case Opcode::ParamEnd:
			return yy::parser::make_OP_PARAM_END();
		case Opcode::Return:
			return yy::parser::make_OP_RETURN();
		case Opcode::Nop:
			return yy::parser::make_OP_NOP();
		case Opcode::Context:
			return yy::parser::make_OP_CONTEXT();
		case Opcode::JumpIfFalse:
		{
			auto sinstr = (int^)instr.Argument;
			//driver.m_ifEndPositions.push_back(size_t(*sinstr));
			std::stringstream labelName;
			labelName << "label_" << size_t(*sinstr);
			return yy::parser::make_OP_JUMP_IF_FALSE(labelName.str());
		}
		case Opcode::Jump:
		{
			auto sinstr = (int^)instr.Argument;
			//driver.m_ifEndPositions.push_back(size_t(*sinstr));
			std::stringstream labelName;
			labelName << "label_" << size_t(*sinstr);
			return yy::parser::make_OP_JUMP(labelName.str());
		}
		case Opcode::StructMember:
		{
			auto sinstr = (Definition^)instr.Argument;
			//driver.m_ifEndPositions.push_back(size_t(*sinstr));
			return yy::parser::make_OP_STRUCT_MEMBER(AST::NetToCppString(sinstr->Name));
		}
		case Opcode::Constructor:
		{
			auto sinstr = (Instructions::Constructor^)instr.Argument;
			driver.m_inConstructor = true;
			driver.m_remainingConstructorExpressions = sinstr->ParameterCount;
			return yy::parser::make_OP_CONSTRUCTOR(AST::NetToCppString(sinstr->Type->Name));
		}
		case Opcode::Int32Zero:
			return yy::parser::make_OP_INT32_ZERO();
		case Opcode::Int32One:
			return yy::parser::make_OP_INT32_ONE();
		case Opcode::ArrayPushBack:
			return yy::parser::make_OP_ARRAY_PUSH_BACK();
		case Opcode::ArrayElement:
			return yy::parser::make_OP_ARRAY_ELEMENT();
		case Opcode::ArraySize:
			return yy::parser::make_OP_ARRAY_SIZE();
		case Opcode::Skip: //No idea what skip does, but maybe we can just safely skip it?
			return yylex(driver);
		case Opcode::TestNotEqual:
			return yy::parser::make_OP_TEST_NOT_EQUAL();
		case Opcode::NameConst:
		{
			auto sinstr = (String^)instr.Argument;
			return yy::parser::make_OP_NAME_CONST(AST::NetToCppString(sinstr));
		}
	}

	std::cout << "error, unhandled opcode: " << AST::NetToCppString(Enum::GetName(instr.Opcode)) << std::endl;


	return yy::parser::make_YYerror();
}

void LocateJumps(ParserDriver &drv)
{
	auto codeHandle = GCHandle::FromIntPtr(System::IntPtr(drv.m_code));

	auto code = (List<Definitions::Instruction>^)codeHandle.Target;

	for(size_t i = 0; i < code->Count; ++i)
	{
		auto instr = code[int(i)];
		switch(instr.Opcode)
		{
			case Opcode::JumpIfFalse:
			{
				auto sinstr = (int^)instr.Argument;
				std::stringstream labelName;
				size_t labelLoc = size_t(*sinstr);
				labelName << "label_" << labelLoc;
				auto existingLabel = std::find_if(drv.m_labelPositions.begin(), drv.m_labelPositions.end(), [labelLoc](std::pair<size_t, std::string> e) -> bool{return e.first == labelLoc;});
				if(existingLabel == drv.m_labelPositions.end())
				{
					drv.m_labelPositions.emplace_back(labelLoc, labelName.str());
				}
			}
			break;
			case Opcode::Jump:
			{
				auto sinstr = (int^)instr.Argument;
				std::stringstream labelName;
				size_t labelLoc = size_t(*sinstr);
				labelName << "label_" << labelLoc;
				auto existingLabel = std::find_if(drv.m_labelPositions.begin(), drv.m_labelPositions.end(), [labelLoc](std::pair<size_t, std::string> e) -> bool{return e.first == labelLoc;});
				if(existingLabel == drv.m_labelPositions.end())
				{
					drv.m_labelPositions.emplace_back(labelLoc, labelName.str());
				}
			}
			break;
		}
	}
}

void ParserDriver::Go()
{
	LocateJumps(*this);

	yy::parser parser(*this);
	//parser.set_debug_stream(std::cout);
	//parser.set_debug_level(1);
	parser.parse();
}