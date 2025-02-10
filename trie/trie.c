#include "trie.h"
#include <stdlib.h>

typedef enum TRIE_PACKAGE_ERROR{
    MEMORY_ERROR,
} TRIE_PACKAGE_ERROR;

TRIE_PACKAGE_ERROR trie_package_error;
size_t trie_package_error_message_len = 0;
char *trie_package_error_message = NULL;

void add_to_trie_package_error_message(char *message){
    size_t prev_len = trie_package_error_message_len;
    if (trie_package_error_message == NULL){
        trie_package_error_message_len = strlen(message);
        trie_package_error_message = malloc(sizeof(char) * (trie_package_error_message_len + 2));
        if (trie_package_error_message == NULL){
            return;
        }
        trie_package_error_message[0] = '\0';
    }else{
        trie_package_error_message_len = trie_package_error_message_len + strlen(message);
        char *temp = realloc(trie_package_error_message, sizeof(char) * (trie_package_error_message_len + 2));
        if (temp == NULL){
            return;
        }
        trie_package_error_message = temp;
    }

    trie_package_error_message[prev_len] = '\n';
    trie_package_error_message[prev_len + 1] = '\0';
    strcpy(trie_package_error_message + prev_len, message);
}

char *get_trie_package_error_message(){
    return trie_package_error_message;
}

void free_trie_package_error_message(){
    if (trie_package_error_message != NULL){
        free(trie_package_error_message);
    }
}

Trie *trie_build(char *value, size_t value_size){
    atexit(free_trie_package_error_message);
    Trie *trie = trie_new(EMPTY_CHAR, 0);
    if (trie == NULL){
        add_to_trie_package_error_message("unable to initizlize trie");
        return NULL;
    }

    int res = trie_add(trie, value, value_size);
    if (!res){
        free(trie);
        add_to_trie_package_error_message("unable to add value to initzlied trie");
        return NULL;
    }

    return trie;
}

Trie *trie_new(int value, int is_terminal){
    Trie *trie = malloc(sizeof(Trie));
    if (trie == NULL){
        add_to_trie_package_error_message("unable to allocate memory for new trie");
        trie_package_error = MEMORY_ERROR;
        return NULL;
    }

    trie->value = value;
    trie->is_terminal = is_terminal;

    for (int i = 0; i < MAX_CHILDREN; i++){
        trie->children[i] = NULL;
    }

    return trie;
}

Trie *trie_search(Trie *trie, char *value, size_t value_size){
    Trie *curr, *temp, *prev;
    size_t i;

    curr = trie;
    prev = curr;
    for (i = 0; i < value_size; i++){
        temp = curr->children[value[i] - 'a'];
        if (temp == NULL){
            return prev;;
        }

        prev = temp;
        curr = temp;
    }

    return curr;
}

int trie_add(Trie *trie, char *value, size_t value_size){
    Trie *curr, *new;
    size_t i;

    curr = trie_search(trie, value, value_size);
    if (curr == NULL){
        return 0;
    }

    for (i = 0; i < value_size; i++){
        new = trie_new(value[i], 0);
        if (new == NULL){
            add_to_trie_package_error_message("unable to create new trie");
            return 0;
        }

        curr->children[value[i] - 'a'] = new;
    }
    new->is_terminal = 1;

    return 1;
}

int trie_delete(Trie *trie, char *value, size_t value_size){
    return 1;
}

void trie_print(Trie *trie){
    Trie *child;
    int i, queue_len;

    queue_len = 0;
    Trie *queue[MAX_CHILDREN];
    printf("parent: %c\n", trie->value);
    for (i = 0; i < MAX_CHILDREN; i++){
        child = trie->children[i];
        if (child != NULL){
            printf("%c -- ", child->value);
            queue[queue_len++] = child;
        }
    }
    printf("\n\n");
    for (i = 0; i < queue_len; i++){
        trie_print(queue[i]);
    }
}

void free_trie(Trie *trie){
    for (int i = 0; i < MAX_CHILDREN; i++){
        if (trie->children[i] != NULL){
            free_trie(trie->children[i]);
        }
    }

    free(trie);
}

int main(void){
    Trie *trie = trie_build("world", strlen("world"));
    if (trie == NULL){
        fprintf(stderr, "unable to build trie\n %s", get_trie_package_error_message());
        return 1;
    }

    trie_print(trie);
    free_trie(trie);
    return 0;
}
