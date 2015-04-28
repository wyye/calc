%{
#include <climits>
#include <cstdlib>
#include <string>
#include "ParserDriver.h"
#include "CalcParser.tab.hh"

#undef yywrap
#define yywrap() 1

static yy::location loc;
%}

%option noyywrap nounput batch debug noinput

%{
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  loc.step ();
%}

[ \t]+ { loc.step (); }
[\n]+ { loc.lines (yyleng); loc.step (); }

"+" { return yy::CalcParser::make_ADD(loc); }
"-" { return yy::CalcParser::make_SUB(loc); }
"*" { return yy::CalcParser::make_MUL(loc); }
"/" { return yy::CalcParser::make_DIV(loc); }
"(" { return yy::CalcParser::make_LPAREN(loc); }
")" { return yy::CalcParser::make_RPAREN(loc); }

"{" { return yy::CalcParser::make_LCURVEPAREN(loc); }
"}" { return yy::CalcParser::make_RCURVEPAREN(loc); }
"!" { return yy::CalcParser::make_NOT(loc); }
":" { return yy::CalcParser::make_COLON(loc); }
";" { return yy::CalcParser::make_SEMICOLON(loc); }
"," { return yy::CalcParser::make_COMMA(loc); }
"=" { return yy::CalcParser::make_ASSIGN(loc); }
"?" { return yy::CalcParser::make_QUESTION(loc); }
">" { return yy::CalcParser::make_GREATER(loc); }
"<" { return yy::CalcParser::make_LESS(loc); }

"==" { return yy::CalcParser::make_EQUAL(loc); }
">=" { return yy::CalcParser::make_GREATEREQUAL(loc); }
"<=" { return yy::CalcParser::make_LESSEQUAL(loc); }
"!=" { return yy::CalcParser::make_NOTEQUAL(loc); }
"++" { return yy::CalcParser::make_INC(loc); }
"--" { return yy::CalcParser::make_DEC(loc); }
"function" { return yy::CalcParser::make_FUNC(loc); }
"while" { return yy::CalcParser::make_WHILE(loc); }
"if" { return yy::CalcParser::make_IF(loc); }
"else" { return yy::CalcParser::make_ELSE(loc); }

[0-9]*\.?[0-9]+ { return yy::CalcParser::make_NUMBER(atof(yytext), loc); }
[a-zA-Z_][a-zA-Z0-9_]* { return yy::CalcParser::make_NAME(yytext, loc); }
. { driver.error(loc, "Invalid character"); }
<<EOF>>    return yy::CalcParser::make_END(loc);
%%
void ParserDriver::scan_begin()
{
	yy_flex_debug = trace_scanning;
	if (!(yyin = fopen(file.c_str(), "r")))
	{
		error("Cannot open " + file);
		exit(-1);
	}
}

void ParserDriver::scan_end ()
{
	fclose (yyin);
	// WARNING if you want use scanner again, don't call yylex_destroy
	yylex_destroy();
}

