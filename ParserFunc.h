#ifndef PARSERFUNC_H
#define PARSERFUNC_H

#include <cstdio>
#include <vector>
#include <string>

class IASTNode;

struct ParserFunc
{
	std::string name;
	std::vector<IASTNode*> arg;
	IASTNode* body;

	ParserFunc() : body(NULL) {}
	~ParserFunc();
};

#endif // PARSERFUNC_H
