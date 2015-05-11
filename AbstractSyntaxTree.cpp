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
	int op = get_op();
	IASTNode* left = get();

	if (exec_st.cmd_state == 0) { // calculate index, only for arrays
		if (left->get_op() == INDEX) {
			ASTBinaryOpNode* index = dynamic_cast<ASTBinaryOpNode*>(left);
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			int_st.command_stack.push(exec_st.command);
			exec_st.command = index->get(1);
		} else if (left->get_op() == VARIABLE) { // nothing to do for variables
			exec_st.cmd_state++;
		} else {
			calc_unreachable("Wrong modifiable");
		}
		return;
	}

	if (exec_st.cmd_state == 1) { // calculate and modify value
		double val;
		double new_val;
		std::string var_name;
		unsigned int ind; // only for array indexing, useless in case of variable
		if (left->get_op() == INDEX) { // modify array element
			double ind_d = int_st.data_stack.top();
			int_st.data_stack.pop();
			if (ind_d < 0.0) calc_unreachable("Array index less than zero");
			if (ind_d > 1e9) calc_unreachable("Array index too high");
			ind = static_cast<unsigned int>(ind_d);
			ASTBinaryOpNode* index = dynamic_cast<ASTBinaryOpNode*>(left);
			ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(index->get(0));
			unsigned int var_id = leafvar->get();
			std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator sym_table_it = int_st.sym_table->find(var_id);
			if (sym_table_it == int_st.sym_table->end()) calc_unreachable("Array id not found");
			if (sym_table_it->second.second <= ind) calc_unreachable("Array index out of range");
			std::map<unsigned int, std::vector<double> >::iterator arrays_it = exec_st.arrays->find(var_id);
			if (arrays_it == exec_st.arrays->end()) {
				std::vector<double> temp_array(sym_table_it->second.second, 0.0);
				(*exec_st.arrays)[var_id] = temp_array;
			}
			val = (*exec_st.arrays)[var_id][ind];

			var_name = sym_table_it->second.first;
		} else if (left->get_op() == VARIABLE) { // modify variable
			ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(left);
			unsigned int var_id = leafvar->get();
			std::map<unsigned int, double>::iterator val_iterator = exec_st.variables->find(var_id);
			if (val_iterator == exec_st.variables->end()) calc_unreachable("Variable not initialized");
			val = val_iterator->second;

			std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator sym_table_it = int_st.sym_table->find(var_id);
			var_name = sym_table_it->second.first;
		} else {
			calc_unreachable("Wrong modifiable");
		}

		switch(op)
		{
		case POST_INC:
			new_val = val + 1.0;
			break;
		case PRE_INC:
			new_val = val + 1.0;
			val = new_val;
			break;
		case POST_DEC:
			new_val = val - 1.0;
			break;
		case PRE_DEC:
			new_val = val - 1.0;
			val = new_val;
			break;
		default:
			calc_unreachable("Operation code is not allowed");
		}

		if (left->get_op() == INDEX) {
			ASTBinaryOpNode* index = dynamic_cast<ASTBinaryOpNode*>(left);
			ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(index->get(0));
			unsigned int var_id = leafvar->get();
			std::map<unsigned int, std::vector<double> >::iterator arrays_it = exec_st.arrays->find(var_id);
			arrays_it->second[ind] = new_val;
			std::cout << var_name << "[" << ind << "] = " << new_val << std::endl;
		} else if (left->get_op() == VARIABLE) {
			ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(left);
			unsigned int var_id = leafvar->get();
			std::map<unsigned int, double>::iterator val_iterator = exec_st.variables->find(var_id);
			val_iterator->second = new_val;
			if (var_name == "result") exec_st.result = new_val;
			std::cout << var_name << " = " << new_val << std::endl;
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

	if (exec_st.cmd_state == 1) // call to child 1
	{
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
		}
		exec_st.cmd_state = int_st.op_stack.top();
		int_st.op_stack.pop();
		exec_st.command = int_st.command_stack.top();
		int_st.command_stack.pop();
		return;
	}

	calc_unreachable("Wrong cmd_state");
}

