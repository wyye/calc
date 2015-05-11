#include <new>

#include "HelpTools.h"

#include "Interpreter.h"
#include "AbstractSyntaxTree.h"

Interpreter::Interpreter(HashTable* functable, std::map<unsigned int, std::pair<std::string, unsigned int> >* sym_table)
{
	int_state.functable = functable;
	int_state.sym_table = sym_table;
	ParserFunc* pf = int_state.functable->get("main");
	if (pf == NULL) {
		calc_unreachable("Function 'main()' not found");
	}
	exec_state.command = new ASTFuncCallNode("main");
	exec_state.cmd_state = 0;
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
		std::cerr << "Interpreter : Out of memory\n";
		throw;
	}
	double ret = int_state.data_stack.top();
	int_state.data_stack.pop();
	return ret;
}

Interpreter::~Interpreter() {
	delete exec_state.command;
}
