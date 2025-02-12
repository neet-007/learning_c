#include "trie.h"

int main(void){
    char *word = "hello";
    Trie *trie = trie_build(word, strlen(word));
    word = "world";
    trie_add(trie, word, strlen(word));
    if (trie == NULL){
        fprintf(stderr, "unable to build trie\n %s", get_trie_package_error_message());
        return 1;
    }

    printf("LEVELS\n");
    trie_print(trie, PRINT_LEVELS);
    printf("\n\nSTRING\n");
    trie_print(trie, PRINT_STRING);
    free_trie(trie);
    return 0;
}
