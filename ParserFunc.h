#ifndef PARSERFUNC_H
#define PARSERFUNC_H

#include <cstdio>
#include <vector>
#include <string>

class IASTNode;

struct ParserFunc
{
	std::string name;
	int args_num;
	std::vector<std::string> arg;
	IASTNode* body;
	ParserFunc() : args_num(0), body(NULL) {}
	~ParserFunc();
};

#endif // PARSERFUNC_H
