#ifndef PARSER_DRIVER_H
#define PARSER_DRIVER_H

#include <string>
#include "HashTable.h"
#include "CalcParser.tab.hh"
#include "Stack.h"

#define YY_DECL yy::CalcParser::symbol_type yylex (ParserDriver& driver)

YY_DECL;

class ParserDriver
{
	void scan_begin();
	void scan_end();

public:
	ParserDriver() : trace_scanning (false), trace_parsing (false) {}

	HashTable functable;

	std::list<std::map<std::string, unsigned int> > sym_table_stack;
	std::map<unsigned int, std::pair<std::string, unsigned int> > sym_table;

	static unsigned int last_index;

	// set true for debugging
	bool trace_scanning;
	bool trace_parsing;

	std::string file;

	int parse(const std::string& f);

	void error(const yy::location& l, const std::string& m);
	void error(const std::string& m);
};



#endif // PARSER_DRIVER_H
