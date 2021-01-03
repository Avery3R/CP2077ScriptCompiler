#include "driver.hpp"

extern void LexScanString(const std::string &str);

ParserDriver::ParserDriver() :
	m_loc(), m_functions(), m_imports(), m_classes()
{
}

int ParserDriver::Go(const std::string &data)
{
	LexScanString(data);
	yy::parser parser(*this);
	//parser.set_debug_stream(std::cout);
	//parser.set_debug_level(1);
	return parser.parse();
}