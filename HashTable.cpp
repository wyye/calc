#include <cstring>

#include "HashTable.h"
#include "ParserFunc.h"

int HashTable::put(ParserFunc* pf)
{
	unsigned int hash = hash_func(pf->name.c_str());
	if (m_hash_table[hash] == NULL) {
		m_hash_table[hash] = new Node;
		m_hash_table[hash]->func = pf;
		m_hash_table[hash]->next = NULL;
		return 1;
	} else {
		Node* curr = m_hash_table[hash];
		while (1) {
			if (pf->name == curr->func->name) {
				m_hash_table[hash]->func = pf;
				return 0;
			} else {
				if (curr->next == NULL) {
					curr->next = new Node;
					m_hash_table[hash]->func = pf;
					m_hash_table[hash]->next = NULL;
					return 1;
				}
				curr = curr->next;
			}
		}
	}
}

ParserFunc* HashTable::get(std::string name) const
{
	unsigned int hash = hash_func(name);
	if (m_hash_table[hash] == NULL) {
		return NULL;
	} else {
		Node* curr = m_hash_table[hash];
		while (curr != NULL) {
			if (name == curr->func->name) {
				return curr->func;
			} else {
				curr = curr->next;
			}
		}
		return NULL;
	}
}
