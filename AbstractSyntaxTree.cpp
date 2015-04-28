#include "AbstractSyntaxTree.h"
#include "ParserFunc.h"

#include <cfloat>
#include <cmath>

#include "HelpTools.h"

int double_equal(double a, double b)
{
	if (fabs(a - b) < DBL_EPSILON) return 1;
	return 0;
}

void ASTUnaryOpNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	if (exec_st.cmd_state == 0) // calculate child, he will return value in data stack
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get();
		return;
	}

	if (exec_st.cmd_state == 1) // return value in stack
	{
		int op = get_op();
		double ret = int_st.data_stack.top();
		int_st.data_stack.pop();
		switch(op)
		{
		case UNARY_MINUS:
			int_st.data_stack.push(-ret);
			break;
		case NOT:
			if (double_equal(ret, 0.0)) int_st.data_stack.push(1.0);
			else int_st.data_stack.push(0.0);
			break;
		default:
			calc_unreachable("Operation code is not allowed");
		}
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTIncrOpNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	if (exec_st.cmd_state == 0)
	{
		int op = get_op();
		char var_name[max_str_size];
		ASTLeafVar* leafvar = get();
		leafvar->get(var_name, max_str_size);
		double val = exec_st.variables->get(var_name);
		switch(op)
		{
		case POST_INC:
			exec_st.variables->put(var_name, val + 1.0);
			std::cout << var_name << " = " << val + 1.0 << "\n";
			break;
		case PRE_INC:
			val += 1.0;
			exec_st.variables->put(var_name, val);
			std::cout << var_name << " = " << val << "\n";
			break;
		case POST_DEC:
			exec_st.variables->put(var_name, val - 1.0);
			std::cout << var_name << " = " << val - 1.0 << "\n";
			break;
		case PRE_DEC:
			val -= 1.0;
			exec_st.variables->put(var_name, val);
			std::cout << var_name << " = " << val << "\n";
			break;
		default:
			calc_unreachable("Operation code is not allowed");
		}
		int_st.data_stack.push(val);
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTBinaryOpNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	int op = get_op();

	if (exec_st.cmd_state == 0) // call to child 2
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get(1);
		return;
	}

	if (exec_st.cmd_state == 1) // call to child 1 (if op == ASSIGN return value)
	{
		if (op == ASSIGN) {
			char var_name[max_str_size];
			ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*> (get(0));
			leafvar->get(var_name, max_str_size);
			exec_st.variables->put(var_name, int_st.data_stack.top());
			std::cout << var_name << " = " << int_st.data_stack.top() << "\n";
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		}
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get(0);
		return;
	}

	if (exec_st.cmd_state == 2) // return value
	{
		double left = int_st.data_stack.top();
		int_st.data_stack.pop();
		double right = int_st.data_stack.top();
		int_st.data_stack.pop();

		switch(op)
		{
		case EQUALITY:
			if (double_equal(left, right)) int_st.data_stack.push(1.0);
			else int_st.data_stack.push(0.0);
			break;
		case NEQUALITY:
			if (double_equal(left, right)) int_st.data_stack.push(0.0);
			else int_st.data_stack.push(1.0);
			break;
		case GREATER:
			if (left > right) int_st.data_stack.push(1.0);
			else int_st.data_stack.push(0.0);
			break;
		case GREATER_EQUAL:
			if (left > right || double_equal(left, right)) int_st.data_stack.push(1.0);
			else int_st.data_stack.push(0.0);
			break;
		case LESS:
			if (left < right) int_st.data_stack.push(1.0);
			else int_st.data_stack.push(0.0);
			break;
		case LESS_EQUAL:
			if (left < right || double_equal(left, right)) int_st.data_stack.push(1.0);
			else int_st.data_stack.push(0.0);
			break;
		case ADD:
			int_st.data_stack.push(left + right);
			break;
		case SUB:
			int_st.data_stack.push(left - right);
			break;
		case MUL:
			int_st.data_stack.push(left * right);
			break;
		case DIV:
			if (double_equal(right, 0.0)) {
				calc_unreachable("Division by zero");
			}
			int_st.data_stack.push(left / right);
			break;
		default:
			calc_unreachable("Operation code is not allowed");
			exit(-1);
		}
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTTernaryOpNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	if (exec_st.cmd_state == 0) // call to child 1
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get(0);
		return;
	}

	if (exec_st.cmd_state == 1) // call to child 2 or 3
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		int op = get_op();
		switch(op)
		{
		case TERNARY: {
			double left = int_st.data_stack.top();
			int_st.data_stack.pop();
			if (double_equal(left, 0.0)) exec_st.command = get(2);
			else exec_st.command = get(1);
			break;
		}
		default:
			calc_unreachable("Operation code is not allowed");
		}
		return;
	}

	if (exec_st.cmd_state == 2) // return result
	{
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTNoRetBinaryOpNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	if (exec_st.cmd_state == 0) // call to child 1
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get(0);
		return;
	}

	if (exec_st.cmd_state == 1)
	{
		int op = get_op();
		switch(op)
		{
		case WHILE_CYCLE: {
			double left = int_st.data_stack.top();
			if (int_st.data_stack.pop()) // delete useless condition result
				calc_unreachable("data stack is empty");
			if (!double_equal(left, 0.0)) {
				int_st.data_stack.pop(); // delete statements returned value from previous cycle execution
				exec_st.cmd_state = 0; // when execute node, check condition again
				int_st.op_stack.push(exec_st.cmd_state);
				exec_st.cmd_state = 0;
				int_st.command_stack.push(exec_st.command);
				exec_st.command = get(1);
				return;
			}
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		}
		case STATEMENTS: {
			if (int_st.data_stack.pop()) // drop useless result
				calc_unreachable("data stack is empty");
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			int_st.command_stack.push(exec_st.command);
			exec_st.command = get(1);
			return;
		}
		default:
			calc_unreachable("Operation code is not allowed");
		}
	}

	if (exec_st.cmd_state == 2) // return
	{
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTNoRetTernaryOpNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	if (exec_st.cmd_state == 0) // call to child 1
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get(0);
		return;
	}

	if (exec_st.cmd_state == 1) // call to child 2 or 3
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		int op = get_op();
		switch(op)
		{
		case IF: {
			double left = int_st.data_stack.top();
			int_st.data_stack.pop();
			if (double_equal(left, 0.0)) exec_st.command = get(2);
			else exec_st.command = get(1);
			break;
		}
		default:
			calc_unreachable("Operation code is not allowed");
		}
		return;
	}

	if (exec_st.cmd_state == 2) // return useless result
	{
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTFuncCallNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	ParserFunc* f = int_st.functable->get(m_name.c_str());
	if (f == NULL) {
		std::cout << "ASTFuncCallNode : Function '" << m_name << "' not found in name table\n";
		exit(-1);
	}
	if (f->arg.size() != m_child_args.size()) {
		std::cout << "ASTFuncCallNode : Wrong number of arguments in function '" << f->name << "'\n";
		exit(-1);
	}

	if (0 <= exec_st.cmd_state && exec_st.cmd_state < f->arg.size()) // call all arguments
	{
		int_st.command_stack.push(exec_st.command);
		exec_st.command = m_child_args[exec_st.cmd_state]; // calculate func arguments
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		return;
	}

	if (exec_st.cmd_state == f->arg.size()) // func call
	{
		if (f->name == "main") {
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			int_st.command_stack.push(exec_st.command);
			exec_st.cmd_state = 0;
			exec_st.command = f->body;
			exec_st.variables = new BinarySearchTree;
			return;
		}

		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		int_st.command_stack.push(exec_st.command);
		int_st.var_stack.push(exec_st.variables);

		exec_st.cmd_state = 0;
		exec_st.command = f->body;
		exec_st.variables = new BinarySearchTree;

		for (int i = f->arg.size() - 1; i >= 0; i--) {
			exec_st.variables->put(f->arg[i].c_str(), int_st.data_stack.top());
			int_st.data_stack.pop();
		}

		return;
	}

	if (exec_st.cmd_state == f->arg.size() + 1) // func return
	{
		double res = exec_st.variables->get("result");
		delete exec_st.variables;
		int_st.data_stack.pop(); // if function has one statement, delete it result, else delete result of last statement
		int_st.data_stack.push(res);
		if (f->name == "main") {
			int_st.execution_end = 1;
			return;
		}
		exec_st.variables = int_st.var_stack.top();
		int_st.var_stack.pop();
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTEmptyNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	int_st.data_stack.push(0.0); // push in stack useless result
	exec_st.cmd_state = int_st.op_stack.top();
	int_st.op_stack.pop();
	exec_st.command = int_st.command_stack.top();
	int_st.command_stack.pop();
	return;
}
