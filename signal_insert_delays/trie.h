#ifndef trie_h
#define trie_h

//
#define TRIE_BIT_MASK 0x7FFF
typedef struct trie_root_struct {
	int num_nodes;
	void* data;
	struct trie_struct* next_one;
	struct trie_struct* next_zero;
} trie_root;

typedef struct trie_struct {
	void* value;
	struct trie_struct* next_one;
	struct trie_struct* next_zero;
} trie_node_t;

void trie_set_verbose(char v);

// Tries to insert the value at key.
// Returns the value inserted, or the current value if the key already exists.
// You can't insert a null key.
void* trie_insert(int key, void* value, trie_root* root);

// Tries to remove the key.
// If the key exists, return the value.
// Otherwise, return null.
void* trie_remove(int key, trie_root* root);

// Tries to find the value.
// If the key exists, return the value.
// Otherwise, return null.
void* trie_value(int key, trie_root* root);

// Returns the sum of return values from the operate.
int trie_iterate(int (*operate)(int key, void* value, trie_root* trie), trie_root* root);

// Deallocates the trie structure.
int trie_deallocate(trie_root* root);

// Allocates a new trie.
trie_root* trie_new(void);

#endif
