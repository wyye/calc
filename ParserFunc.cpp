#include "ParserFunc.h"
#include "AbstractSyntaxTree.h"

ParserFunc::~ParserFunc()
{
	if (body != NULL) {
		delete body;
		body = NULL;
	}
}
