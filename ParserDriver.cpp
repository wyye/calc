#include "ParserDriver.h"
#include "CalcParser.tab.hh"

unsigned int ParserDriver::last_index = 0;

int ParserDriver::parse (const std::string &f)
{
	file = f;
	scan_begin ();
	yy::CalcParser parser(*this);
	parser.set_debug_level(trace_parsing);
	int res = parser.parse();
	scan_end();
	return res;
}

void ParserDriver::error (const yy::location& l, const std::string& m)
{
	std::cerr << l << " : " << m << std::endl;
	exit(-1);
}

void ParserDriver::error (const std::string& m)
{
	std::cerr << m << std::endl;
	exit(-1);
}
