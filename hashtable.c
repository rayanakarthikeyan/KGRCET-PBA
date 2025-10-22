#include <stdio.h>
#include <stdlib.h>

#define TABLE_SIZE 1009

typedef enum { LINEAR_PROBING=1, SEPARATE_CHAINING=2 } CollisionMethod;

// For Linear Probing
typedef struct {
    int *table;
    int size;
    int count;
    int collisions;
    int probes;
} LinearProbingHashTable;

// For Separate Chaining
typedef struct Node {
    int key;
    struct Node* next;
} Node;

typedef struct {
    Node** chains;
    int size;
    int count;
    int collisions;
} SeparateChainingHashTable;

// Utility functions

int hash1(int key) {
    return key % TABLE_SIZE;
}

LinearProbingHashTable* create_linear_table() {
    LinearProbingHashTable* ht = malloc(sizeof(LinearProbingHashTable));
    ht->size = TABLE_SIZE;
    ht->count = 0;
    ht->collisions = 0;
    ht->probes = 0;
    ht->table = malloc(sizeof(int)*ht->size);
    for(int i=0; i < ht->size; i++) ht->table[i] = -1;
    return ht;
}

int linear_insert(LinearProbingHashTable* ht, int key) {
    int idx = hash1(key);
    int probes = 1;

    while(ht->table[idx] != -1) {
        if(ht->table[idx] == key) {
            ht->probes += probes;
            return 0; // already exists
        }
        idx = (idx + 1) % ht->size;
        probes++;
        ht->collisions++;
        if(probes > ht->size) return -1; // full table
    }
    ht->table[idx] = key;
    ht->count++;
    ht->probes += probes;
    return 1; // inserted
}

// Separate chaining functions
SeparateChainingHashTable* create_chaining_table() {
    SeparateChainingHashTable* ht = malloc(sizeof(SeparateChainingHashTable));
    ht->size = TABLE_SIZE;
    ht->count = 0;
    ht->collisions = 0;
    ht->chains = malloc(sizeof(Node*) * ht->size);
    for(int i=0; i < ht->size; i++) ht->chains[i] = NULL;
    return ht;
}

int chaining_insert(SeparateChainingHashTable* ht, int key) {
    int idx = hash1(key);
    Node* head = ht->chains[idx];

    Node* current = head;
    while(current) {
        if(current->key == key) return 0; // already exists
        current = current->next;
    }
    if(head != NULL) ht->collisions++;

    Node* new_node = malloc(sizeof(Node));
    new_node->key = key;
    new_node->next = head;
    ht->chains[idx] = new_node;
    ht->count++;
    return 1; // inserted
}

// Cleanup functions

void free_linear_table(LinearProbingHashTable* ht) {
    free(ht->table);
    free(ht);
}

void free_chaining_table(SeparateChainingHashTable* ht) {
    for(int i=0; i < ht->size; i++) {
        Node* current = ht->chains[i];
        while(current) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->chains);
    free(ht);
}

// Main API for insertion: returns collisions and probes for the batch of keys

typedef struct {
    int total_inserts;
    int total_collisions;
    int total_probes; // only for linear probing
} Stats;

Stats run_linear_probing(int* keys, int n) {
    Stats stats = {0,0,0};
    LinearProbingHashTable* ht = create_linear_table();

    for(int i=0; i < n; i++) {
        int res = linear_insert(ht, keys[i]);
        if(res == 1) stats.total_inserts++;
    }
    stats.total_collisions = ht->collisions;
    stats.total_probes = ht->probes;

    free_linear_table(ht);
    return stats;
}

Stats run_separate_chaining(int* keys, int n) {
    Stats stats = {0,0,0};
    SeparateChainingHashTable* ht = create_chaining_table();

    for(int i=0; i < n; i++) {
        int res = chaining_insert(ht, keys[i]);
        if(res == 1) stats.total_inserts++;
    }
    stats.total_collisions = ht->collisions;
    stats.total_probes = 0; // not meaningful here

    free_chaining_table(ht);
    return stats;
}

// Expose a function for Python to call
// We use collision_method = 1 for linear, 2 for chaining
// keys is int array, n is number of keys
// result is Stats struct returned by value (use pointer in Python)

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT Stats run_hash_test(int* keys, int n, int collision_method) {
    if(collision_method == LINEAR_PROBING) {
        return run_linear_probing(keys, n);
    } else if(collision_method == SEPARATE_CHAINING) {
        return run_separate_chaining(keys, n);
    }
    Stats empty = {0,0,0};
    return empty;
}
