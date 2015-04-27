#ifndef STACK_H
#define STACK_H

#include <cstdio>
#include <iostream>

template <typename T>
class Stack {
	struct StackElem {
		T m_value;
		StackElem* m_next;
		StackElem(const T& value, StackElem* next) : m_value(value), m_next(next) {}
	};
	StackElem* m_top;
	const Stack& operator = (const Stack&);
	Stack(const Stack&);
public:
	Stack() : m_top(NULL)
	{}
	~Stack()
	{
		while (pop() != 1) {}
	}
	void push(const T& value)
	{
		m_top = new StackElem(value, m_top);
	}
	// return 1 if stack empty, else return 0
	int pop()
	{
		if (m_top == NULL) return 1;
		StackElem* temp = m_top->m_next;
		delete m_top;
		m_top = temp;
		return 0;
	}
	T top() const
	{
		return m_top->m_value;
	}
	void print()
	{
		std::cout << "Stack: ";
		StackElem* cur = m_top;
		while (cur != NULL) {
			std::cout << cur->m_value << " ";
			cur = cur->m_next;
		}
		std::cout << "\n";
	}
};

#endif
