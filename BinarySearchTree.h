#ifndef BINARY_SEARCH_TREE_H

#define BINARY_SEARCH_TREE_H

#include <cstdio>
#include <cstring>
#include <cstdlib>

class BinarySearchTree
{
public:
	static const unsigned int max_record_size = 256; // max length of variable name
private:
	struct Node {
		char name[max_record_size];
		double value;
		Node* left;
		Node* right;
		Node() : value(0.0), left(NULL), right(NULL) {}
		~Node() {
			if (left != NULL) delete left;
			if (right != NULL) delete right;
		}
	} * root;
	BinarySearchTree(const BinarySearchTree &);
	const BinarySearchTree& operator = (const BinarySearchTree &);
public:  
	BinarySearchTree() : root (NULL) {}
	~BinarySearchTree() { if (root != NULL) delete root; }
	// return 0 if variable exist, 1 if variable not exist
	int put(const char* name, const double val)
	{
		Node** current = &root;
		while (*current != NULL) {
			int eq = strcmp(name, (*current)->name);
			if (eq > 0) current = &((*current)->right);
			else if (eq < 0) current = &((*current)->left);
			else {
				(*current)->value = val;
				return 0;
			}
		}
		*current = new Node;
		strncpy((*current)->name, name, max_record_size - 1);
		(*current)->name[max_record_size - 1] = '\0';
		(*current)->value = val;
		return 1;
	}
	double get(const char* name)
	{
		Node* current = root;
		while (current != NULL) {
			int eq = strcmp(name, current->name);
			if (eq > 0) current = current->right;
			else if (eq < 0) current = current->left;
			else return current->value;
		}
		printf("BinarySearchTree : Record '%s' not found\n", name);
		exit(-1);
	}
};

#endif // BINARY_SEARCH_TREE_H
