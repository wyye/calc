#ifndef ABSTRACT_SYNTAX_TREE_H

#define ABSTRACT_SYNTAX_TREE_H

#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>
#include <map>
#include <list>

#include "HashTable.h"
#include "BinarySearchTree.h"
#include "Interpreter.h"
#include "SSA.h"

#include "HelpTools.h"

static const int max_str_size = 256;

enum operation {
		UNKNOWN, EMPTY,
		FUNC_CALL,
		STATEMENTS, WHILE_CYCLE, IF,
		ASSIGN,
		TERNARY, EQUALITY, NEQUALITY,
		GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
		ADD, SUB, MUL, DIV, UNARY_MINUS, NOT,
		POST_INC, PRE_INC, POST_DEC, PRE_DEC,
		INDEX,
		VARIABLE, NUMBER
	};

int double_equal(double a, double b);

class IASTNode {
	int m_op;

	IASTNode& operator = (const IASTNode& rhs);
	IASTNode(const IASTNode& rhs);
public:
	IASTNode(int operation) : m_op (operation) {}
	virtual ~IASTNode() {};
	int get_op() const { return m_op; }
	void set_op(int operation) { m_op = operation; }
	virtual void run(InterpreterState&, ExecutionState&) = 0;
	virtual ISSANode* make_ssa(SSAList& ssa) = 0;
	virtual void print(int semicolon = 1) = 0;
};

class ASTEmptyNode : public IASTNode
{
public:
	ASTEmptyNode() : IASTNode(EMPTY) {}
	~ASTEmptyNode() {}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList&)
	{
		//calc_unreachable("trying to convert empty node to ssa");
		return NULL;
	}
	void print(int) {};
};

class ASTUnaryOpNode : public IASTNode
{
	IASTNode* m_child;

public:
	ASTUnaryOpNode(int operation) : IASTNode(operation), m_child(NULL) {}
	~ASTUnaryOpNode() { if (m_child != NULL) delete(m_child); }
	void set(IASTNode* node) { m_child = node; }
	IASTNode* get() const { return m_child; }
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		ISSANode* right = m_child->make_ssa(ssa);
		std::string left_name = ssa.new_name();
		ISSANode* left = ssa.make_var(left_name);
		int op;
		if (get_op() == UNARY_MINUS) op = ISSANode::UNARY_MINUS;
		if (get_op() == NOT) op = ISSANode::NOT;
		ssa.make_unary(op, left, right);
		return ssa.make_var(left_name);
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op == UNARY_MINUS) std::cout << "(-";
		else if (op == NOT) std::cout << "(!";
		else {
			calc_unreachable("Unknown operation");
		}
		get()->print(0);
		std::cout << ")";
		if (semicolon) std::cout << ";\n";
	}
};

