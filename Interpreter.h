#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "HashTable.h"
#include "BinarySearchTree.h"
#include "ParserFunc.h"
#include "Stack.h"

struct ExecutionState
{
	IASTNode* command;
	unsigned int cmd_state;
	BinarySearchTree* variables;
};

struct InterpreterState
{
	HashTable* functable;
	Stack<BinarySearchTree*> var_stack;
	Stack<int> op_stack;
	Stack<double> data_stack;
	Stack<IASTNode*> command_stack;
	int execution_end;
};

class Interpreter
{
	ExecutionState exec_state;
	InterpreterState int_state;
	Interpreter(const Interpreter&);
	const Interpreter& operator=(const Interpreter&);
public:
	Interpreter(HashTable* functable);
	~Interpreter();
	double run();
};

#endif
