#ifndef TRIE_PACKAGE
#define TRIE_PACKAGE

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHILDREN 27
#define EMPTY_CHAR '\0'

typedef struct Trie Trie;

typedef struct Trie{
    int value;
    int is_terminal;
    Trie *children[MAX_CHILDREN];
}Trie;

Trie *trie_build(char *value, size_t value_size);
Trie *trie_new(int value, int is_terminal);
Trie *trie_search(Trie *trie, char *value, size_t value_size);
int trie_add(Trie *trie, char *value, size_t value_size);
int trie_delete(Trie *trie, char *value, size_t value_size);
void trie_print(Trie *trie);
void free_trie(Trie *trie);
char *get_trie_package_error_message();

#endif
