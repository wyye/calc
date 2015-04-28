#ifndef SSA_H
#define SSA_H

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "HelpTools.h"

class ISSANode;

class SSAList
{
	ISSANode* m_start;
	ISSANode* m_end;

	static int last_name;

	void add(ISSANode* n);

	SSAList(const SSAList&);
	void operator=(const SSAList&);
public:
	SSAList();
	~SSAList();
	ISSANode* get_first() const;
	ISSANode* get_end() const;
	std::string new_name();
	ISSANode* make_num(double val);
	ISSANode* make_var(std::string name);
	void set_first(ISSANode* first);
	void set_end(ISSANode* last);
	void make_unary(int op, ISSANode* left, ISSANode* right);
	void make_binary(int op, ISSANode* left, ISSANode* right1, ISSANode* right2);
	void make_assign(ISSANode* left, ISSANode* right);
	void make_ternary(int, ISSANode* condition, SSAList* ternary_true, SSAList* ternary_false);
	void make_if(ISSANode* condition, SSAList* ternary_true, SSAList* ternary_false);
	void print();
	void make_ssa(std::map<std::string, int>&, std::map<std::string, int>&);
	void add_front(ISSANode* front);
};

class ISSANode
{
public:
	enum operation {
		UNKNOWN,
		IF,
		ASSIGN, TERNARY, EQUALITY, NEQUALITY,
		GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
		ADD, SUB, MUL, DIV, UNARY_MINUS, NOT,
		VARIABLE, NUMBER,
		PHI
	};
private:
	int m_op;
	ISSANode* m_next;
	ISSANode& operator = (const ISSANode& rhs);
	ISSANode(const ISSANode& rhs);
public:
	ISSANode(int operation) : m_op(operation), m_next(NULL) {}
	virtual ~ISSANode() {};
	int get_op() const { return m_op; }
	void set_op(int operation) { m_op = operation; }
	ISSANode* get_next() const { return m_next; }
	void set_next(ISSANode* next) { m_next = next; }
	virtual void print() = 0;
	virtual void make_ssa(std::map<std::string, int>&, std::map<std::string, int>&, int change = 0) = 0;
};

class SSAUnaryOpNode : public ISSANode
{
	ISSANode* m_child;

public:
	SSAUnaryOpNode(int operation) : ISSANode(operation), m_child(NULL) {}
	~SSAUnaryOpNode() { if (m_child != NULL) delete(m_child); }
	void set(ISSANode* node) { m_child = node; }
	ISSANode* get() const { return m_child; }
	void print()
	{
		int op = get_op();
		if (op == ISSANode::UNARY_MINUS) std::cout << "- ";
		else if (op == ISSANode::NOT) std::cout << "! ";
		else {
			calc_unreachable("Unknown operation");
		}
		m_child->print();
	}
	void make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int)
	{
		m_child->make_ssa(left_vars, right_vars);
	}
};

class SSABinaryOpNode : public SSAUnaryOpNode
{
	ISSANode* m_child2;

public:
	SSABinaryOpNode(int operation) : SSAUnaryOpNode(operation), m_child2(NULL) {}
	~SSABinaryOpNode() { if (m_child2 != NULL) delete(m_child2); }
	void set(ISSANode* node1, ISSANode* node2)
	{
		SSAUnaryOpNode::set(node1);
		m_child2 = node2;
	}
	ISSANode* get(int num) const
	{
		assert(num == 0 || num == 1);
		if (num == 0) return SSAUnaryOpNode::get();
		else return m_child2;
	}
	void print()
	{
		get(0)->print();
		int op = get_op();
		if (op == ISSANode::ASSIGN) {
			std::cout << " = ";
			get(1)->print();
			std::cout << ";\n";
			return;
		}
		else if (op == ISSANode::EQUALITY) std::cout << " == ";
		else if (op == ISSANode::NEQUALITY) std::cout << " != ";
		else if (op == ISSANode::GREATER) std::cout << " > ";
		else if (op == ISSANode::GREATER_EQUAL) std::cout << " >= ";
		else if (op == ISSANode::LESS) std::cout << " < ";
		else if (op == ISSANode::LESS_EQUAL) std::cout << " <= ";
		else if (op == ISSANode::ADD) std::cout << " + ";
		else if (op == ISSANode::SUB) std::cout << " - ";
		else if (op == ISSANode::MUL) std::cout << " * ";
		else if (op == ISSANode::DIV) std::cout << " / ";
		else {
			calc_unreachable("Unknown operation");
		}
		get(1)->print();
	}
	void make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int)
	{
		get(1)->make_ssa(left_vars, right_vars);
		if (get_op() == ISSANode::ASSIGN) get(0)->make_ssa(left_vars, right_vars, 1);
		else get(0)->make_ssa(left_vars, right_vars);
	}
};