class ASTBinaryOpNode : public ASTUnaryOpNode
{
	IASTNode* m_child2;

public:
	ASTBinaryOpNode(int operation) : ASTUnaryOpNode(operation), m_child2(NULL) {}
	~ASTBinaryOpNode() { if (m_child2 != NULL) delete(m_child2); }
	void set(IASTNode* node1, IASTNode* node2)
	{
		ASTUnaryOpNode::set(node1);
		m_child2 = node2;
	}
	IASTNode* get(int num) const
	{
		assert(num == 0 || num == 1);
		if (num == 0) return ASTUnaryOpNode::get();
		else return m_child2;
	}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		int op;
		std::string left_name = ssa.new_name();
		ISSANode* left = ssa.make_var(left_name);
		ISSANode* right1 = get(0)->make_ssa(ssa);
		ISSANode* right2 = get(1)->make_ssa(ssa);
		if (get_op() == EQUALITY) op = ISSANode::EQUALITY;
		else if (get_op() == NEQUALITY) op = ISSANode::NEQUALITY;
		else if (get_op() == GREATER) op = ISSANode::GREATER;
		else if (get_op() == GREATER_EQUAL) op = ISSANode::GREATER_EQUAL;
		else if (get_op() == LESS) op = ISSANode::LESS;
		else if (get_op() == LESS_EQUAL) op = ISSANode::LESS_EQUAL;
		else if (get_op() == ADD) op = ISSANode::ADD;
		else if (get_op() == SUB) op = ISSANode::SUB;
		else if (get_op() == MUL) op = ISSANode::MUL;
		else if (get_op() == DIV) op = ISSANode::DIV;
		else {
			calc_unreachable("Unknown operation");
		}
		ssa.make_binary(op, left, right1, right2);
		return ssa.make_var(left_name);
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		std::cout << "(";
		get(0)->print(0);
		if (op == EQUALITY) std::cout << "==";
		else if (op == NEQUALITY) std::cout << "!=";
		else if (op == GREATER) std::cout << ">";
		else if (op == GREATER_EQUAL) std::cout << ">=";
		else if (op == LESS) std::cout << "<";
		else if (op == LESS_EQUAL) std::cout << "<=";
		else if (op == ADD) std::cout << "+";
		else if (op == SUB) std::cout << "-";
		else if (op == MUL) std::cout << "*";
		else if (op == DIV) std::cout << "/";
		else {
			calc_unreachable("Unknown operation");
		}
		get(1)->print(0);
		std::cout << ")";
		if (semicolon) std::cout << ";" << std::endl;
	}
};

class ASTIndexNode : public ASTBinaryOpNode
{
public:
	ASTIndexNode() : ASTBinaryOpNode(INDEX) {}
	~ASTIndexNode() {}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		int op;
		std::string left_name = ssa.new_name();
		ISSANode* left = ssa.make_var(left_name);
		ISSANode* right1 = get(0)->make_ssa(ssa);
		ISSANode* right2 = get(1)->make_ssa(ssa);
		if (get_op() == INDEX) op = ISSANode::INDEX;
		else {
			calc_unreachable("Unknown operation");
		}
		ssa.make_binary(op, left, right1, right2);
		return ssa.make_var(left_name);
	}
	void print()
	{
		int op = get_op();
		std::cout << "(";
		get(0)->print(0);
		if (op == INDEX) {
			std::cout << "[";
			get(1)->print(0);
			std::cout << "];" << std::endl;
		}
		else {
			calc_unreachable("Unknown operation");
		}
	}
};

class ASTAssignNode : public ASTBinaryOpNode
{
public:
	ASTAssignNode() : ASTBinaryOpNode(ASSIGN) {}
	~ASTAssignNode() {}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa);
	virtual void print(int semicolon)
	{
		calc_unreachable("Not implemented");
	}
};

class ASTTernaryOpNode : public ASTBinaryOpNode
{
	IASTNode* m_child3;

public:
	ASTTernaryOpNode(int operation) : ASTBinaryOpNode(operation), m_child3(NULL) {}
	~ASTTernaryOpNode() { if (m_child3 != NULL) delete(m_child3); }
	void set(IASTNode* node1, IASTNode* node2, IASTNode* node3)
	{
		ASTBinaryOpNode::set(node1, node2);
		m_child3 = node3;
	}
	IASTNode* get(int num) const
	{
		assert(num == 0 || num == 1 || num == 2);
		if (num == 2) return m_child3;
		else return ASTBinaryOpNode::get(num);
	}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		ISSANode* condition = get(0)->make_ssa(ssa);
		SSAList* ssa1 = new SSAList;
		SSAList* ssa2 = new SSAList;
		ISSANode* tern_true = get(1)->make_ssa(*ssa1);
		ISSANode* tern_false = get(2)->make_ssa(*ssa2);
		std::string left1_name = ssa.new_name();
		std::string left2_name = ssa.new_name();
		ISSANode* left1 = ssa.make_var(left1_name);
		ISSANode* left2 = ssa.make_var(left2_name);
		ssa1->make_assign(left1, tern_true);
		ssa2->make_assign(left2, tern_false);
		ssa.make_ternary(ISSANode::TERNARY, condition, ssa1, ssa2);
		SSAPhiNode* phi = new SSAPhiNode;
		phi->set(ssa.make_var(left1_name));
		phi->set(ssa.make_var(left2_name));
		std::string left_name = ssa.new_name();
		ISSANode* left = new SSALeafVar(left_name);
		ssa.make_assign(left, phi);
		return ssa.make_var(left_name);
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op != TERNARY) {
			calc_unreachable("Unknown operation");
		}
		std::cout << "(";
		get(0)->print(0);
		std::cout << ")?(";
		get(1)->print(0);
		std::cout << "):(";
		get(2)->print(0);
		std::cout << ")";
		if (semicolon) std::cout << ";\n";
	}
};

