#include <new>

#include "Interpreter.h"
#include "AbstractSyntaxTree.h"

Interpreter::Interpreter(HashTable* functable)
{
	int_state.functable = functable;
	ParserFunc* pf = int_state.functable->get("main");
	if (pf == NULL) {
		printf("Interpreter : Function 'main()' not found\n");
		exit(-1);
	}
	exec_state.command = new ASTFuncCallNode("main");
	exec_state.cmd_state = 0;
	exec_state.variables = NULL;
	int_state.execution_end = 0;
}

double Interpreter::run() {
	try {
		while (!int_state.execution_end) {
			//int_state.data_stack.print();
			exec_state.command->run(int_state, exec_state);
		}
	}
	catch (const std::bad_alloc&)
	{
		printf("Interpreter : Out of memory\n");
		throw;
	}
	double ret = int_state.data_stack.top();
	int_state.data_stack.pop();
	return ret;
}

Interpreter::~Interpreter() {
	delete exec_state.command;
}
