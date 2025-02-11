#include "trie.h"

typedef enum Trie_package_error{
    MEMORY_ERROR,
    INVALID_PRINT_TYPE,
} Trie_package_error;

Trie_package_error trie_package_error;
size_t trie_package_error_message_len = 0;
char *trie_package_error_message = NULL;

void add_to_trie_package_error_message(char *message){
    size_t prev_len = trie_package_error_message ? trie_package_error_message_len : 0;

    trie_package_error_message_len = prev_len + strlen(message);

    char *temp = realloc(trie_package_error_message, sizeof(char) * (trie_package_error_message_len + 2));
    if (temp == NULL) {
        return;
    }
    trie_package_error_message = temp;

    strcpy(trie_package_error_message + prev_len, message);

    trie_package_error_message[trie_package_error_message_len] = '\n';
    trie_package_error_message[trie_package_error_message_len + 1] = '\0';
}

char *get_trie_package_error_message(){
    return trie_package_error_message;
}

void free_trie_package_error_message(){
    if (trie_package_error_message != NULL){
        free(trie_package_error_message);
    }
}

void set_trie_package_error(Trie_package_error error){
    trie_package_error = error;
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
        set_trie_package_error(MEMORY_ERROR);
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
    Trie *curr, *temp;
    size_t i;

    curr = trie;
    for (i = 0; i < value_size; i++){
        temp = curr->children[value[i] - 'a'];
        if (temp == NULL){
            return NULL;
        }

        curr = temp;
    }

    return curr;
}

int trie_add(Trie *trie, char *value, size_t value_size){
    Trie *curr, *new;
    size_t i;
    int index;

    curr = trie;
    for (i = 0; i < value_size; i++){
        new = trie_new(value[i], 0);
        if (new == NULL){
            add_to_trie_package_error_message("unable to create new trie");
            return 0;
        }
        index = value[i] - 'a';

        while(curr->children[index] != NULL){
            curr = curr->children[index];
        }

        curr->children[index] = new;
        curr = new;
    }
    new->is_terminal = 1;

    return 1;
}

int trie_delete(Trie *trie, char *value, size_t value_size){
    return 1;
}

void trie_print(Trie *trie, Trie_package_print_type print_type){
    switch (print_type) {
        case PRINT_LEVELS:{
            Trie *child, **temp_queue;
            int i;
            size_t queue_len;

            queue_len = 0;
            Trie *queue[MAX_CHILDREN];

            printf("level start\nparent: %c\n", trie->value);
            for (i = 0; i < MAX_CHILDREN; i++){
                child = trie->children[i];
                if (child != NULL){
                    printf("%c -- ", child->value);
                    queue[queue_len++] = child;
                }
            }
            printf("\nlevel end\n\n");
            for (i = 0; i < queue_len; i++){
                trie_print(queue[i], print_type);
            }
            break;
        }
        case PRINT_STRING:{
            Trie *child;
            int i;

            printf("%c", trie->value);
            for (i = 0; i < MAX_CHILDREN; i++){
                child = trie->children[i];
                if (child != NULL){
                    trie_print(child, print_type);
                }
            }
            printf("\n");
            break;
        }
        default:{
            set_trie_package_error(INVALID_PRINT_TYPE);
            add_to_trie_package_error_message("invalid print type for trie");
            return;
        }
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
    char *word = "helloworld";
    Trie *trie = trie_build(word, strlen(word));
    if (trie == NULL){
        fprintf(stderr, "unable to build trie\n %s", get_trie_package_error_message());
        return 1;
    }

    trie_print(trie, PRINT_STRING);
    free_trie(trie);
    return 0;
}