class ASTLeafVar : public IASTNode
{
	unsigned int m_id;

public:
	ASTLeafVar(unsigned int id) : IASTNode(VARIABLE), m_id(id) {}
	~ASTLeafVar() {}
	unsigned int get() const { return m_id; }
	void set(unsigned int id) { m_id = id; }
	void run(InterpreterState& int_st, ExecutionState& exec_st)
	{
		if (exec_st.cmd_state == 0) // call to child 1
		{
			if (exec_st.variables->count(m_id) == 0) {
				if (int_st.sym_table->find(m_id) == int_st.sym_table->end()) calc_unreachable("Variable id not found");
				calc_unreachable("Variable '" + int_st.sym_table->find(m_id)->second.first + "' not initialized");
			}
			double var = (*exec_st.variables)[m_id];
			int_st.data_stack.push(var);
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		}

		calc_unreachable("Wrong cmd_state");
	}
	ISSANode* make_ssa(SSAList& ssa)
	{
		//TODO
		calc_unreachable("Not implemented");
		return NULL;
		/*
		char* buf = new char[strlen(m_name.c_str()) + 3];
		sprintf(buf, "u_%s", m_name.c_str());
		ISSANode* var = ssa.make_var(buf);
		delete[] buf;
		return var;
		*/
	}
	virtual void print(int semicolon)
	{
		calc_unreachable("Not implemented");
		/*int op = get_op();
		if (op != VARIABLE) {
			calc_unreachable("Unknown operation");
		}
		std::cout << m_name;
		if (semicolon) std::cout << ";\n";*/
	}
};

class ASTIncrOpNode : public IASTNode
{
	IASTNode* m_child;

public:
	ASTIncrOpNode(int operation) : IASTNode(operation), m_child(NULL) {}
	~ASTIncrOpNode() { if (m_child != NULL) delete(m_child); }
	void set(IASTNode* node) { m_child = node; }
	IASTNode* get() const { return m_child; }
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		std::string left_name = ssa.new_name();
		ISSANode* a_left = ssa.make_var(left_name);
		ISSANode* a_right = get()->make_ssa(ssa);
		ISSANode* b_left = get()->make_ssa(ssa);
		ISSANode* b_right1 = get()->make_ssa(ssa);
		ISSANode* b_right2 = ssa.make_num(1.0);
		int op = get_op();
		if (op == PRE_INC) {
			ssa.make_binary(ISSANode::ADD, b_left, b_right1, b_right2);
			ssa.make_assign(a_left, a_right);
			return ssa.make_var(left_name);
		} else if (op == PRE_DEC) {
			ssa.make_binary(ISSANode::SUB, b_left, b_right1, b_right2);
			ssa.make_assign(a_left, a_right);
			return ssa.make_var(left_name);
		} else if (op == POST_INC) {
			ssa.make_assign(a_left, a_right);
			ssa.make_binary(ISSANode::ADD, b_left, b_right1, b_right2);
			return ssa.make_var(left_name);
		} else if (op == POST_DEC) {
			ssa.make_assign(a_left, a_right);
			ssa.make_binary(ISSANode::SUB, b_left, b_right1, b_right2);
			return ssa.make_var(left_name);
		} else {
			calc_unreachable("Unknown operation\n");
			return NULL;
		}
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		std::cout << "(";
		if (op == PRE_INC) std::cout << "++";
		if (op == PRE_DEC) std::cout << "--";
		get()->print(0);
		if (op == POST_INC) std::cout << "++";
		if (op == POST_DEC) std::cout << "--";
		std::cout << ")";
		if (semicolon) std::cout << ";\n";
	}
};

