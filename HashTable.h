#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <cassert>
#include <cstdio>
#include <cstring>

#include "ParserFunc.h"

class HashTable
{
	static const int max_buf_length;
	static const unsigned int hash_coeff = 756629;
	struct Node
	{
		ParserFunc* func;
		Node* next;
		Node() : func(NULL), next(NULL)
		{}
		~Node()
		{
			if (func != NULL) delete func;
			if (next != NULL) delete next;
		}
	};
	int m_size;
	Node** m_hash_table;
	unsigned int hash_func(std::string str) const
	{
		unsigned int res = 0;
		for (unsigned int i = 0; i < str.size(); i++)
			res += str[i] * hash_coeff;
		return res % m_size;
	}

	HashTable& operator = (const HashTable& rhs);
	HashTable(const HashTable& rhs);
public:
	HashTable(int size = 31) : m_size(size)
	{
		m_hash_table = new Node*[size];
		assert(m_hash_table);
		for (int i = 0; i < size; i++)
			m_hash_table[i] = NULL;
	}
	~HashTable()
	{
		for (int i = 0; i < m_size; i++)
			if (m_hash_table[i] != NULL) delete m_hash_table[i];
		delete[] m_hash_table;
	}
	// return 0 if exist, 1 if not exist
	int put(ParserFunc* pf);
	ParserFunc* get(std::string name) const;
};

#endif // HASHTABLE_H
