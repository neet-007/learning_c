#ifndef TRIE_PACKAGE
#define TRIE_PACKAGE

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHILDREN 128
#define EMPTY_CHAR '\0'

typedef struct Trie Trie;

typedef struct Trie{
    int value;
    int word_count;
    Trie *parent;
    Trie *children[MAX_CHILDREN];
}Trie;

typedef enum Trie_package_print_type{
    PRINT_LEVELS,
    PRINT_STRING,
} Trie_package_print_type;

Trie *trie_build(char *value, size_t value_size);
Trie *trie_new(int value, int is_terminal, Trie *parent);
Trie *trie_search(Trie *trie, char *value, size_t value_size);
int trie_add(Trie *trie, char *value, size_t value_size);
int trie_delete(Trie *trie, char *value, size_t value_size);
void trie_print(Trie *trie, Trie_package_print_type print_type);
void trie_free(Trie *trie);
char *get_trie_package_error_message();

#endif