class ASTLeafNum : public IASTNode
{
	double m_value;

public:
	ASTLeafNum(double value) : IASTNode(NUMBER), m_value(value) {}
	~ASTLeafNum() {}
	double get() const { return m_value; }
	void set(double value) { m_value = value; }
	void run(InterpreterState& int_st, ExecutionState& exec_st)
	{
		if (exec_st.cmd_state == 0)
		{
			int_st.data_stack.push(m_value);
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		}

		calc_unreachable("Wrong cmd_state");
	}
	ISSANode* make_ssa(SSAList& ssa)
	{
		return ssa.make_num(m_value);
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op != NUMBER) {
			calc_unreachable("Unknown operation");
			exit(-1);
		}
		std::cout << m_value;
		if (semicolon) std::cout << ";\n";
	}
};

class ASTNoRetBinaryOpNode : public ASTBinaryOpNode
{
public:
	ASTNoRetBinaryOpNode(int operation) : ASTBinaryOpNode(operation) { }
	~ASTNoRetBinaryOpNode() { }
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		int op = get_op();
		if (op == WHILE_CYCLE) {
			calc_unreachable("make_ssa is not working for while cycle");
		} else if (op == STATEMENTS) {
			delete get(0)->make_ssa(ssa);
			delete get(1)->make_ssa(ssa);
		} else {
			calc_unreachable("Unknown operation");
		}
		return NULL;
	}
	void print(int semicolon)
	{
		int op = get_op();
		if (op == WHILE_CYCLE) {
			std::cout << "while(";
			get(0)->print(0);
			std::cout << ")\n{\n";
			get(1)->print(1);
			std::cout << "}\n";
		} else if (op == STATEMENTS) {
			get(0)->print(1);
			get(1)->print(1);
		} else {
			calc_unreachable("Unknown operation");
		}
	}
};

class ASTNoRetTernaryOpNode : public ASTTernaryOpNode
{
public:
	ASTNoRetTernaryOpNode(int operation) : ASTTernaryOpNode(operation) {}
	~ASTNoRetTernaryOpNode() {}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		int op = get_op();
		if (op == IF) {
			ISSANode* condition = get(0)->make_ssa(ssa);
			SSAList* ssa1 = new SSAList;
			SSAList* ssa2 = new SSAList;
			delete get(1)->make_ssa(*ssa1);
			delete get(2)->make_ssa(*ssa2);
			ssa.make_ternary(ISSANode::IF, condition, ssa1, ssa2);
		} else {
			calc_unreachable("Unknown operation");
		}
		return NULL;
	}
	void print(int semicolon)
	{
		// TODO
		calc_unreachable("Not implemented");
	}
};

class ASTFuncCallNode : public IASTNode
{
	std::string m_name;
	std::vector<IASTNode*> m_child_args;
public:
	ASTFuncCallNode(std::string name) : IASTNode(FUNC_CALL), m_name(name)
	{}
	IASTNode* get_args(int num)
	{
		return m_child_args[num];
	}
	void set_args(IASTNode* arg) {
		m_child_args.push_back(arg);
	}
	~ASTFuncCallNode()
	{
		std::vector<IASTNode*>::iterator it;
		for (it = m_child_args.begin(); it != m_child_args.end(); ++it)
			if (*it != NULL) delete *it;
	}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		calc_unreachable("make_ssa is not working for func calls");
		return NULL;
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op != FUNC_CALL) {
			calc_unreachable("Unknown operation");
		}
		std::cout << m_name << "(";
		if (!m_child_args.empty())
			m_child_args[0]->print(0);
		for (unsigned int i = 1; i < m_child_args.size(); i++) {
			std::cout << ", ";
			m_child_args[i]->print(0);
		}
		std::cout << ")";
		if (semicolon) std::cout << ";\n";
	}
};



#endif
