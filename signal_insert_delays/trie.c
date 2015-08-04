#include "trie.h"
#include <stdlib.h>

trie_root trie_new(void){
	trie_root r;
	r.num_nodes	= 0;
	r.next_one	= NULL;
	r.next_zero	= NULL;
	r.data		= NULL;
	return r;
}

trie_node_t* trie_node(void){
	trie_node_t* n =  (trie_node_t*) malloc(sizeof(trie_node));
	n->value = NULL;
	n->next_one = NULL;
	n->next_zero = NULL;
	return n;
}

void* trie_insert(int key, void* value, trie_root* root){
	if(value == NULL){
		return NULL;
	}

	int k = key & TRIE_BIT_MASK;
	trie_node_t* currentNode;
	if(k & 0x1){
		if(root->next_one != NULL){
			currentNode = root->next_one;
		}
		else{
			currentNode = trie_node();
		}
	}
	else{
		if(root->next_zero != NULL){
			currentNode = root->next_zero;
		}
		else{
			currentNode = trie_node();
		}
	}

	k = (k >> 1) & TRIE_BIT_MASK;
	while(k){
		if(k & 0x1){
			if(currentNode->next_one != NULL){
				currentNode = currentNode->next_one;
			}
			else{
				trie_node_t* cn = trie_node();
				currentNode->next_one = cn;
				currentNode = cn;
			}
		}
		else{
			if(currentNode->next_zero != NULL){
				currentNode = currentNode->next_zero;
			}
			else{
				trie_node_t* cn = trie_node();
				currentNode->next_zero = cn;
				currentNode = cn;
			}
		}
		k = (k >> 1) & TRIE_BIT_MASK;
	}
	
	if(currentNode->value == NULL){
		currentNode->value = value;
		(root->num_nodes)++;
	}
	return currentNode->value;
}

// Returns 1 if the node was actually deleted from the tree.
// Returns 0 if the node was kept because it was needed for the structure.
int trie_remove_recursive(int k, trie_node_t* currentNode, void** v){
	if(k){
		*v = NULL;
		if(k & 0x1 && currentNode->next_one != NULL){
			if(trie_remove_recursive((k >> 1) & TRIE_BIT_MASK, currentNode->next_one, v))
				currentNode->next_one = NULL;
		}
		else if(currentNode->next_zero != NULL){
			if(trie_remove_recursive((k >> 1) & TRIE_BIT_MASK, currentNode->next_zero, v))
				currentNode->next_zero = NULL;
		}

		if(*v != NULL && currentNode->value == NULL && currentNode->next_zero == NULL && currentNode->next_one == NULL){
			free(currentNode);
			return 1;
		}
		else{
			return 0;
		}
	}
	else{
		*v = currentNode->value;
		currentNode->value = NULL;
		if(currentNode->next_zero == NULL && currentNode->next_one == NULL){
			free(currentNode);
			return 1;
		}
		else{
			return 0;
		}
	}
}

void* trie_remove(int key, trie_root* root){
	int k = key & TRIE_BIT_MASK;
	trie_node_t* currentNode;
	if(k & 0x1){
		if(root->next_one != NULL){
			currentNode = root->next_one;
		}
		else{
			return NULL;
		}
	}
	else{
		if(root->next_zero != NULL){
			currentNode = root->next_zero;
		}
		else{
			return NULL;
		}
	}
	k = (k >> 1) & TRIE_BIT_MASK;

	void* v = NULL;
	trie_remove_recursive(k, currentNode, &v);
	if(v != NULL)
		(root->num_nodes)--;

	return v;
}

void* trie_value(int key, trie_root* root){
	int k = key & TRIE_BIT_MASK;
	trie_node_t* currentNode;
	if(k & 0x1){
		if(root->next_one != NULL){
			currentNode = root->next_one;
		}
		else{
			return NULL;
		}
	}
	else{
		if(root->next_zero != NULL){
			currentNode = root->next_zero;
		}
		else{
			return NULL;
		}
	}
	k = (k >> 1) & TRIE_BIT_MASK;
	while(k){
		if(k & 0x1){
			if(currentNode->next_one != NULL){
				currentNode = currentNode->next_one;
			}
			else{
				return NULL;
			}
		}
		else{
			if(currentNode->next_zero != NULL){
				currentNode = currentNode->next_zero;
			}
			else{
				return NULL;
			}
		}
		k = (k >> 1) & TRIE_BIT_MASK;
	}
	
	return currentNode->value;
}

int trie_iterate_recursive(int (*operate)(int key, void* value), trie_node_t* currentNode, int k){
	int r = 0;
	if(currentNode != NULL){
		if(currentNode->value != NULL){
			r += operate(k, currentNode->value);
		}
		r += trie_iterate_recursive(operate, currentNode->next_zero, (k<<1));
		r += trie_iterate_recursive(operate, currentNode->next_one, (k<<1)+1);
	}
	return r;
}

int trie_iterate(int (*operate)(int key, void* value), trie_root* root){
	int r = 0;
	r += trie_iterate_recursive(operate, root->next_zero, 0);
	r += trie_iterate_recursive(operate, root->next_one, 1);
	return r;
}

int trie_node_free(int key, void* value, trie_root* root){
	void* v = trie_remove(key, root);

	if(v != NULL){
		free(v);
		return 1;
	}
	else{
		return 0;
	}
}

int trie_deallocate(trie_root* root){
	return trie_iterate(&trie_node_free, root);
}
