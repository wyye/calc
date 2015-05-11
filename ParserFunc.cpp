#include "ParserFunc.h"
#include "AbstractSyntaxTree.h"

ParserFunc::~ParserFunc()
{
	if (body != NULL) {
		delete body;
		body = NULL;
	}
	for (std::vector<IASTNode*>::iterator it = arg.begin(); it != arg.end(); ++it) {
		delete *it;
	}
}
