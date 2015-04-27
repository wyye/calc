#include "SSA.h"

void SSAList::add(ISSANode* n)
{
	if (m_start == NULL) m_start = m_end = n;
	else {
		m_end->set_next(n);
		m_end = n;
	}
}

SSAList::SSAList() : m_start(NULL), m_end(NULL) {}

SSAList::~SSAList()
{
	if (m_start != NULL) {
		ISSANode* cur = m_start;
		do {
			cur = cur->get_next();
			delete m_start;
			m_start = cur;
		} while (cur != NULL);
	}
}

ISSANode* SSAList::get_first() const { return m_start; }

ISSANode* SSAList::get_end() const { return m_end; }

std::string SSAList::new_name()
{
	char buf[20];
	sprintf(buf, "t_%d", last_name++);
	std::string name = buf;
	return name;
}

ISSANode* SSAList::make_num(double val) { return new SSALeafNum(val); }

ISSANode* SSAList::make_var(std::string name) { return new SSALeafVar(name); }

void SSAList::set_first(ISSANode* first) { m_start = first; }

void SSAList::set_end(ISSANode* last) { m_end = last; }

void SSAList::make_unary(int op, ISSANode* left, ISSANode* right)
{
	SSABinaryOpNode* assign = new SSABinaryOpNode(ISSANode::ASSIGN);
	SSAUnaryOpNode* unary = new SSAUnaryOpNode(op);
	unary->set(right);
	assign->set(left, unary);
	add(assign);
}

void SSAList::make_binary(int op, ISSANode* left, ISSANode* right1, ISSANode* right2)
{
	SSABinaryOpNode* assign = new SSABinaryOpNode(ISSANode::ASSIGN);
	SSABinaryOpNode* binary = new SSABinaryOpNode(op);
	binary->set(right1, right2);
	assign->set(left, binary);
	add(assign);
}

void SSAList::make_assign(ISSANode* left, ISSANode* right)
{
	SSABinaryOpNode* assign = new SSABinaryOpNode(ISSANode::ASSIGN);
	assign->set(left, right);
	add(assign);
}

void SSAList::make_ternary(ISSANode* condition, SSAList* ternary_true, SSAList* ternary_false)
{
	SSATernaryOpNode* ternary = new SSATernaryOpNode(ISSANode::TERNARY);
	ternary->set(condition, ternary_true, ternary_false);
	add(ternary);
}

void SSAList::print()
{
	if (m_start == NULL) return;
	ISSANode* cur = m_start;
	do {
		cur->print();
		cur = cur->get_next();
	} while (cur != NULL);
}

void SSAList::make_ssa(std::map<std::string, int>& vars1, std::map<std::string, int>& vars2)
{
	if (m_start == NULL) return;
	ISSANode* cur = m_start;
	do {
		cur->make_ssa(vars1, vars2);
		cur = cur->get_next();
	} while (cur != NULL);
}

void SSAList::add_front(ISSANode* node)
{
	node->set_next(m_start);
	m_start = node;
}

void SSATernaryOpNode::make_ssa(std::map<std::string, int>& left_vars, std::map<std::string, int>& right_vars, int change)
{
	m_child->make_ssa(left_vars, right_vars);
	std::map<std::string, int> right_vars1, right_vars2;
	right_vars1 = right_vars2 = right_vars;
	std::map<std::string, int>::iterator it, it1, it2;
	m_true->make_ssa(left_vars, right_vars1);
	m_false->make_ssa(left_vars, right_vars2);

	// create phi nodes
	char* temp_num = new char[20];

	for (it = right_vars.begin(); it != right_vars.end(); ++it) {
		it1 = right_vars1.find(it->first);
		it2 = right_vars2.find(it->first);
		if (it1->second != it2->second) {
			SSAPhiNode* phi = new SSAPhiNode;
			std::string temp_str = it1->first;
			sprintf(temp_num, "_%d", it1->second);
			SSALeafVar* var1 = new SSALeafVar(temp_str+temp_num);
			sprintf(temp_num, "_%d", it2->second);
			SSALeafVar* var2 = new SSALeafVar(temp_str+temp_num);
			SSALeafVar* var = new SSALeafVar(temp_str);
			SSABinaryOpNode* assign = new SSABinaryOpNode(ISSANode::ASSIGN);
			phi->set(var1);
			phi->set(var2);
			assign->set(var, phi);
			assign->set_next(get_next());
			set_next(assign);
		}
	}

	delete[] temp_num;
}

int SSAList::last_name = 0;
