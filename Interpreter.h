#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "HashTable.h"
#include "BinarySearchTree.h"
#include "ParserFunc.h"
#include "Stack.h"

#include <map>
#include <string>
#include <vector>

struct ExecutionState
{
	IASTNode* command;
	unsigned int cmd_state;
	std::map<unsigned int, double>* variables;
	std::map<unsigned int, std::vector<double> >* arrays;
	double result; // result of last function call, existence of result assignment checked by parser
};

struct InterpreterState
{
	HashTable* functable;
	std::map<unsigned int, std::pair<std::string, unsigned int> >* sym_table;

	Stack<std::map<unsigned int, double>*> var_stack;
	Stack<std::map<unsigned int, std::vector<double> >*> arr_stack;
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
	Interpreter(HashTable* functable, std::map<unsigned int, std::pair<std::string, unsigned int> >* sym_table);
	~Interpreter();
	double run();
};

#endif