void ASTIndexNode::run(InterpreterState& int_st, ExecutionState& exec_st)
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

	if (exec_st.cmd_state == 1) // return value
	{
		switch(op)
		{
		case INDEX: {
			double ind_d = int_st.data_stack.top();
			int_st.data_stack.pop();
			if (ind_d < 0.0) calc_unreachable("Array index less than zero");
			if (ind_d > 1e9) calc_unreachable("Array index too high");
			unsigned int ind = static_cast<unsigned int>(ind_d);
			ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(get(0));
			unsigned int var_id = leafvar->get();
			std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator sym_table_it = int_st.sym_table->find(var_id);
			if (sym_table_it == int_st.sym_table->end()) calc_unreachable("Array id not found");
			if (sym_table_it->second.second <= ind) calc_unreachable("Array index out of range");
			std::map<unsigned int, std::vector<double> >::iterator arrays_it = exec_st.arrays->find(var_id);
			if (arrays_it == exec_st.arrays->end()) {
				std::vector<double> temp_array(sym_table_it->second.second, 0.0);
				(*exec_st.arrays)[var_id] = temp_array;
			}
			int_st.data_stack.push((*exec_st.arrays)[var_id][ind]);
			break;
		}
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

void ASTAssignNode::run(InterpreterState& int_st, ExecutionState& exec_st)
{
	int op = get_op();
	IASTNode* left = get(0);

	if (exec_st.cmd_state == 0) // call to child 2
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		exec_st.cmd_state = 0;
		int_st.command_stack.push(exec_st.command);
		exec_st.command = get(1);
		return;
	}

	if (exec_st.cmd_state == 1) { // calculate index, only for arrays
		if (left->get_op() == INDEX) {
			ASTBinaryOpNode* index = dynamic_cast<ASTBinaryOpNode*>(left);
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			int_st.command_stack.push(exec_st.command);
			exec_st.command = index->get(1);
		} else if (left->get_op() == VARIABLE) { // nothing to do for variables
			exec_st.cmd_state++;
		} else {
			calc_unreachable("Wrong modifiable");
		}
		return;
	}

	if (exec_st.cmd_state == 2) // assign to lhs and return value
	{
		if (op == ASSIGN) {
			std::string var_name;
			if (left->get_op() == INDEX) { // modify array element
				double ind_d = int_st.data_stack.top();
				int_st.data_stack.pop();
				if (ind_d < 0.0) calc_unreachable("Array index less than zero");
				if (ind_d > 1e9) calc_unreachable("Array index too high");
				unsigned int ind = static_cast<unsigned int>(ind_d);
				ASTBinaryOpNode* index = dynamic_cast<ASTBinaryOpNode*>(left);
				ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(index->get(0));
				unsigned int var_id = leafvar->get();
				std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator sym_table_it = int_st.sym_table->find(var_id);
				if (sym_table_it == int_st.sym_table->end()) calc_unreachable("Array id not found");
				if (sym_table_it->second.second <= ind) calc_unreachable("Array index out of range");
				std::map<unsigned int, std::vector<double> >::iterator arrays_it = exec_st.arrays->find(var_id);
				if (arrays_it == exec_st.arrays->end()) {
					std::vector<double> temp_array(sym_table_it->second.second, 0.0);
					(*exec_st.arrays)[var_id] = temp_array;
				}
				(*exec_st.arrays)[var_id][ind] = int_st.data_stack.top();

				var_name = sym_table_it->second.first;
				std::cout << var_name << "[" << ind << "] = " << int_st.data_stack.top() << std::endl;
			} else if (left->get_op() == VARIABLE) { // modify variable
				ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(left);
				unsigned int var_id = leafvar->get();
				(*exec_st.variables)[var_id] = int_st.data_stack.top();

				std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator sym_table_it = int_st.sym_table->find(var_id);
				var_name = sym_table_it->second.first;
				if (var_name == "result") exec_st.result = int_st.data_stack.top();
				std::cout << var_name << " = " << int_st.data_stack.top() << std::endl;
			} else {
				calc_unreachable("Wrong modifiable");
			}
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		} else {
			calc_unreachable("Unknown operation");
		}
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
	switch (get_op())
	{
	case WHILE_CYCLE: {
		if (exec_st.cmd_state == 0) { // call to child 1 (condition)
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			int_st.command_stack.push(exec_st.command);
			exec_st.command = get(0);
			return;
		} else if (exec_st.cmd_state == 1) { // check condition
			double left = int_st.data_stack.top();
			if (int_st.data_stack.pop()) // drop condition result
				calc_unreachable("data stack is empty");
			if (!double_equal(left, 0.0)) { // condition true
				exec_st.cmd_state = 2; // after calc cycle body go to while-cycle state 2
				int_st.op_stack.push(exec_st.cmd_state);
				exec_st.cmd_state = 0;
				int_st.command_stack.push(exec_st.command);
				exec_st.command = get(1);
				return;
			} else {
				int_st.data_stack.push(0.0); // put useless result of while-cycle in stack
				exec_st.cmd_state = int_st.op_stack.top();
				int_st.op_stack.pop();
				exec_st.command = int_st.command_stack.top();
				int_st.command_stack.pop();
				return;
			}
		} else if (exec_st.cmd_state == 2) { // after calc cycle body
			if (int_st.data_stack.pop()) // drop cycle body result
				calc_unreachable("data stack is empty");
			exec_st.cmd_state = 0; // check condition again
			return;
		} else calc_unreachable("Wrong cmd_state");
		break;
	}
	case STATEMENTS: {
		if (exec_st.cmd_state == 0) { // call to child 1
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			int_st.command_stack.push(exec_st.command);
			exec_st.command = get(0);
			return;
		} else if (exec_st.cmd_state == 1) {
			if (int_st.data_stack.pop()) // drop useless result of left node
				calc_unreachable("data stack is empty");
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			int_st.command_stack.push(exec_st.command);
			exec_st.command = get(1); // call right node
			return;
		} else if (exec_st.cmd_state == 2) { // return
			// don't drop result of right node
			// it wil be returning value of current STATEMENTS node
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		} else calc_unreachable("Wrong cmd_state");
		break;
	}
	default:
		calc_unreachable("Unknown operation");
	}
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
	ParserFunc* f = int_st.functable->get(m_name);
	if (f == NULL) {
		std::string msg = "Function '" + m_name + "' not found in name table";
		calc_unreachable(msg);
	}
	if (f->arg.size() != m_child_args.size()) {
		std::string msg = "Wrong number of arguments in function '" + f->name + "'";
		calc_unreachable(msg);
	}

	if (0 <= exec_st.cmd_state && exec_st.cmd_state < f->arg.size()) // calcualte all arguments, except arrays
	{
		if (f->arg[exec_st.cmd_state]->get_op() == VARIABLE)
		{
			int_st.command_stack.push(exec_st.command);
			exec_st.command = m_child_args[exec_st.cmd_state]; // calculate func arguments
			exec_st.cmd_state++;
			int_st.op_stack.push(exec_st.cmd_state);
			exec_st.cmd_state = 0;
			return;
		} else {
			exec_st.cmd_state++;
			return;
		}
	}

	if (exec_st.cmd_state == f->arg.size()) // func call
	{
		exec_st.cmd_state++;
		int_st.op_stack.push(exec_st.cmd_state);
		int_st.command_stack.push(exec_st.command);
		int_st.var_stack.push(exec_st.variables);
		int_st.arr_stack.push(exec_st.arrays);

		exec_st.cmd_state = 0;
		exec_st.command = f->body;
		exec_st.variables = new std::map<unsigned int, double>;
		std::map<unsigned int, std::vector<double> >* temp_arrays = new std::map<unsigned int, std::vector<double> >;

		for (int i = f->arg.size() - 1; i >= 0; i--) {
			if (f->arg[i]->get_op() == VARIABLE) {
				ASTLeafVar* leafvar = dynamic_cast<ASTLeafVar*>(f->arg[i]);
				unsigned int var_id = leafvar->get();
				(*exec_st.variables)[var_id] = int_st.data_stack.top();
				int_st.data_stack.pop();
			} else { // copy array
				ASTIndexNode* index_in = dynamic_cast<ASTIndexNode*>(f->arg[i]);
				ASTLeafVar* variable_in = dynamic_cast<ASTLeafVar*>(index_in->get(0));
				unsigned int id_in = variable_in->get();

				ASTLeafVar* variable_out = dynamic_cast<ASTLeafVar*>(m_child_args[i]);
				unsigned int id_out = variable_out->get();

				std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator st_it_in = int_st.sym_table->find(id_in);
				std::map<unsigned int, std::pair<std::string, unsigned int> >::iterator st_it_out = int_st.sym_table->find(id_out);

				// check sizes of in and out arrays
				if (st_it_in->second.second != st_it_out->second.second) calc_unreachable("Array has wrong size in function call");

				// fill outer array by zero if it isn't initialized
				std::map<unsigned int, std::vector<double> >::iterator it_out = exec_st.arrays->find(id_out);
				if (it_out == exec_st.arrays->end()) {
					std::vector<double> temp_array(st_it_out->second.second, 0.0);
					(*exec_st.arrays)[id_out] = temp_array;
				}

				// copy array
				(*temp_arrays)[id_in] = (*exec_st.arrays)[id_out];
			}
		}

		exec_st.arrays = temp_arrays;

		return;
	}

	if (exec_st.cmd_state == f->arg.size() + 1) // func return
	{
		double res = exec_st.result;
		delete exec_st.variables;
		delete exec_st.arrays;
		int_st.data_stack.pop(); // if function has one statement, delete it result, else delete result of last statement
		int_st.data_stack.push(res);
		if (f->name == "main") {
			int_st.execution_end = 1;
			return;
		}
		exec_st.variables = int_st.var_stack.top();
		int_st.var_stack.pop();
		exec_st.arrays = int_st.arr_stack.top();
		int_st.arr_stack.pop();
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

ISSANode* ASTAssignNode::make_ssa(SSAList& ssa)
{
	if (get_op() == ASSIGN) {
		if (!dynamic_cast<ASTLeafVar*>(get(0))) calc_unreachable("Not implemented");
		ISSANode* left = get(0)->make_ssa(ssa);
		ISSANode* right = get(1)->make_ssa(ssa);
		ssa.make_assign(left, right);
		return get(0)->make_ssa(ssa);
	} else {
		calc_unreachable("Unknown operation");
		return NULL;
	}
}