class SSALeafVar : public ISSANode
{
	std::string m_name;

public:
	SSALeafVar(std::string name) : ISSANode(VARIABLE), m_name(name) {}
	~SSALeafVar() {}
	std::string get() const { return m_name; }
	void set(std::string name) { m_name = name; }
	void print()
	{
		std::cout << m_name;
	}
	void make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int change)
	{
		if (m_name[0] != 'u') return; // work only with user variables
		std::map<std::string, int>::iterator it;
		int num = 0;
		if (change == 0) {
			it = right_vars.find(m_name);
			if (it == right_vars.end()) {
				std::cout << "SSALeafVar::make_ssa : Undefined variable '" << m_name << "'\n";
				exit(-1);
			}
			num = it->second;
		} else {
			it = left_vars.find(m_name);
			if (it == left_vars.end()) {
				num = right_vars[m_name] = left_vars[m_name] = 0;
			} else {
				num = right_vars[m_name] = ++left_vars[m_name];
			}
		}
		char number[20];
		sprintf(number, "_%d", num);
		m_name.append(number);
	}
};

class SSALeafNum : public ISSANode
{
	double m_value;

public:
	SSALeafNum(double value) : ISSANode(NUMBER), m_value(value) {}
	~SSALeafNum() {}
	double get() const { return m_value; }
	void set(double value) { m_value = value; }
	void print()
	{
		std::cout << m_value;
	}
	void make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int change) {}
};

class SSATernaryOpNode : public ISSANode
{
	ISSANode* m_child;
	SSAList* m_true;
	SSAList* m_false;

public:
	SSATernaryOpNode(int operation) : ISSANode(operation), m_child(NULL), m_true(NULL), m_false(NULL) {}
	~SSATernaryOpNode() {
		if (m_child != NULL) delete m_child;
		if (m_true != NULL) delete m_true;
		if (m_false != NULL) delete m_false;
	}
	void set(ISSANode* node1, SSAList* node2, SSAList* node3)
	{
		m_child = node1;
		m_true = node2;
		m_false = node3;
	}
	ISSANode* get_cond() const { return m_child; }
	SSAList* get_true() const { return m_true; }
	SSAList* get_false() const { return m_false; }
	void print()
	{
		std::cout << "if ( ";
		get_cond()->print();
		std::cout << " ) {\n";
		get_true()->print();
		std::cout << "} else {\n";
		get_false()->print();
		std::cout << "}\n";
	}
	void make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int change);
};

class SSAPhiNode : public ISSANode
{
	std::vector<ISSANode*> m_vars;

public:
	SSAPhiNode() : ISSANode(PHI) {}
	~SSAPhiNode()
	{
		std::vector<ISSANode*>::iterator it;
		for (it = m_vars.begin(); it != m_vars.end(); ++it) {
			delete *it;
		}
	}
	void set(ISSANode* node) { m_vars.push_back(node); }
	ISSANode* get(int pos) const { return m_vars[pos]; }
	void print()
	{
		std::vector<ISSANode*>::iterator it;
		std::cout << "phi (";
		it = m_vars.begin();
		if (m_vars.empty()) return;
		(*it)->print();
		++it;
		while (it != m_vars.end()) {
			std::cout << ", ";
			(*it)->print();
			++it;
		}
		std::cout << ")";
	}
	void make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int) {}
};

#endif // SSA_H
