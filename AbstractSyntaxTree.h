#ifndef ABSTRACT_SYNTAX_TREE_H

#define ABSTRACT_SYNTAX_TREE_H

#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>
#include <map>

#include "HashTable.h"
#include "BinarySearchTree.h"
#include "Interpreter.h"
#include "SSA.h"

static const int max_str_size = 256;

enum operation {
		UNKNOWN,
		FUNC_CALL,
		STATEMENTS, WHILE_CYCLE,
		ASSIGN, TERNARY, EQUALITY, NEQUALITY,
		GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
		ADD, SUB, MUL, DIV, UNARY_MINUS, NOT,
		POST_INC, PRE_INC, POST_DEC, PRE_DEC,
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
		if (op == UNARY_MINUS) printf("(-");
		else if (op == NOT) printf("(!");
		else {
			printf("ASTUnaryOpNode::print : Unknown operation\n");
			exit(-1);
		}
		get()->print(0);
		printf(")");
		if (semicolon) printf(";\n");
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
		if (get_op() == ASSIGN) {
			op = ISSANode::ASSIGN;
			ISSANode* left = get(0)->make_ssa(ssa);
			ISSANode* right = get(1)->make_ssa(ssa);
			ssa.make_assign(left, right);
			return get(0)->make_ssa(ssa);
		}
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
			printf("ASTBinaryOpNode::make_ssa : Unknown operation\n");
			exit(-1);
		}
		ssa.make_binary(op, left, right1, right2);
		return ssa.make_var(left_name);
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		printf("(");
		get(0)->print(0);
		if (op == ASSIGN) printf("=");
		else if (op == EQUALITY) printf("==");
		else if (op == NEQUALITY) printf("!=");
		else if (op == GREATER) printf(">");
		else if (op == GREATER_EQUAL) printf(">=");
		else if (op == LESS) printf("<");
		else if (op == LESS_EQUAL) printf("<=");
		else if (op == ADD) printf("+");
		else if (op == SUB) printf("-");
		else if (op == MUL) printf("*");
		else if (op == DIV) printf("/");
		else {
			printf("ASTBinaryOpNode::print : Unknown operation\n");
			exit(-1);
		}
		get(1)->print(0);
		printf(")");
		if (semicolon) printf(";\n");
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
		ssa.make_ternary(condition, ssa1, ssa2);
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
			printf("ASTTernaryOpNode::print : Unknown operation\n");
			exit(-1);
		}
		printf("(");
		get(0)->print(0);
		printf(")?(");
		get(1)->print(0);
		printf("):(");
		get(2)->print(0);
		printf(")");
		if (semicolon) printf(";\n");
	}
};

class ASTLeafVar : public IASTNode
{
	std::string m_name;

public:
	ASTLeafVar(const char* name) : IASTNode(VARIABLE), m_name(name) {}
	~ASTLeafVar() {}
	void get(char* name, int n) const
	{
		strncpy(name, m_name.c_str(), n-1);
		name[n-1] = '\0';
	}
	void set(char* name) {
		m_name = name;
	}
	void run(InterpreterState& int_st, ExecutionState& exec_st)
	{
		if (exec_st.cmd_state == 0) // call to child 1
		{
			double var = exec_st.variables->get(m_name.c_str());
			int_st.data_stack.push(var);
			exec_st.cmd_state = int_st.op_stack.top();
			int_st.op_stack.pop();
			exec_st.command = int_st.command_stack.top();
			int_st.command_stack.pop();
			return;
		}

		printf("AbstractSyntaxTree : ASTLeafVar : Wrong cmd_state\n");
		exit(-1);
	}
	ISSANode* make_ssa(SSAList& ssa)
	{
		char* buf = new char[strlen(m_name.c_str()) + 3];
		sprintf(buf, "u_%s", m_name.c_str());
		ISSANode* var = ssa.make_var(buf);
		delete[] buf;
		return var;
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op != VARIABLE) {
			printf("ASTLeafVar::print : Unknown operation\n");
			exit(-1);
		}
		printf("%s", m_name.c_str());
		if (semicolon) printf(";\n");
	}
};

class ASTIncrOpNode : public IASTNode
{
	ASTLeafVar* m_child;

public:
	ASTIncrOpNode(int operation) : IASTNode(operation), m_child(NULL) {}
	~ASTIncrOpNode() { if (m_child != NULL) delete(m_child); }
	void set(ASTLeafVar* node) { m_child = node; }
	ASTLeafVar* get() const { return m_child; }
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
			printf("ASTIncrOpNode::make_ssa : Unknown operation\n");
			exit(-1);
			return NULL;
		}
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		printf("(");
		if (op == PRE_INC) printf("++");
		if (op == PRE_DEC) printf("--");
		get()->print(0);
		if (op == POST_INC) printf("++");
		if (op == POST_DEC) printf("--");
		printf(")");
		if (semicolon) printf(";\n");
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

		printf("AbstractSyntaxTree : ASTLeafNum : Wrong cmd_state\n");
		exit(-1);
	}
	ISSANode* make_ssa(SSAList& ssa)
	{
		return ssa.make_num(m_value);
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op != NUMBER) {
			printf("ASTLeafNum::print : Unknown operation\n");
			exit(-1);
		}
		printf("%lg", m_value);
		if (semicolon) printf(";\n");
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
			printf("make_ssa is not working for while cycle\n");
			exit(-1);
		} else if (op == STATEMENTS) {
			delete get(0)->make_ssa(ssa);
			delete get(1)->make_ssa(ssa);
		} else {
			printf("ASTNoRetBinaryOpNode::make_ssa : Unknown operation\n");
			exit(-1);
		}
		return NULL;
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op == WHILE_CYCLE) {
			printf("while(");
			get(0)->print(0);
			printf(")\n{\n");
			get(1)->print(1);
			printf("}\n");
		} else if (op == STATEMENTS) {
			get(0)->print(1);
			get(1)->print(1);
		} else {
			printf("ASTNoRetBinaryOpNode::print : Unknown operation\n");
			exit(-1);
		}
	}
};

class ASTFuncCallNode : public IASTNode
{
	std::string m_name;
	int m_args_num;
	std::vector<IASTNode*> m_child_args;
public:
	ASTFuncCallNode(std::string name) : IASTNode(FUNC_CALL), m_name(name), m_args_num(0)
	{}
	IASTNode* get_args(int num)
	{
		return m_child_args[num];
	}
	void set_args(IASTNode* arg) {
		m_child_args.push_back(arg);
		++m_args_num;
	}
	~ASTFuncCallNode()
	{
		for (int i = 0; i < m_args_num; i++)
			if (m_child_args[i] != NULL) delete m_child_args[i];
	}
	void run(InterpreterState&, ExecutionState&);
	ISSANode* make_ssa(SSAList& ssa)
	{
		printf("make_ssa is not working for func calls\n");
		exit(-1);
		return NULL;
	}
	virtual void print(int semicolon)
	{
		int op = get_op();
		if (op != FUNC_CALL) {
			printf("ASTFuncCallNode::print : Unknown operation\n");
			exit(-1);
		}
		std::cout << m_name << "(";
		if (m_args_num != 0)
			m_child_args[0]->print(0);
		for (int i = 1; i < m_args_num; i++) {
			std::cout << ", ";
			m_child_args[i]->print(0);
		}
		std::cout << ")";
		if (semicolon) std::cout << ";\n";
	}
};

#endif
