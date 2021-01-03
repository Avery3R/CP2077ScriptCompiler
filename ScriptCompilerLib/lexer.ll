%option nounistd
%option never-interactive
%option noyywrap

%{
#include "driver.hpp"
#include "parserimpl.hpp"

#define YY_DECL yy::parser::symbol_type yylex(ParserDriver &driver)

#define yyterminate() return yy::parser::make_YYEOF(driver.m_loc);

#define YY_USER_ACTION driver.m_loc.step(); driver.m_loc.columns(yyleng);
%}

%%

[ ]+                   driver.m_loc.step();
\t+                    driver.m_loc.step();
\r                     driver.m_loc.step();
\n                     driver.m_loc.lines(yyleng); driver.m_loc.step();
import                 return yy::parser::make_TOKEN_IMPORT(driver.m_loc);
class                  return yy::parser::make_TOKEN_CLASS(driver.m_loc);
var                    return yy::parser::make_TOKEN_VAR(driver.m_loc);
ref                    return yy::parser::make_TOKEN_REF(driver.m_loc);
new                    return yy::parser::make_TOKEN_NEW(driver.m_loc);
null                   return yy::parser::make_TOKEN_NULL(driver.m_loc);
function               return yy::parser::make_TOKEN_FUNCTION(driver.m_loc);
exec                   return yy::parser::make_TOKEN_EXEC(driver.m_loc);
event                  return yy::parser::make_TOKEN_EVENT(driver.m_loc);
public                 return yy::parser::make_TOKEN_PUBLIC(driver.m_loc);
protected              return yy::parser::make_TOKEN_PROTECTED(driver.m_loc);
private                return yy::parser::make_TOKEN_PRIVATE(driver.m_loc);
inject                 return yy::parser::make_TOKEN_INJECT(driver.m_loc);
static                 return yy::parser::make_TOKEN_STATIC(driver.m_loc);
native                 return yy::parser::make_TOKEN_NATIVE(driver.m_loc);
this                 return yy::parser::make_TOKEN_THIS(driver.m_loc);
\*=                    return yy::parser::make_TOKEN_OP_ASSIGN_MULT(driver.m_loc);
[0-9]+\.[0-9]*f        return yy::parser::make_TOKEN_FLOAT(strtof(yytext, nullptr), driver.m_loc);
[0-9]+i64              return yy::parser::make_TOKEN_INT64(strtoll(yytext, nullptr, 10), driver.m_loc);
[0-9]+i32              return yy::parser::make_TOKEN_INT32(strtol(yytext, nullptr, 10), driver.m_loc);
[0-9]+i16              return yy::parser::make_TOKEN_INT16((int16_t)strtol(yytext, nullptr, 10), driver.m_loc);
[0-9]+i8               return yy::parser::make_TOKEN_INT8((int8_t)strtol(yytext, nullptr, 10), driver.m_loc);
[0-9]                  throw yy::parser::syntax_error(driver.m_loc, std::string("Numbers need a i64/i32/i16/i8 postfix ") + yytext);
[_a-zA-Z][_a-zA-Z0-9]* return yy::parser::make_TOKEN_IDENTIFIER(yytext, driver.m_loc);
.                      return yy::parser::symbol_type(*yytext, driver.m_loc);

%%

void LexScanString(const std::string &str)
{
	yy_scan_string(str.c_str());
}