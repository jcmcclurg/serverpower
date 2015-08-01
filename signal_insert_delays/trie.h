#ifndef trie_h
#define trie_h

#define TRIE_BIT_MASK 0x7FFF
typedef struct trie_root_struct {
	struct trie_struct* next_one;
	struct trie_struct* next_zero;
} trie_root;

typedef struct trie_struct {
	void* value;
	struct trie_struct* next_one;
	struct trie_struct* next_zero;
} trie_node_t;

// Tries to insert the value at key.
// If key already exists, returns null.
// Otherwise, returns the new node in the trie.
// You can't insert a null key.
trie_node_t* trie_insert(int key, void* value, trie_root* root);

// Tries to remove the key.
// If the key exists, return the value.
// Otherwise, return null.
void* trie_remove(int key, trie_root* root);

// Tries to find the value.
// If the key exists, return the value.
// Otherwise, return null.
void* trie_value(int key, trie_root* root);

// Allocates a new trie.
trie_root trie_new(void);

#endif
