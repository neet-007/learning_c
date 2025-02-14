#include "trie.h"

typedef enum Trie_package_error{
    NO_ERROR,
    MEMORY_ERROR,
    INVALID_PRINT_TYPE,
} Trie_package_error;

Trie_package_error trie_package_error = NO_ERROR;
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
        trie_package_error_message = NULL;
    }
}

void set_trie_package_error(Trie_package_error error){
    trie_package_error = error;
}

Trie *trie_build(char *value, size_t value_size){
    atexit(free_trie_package_error_message);
    Trie *trie = trie_new(EMPTY_CHAR, 0, NULL);
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

// parent=NULL
Trie *trie_new(int value, int is_terminal, Trie *parent){
    Trie *trie = malloc(sizeof(Trie));
    if (trie == NULL){
        add_to_trie_package_error_message("unable to allocate memory for new trie");
        set_trie_package_error(MEMORY_ERROR);
        return NULL;
    }

    trie->value = value;
    trie->word_count = is_terminal;
    trie->parent = parent;

    for (int i = 0; i < MAX_CHILDREN; i++){
        trie->children[i] = NULL;
    }

    return trie;
}

Trie *trie_search(Trie *trie, char *value, size_t value_size){
    Trie *curr;
    size_t i;

    curr = trie;
    for (i = 0; i < value_size; i++){
        curr = curr->children[value[i]];
        if (curr == NULL){
            return NULL;
        }
    }

    return curr;
}

int trie_add(Trie *trie, char *value, size_t value_size){
    Trie *curr, *new;
    size_t i;
    int index;

    curr = trie;
    new = NULL;
    for (i = 0; i < value_size; i++){
        index = value[i];
        if (index < 0 || MAX_CHILDREN < index){
            add_to_trie_package_error_message("unable to add value outside the range");
            return 0;
        }
        if (curr->children[index] != NULL){
            curr = curr->children[index];
            new = curr;
            continue;
        }

        new = trie_new(index, 0, curr);
        if (new == NULL){
            add_to_trie_package_error_message("unable to create new trie");
            return 0;
        }

        curr->children[index] = new;
        curr = new;
    }
    if (new){
        new->word_count++;
    }

    return 1;
}

int trie_delete(Trie *trie, char *value, size_t value_size){
    Trie *curr, **queue, **temp;
    size_t j, queue_len, queue_size;
    int found, i;

    queue_size = MAX_CHILDREN;
    queue = malloc(sizeof(Trie) * queue_size);
    if (queue == NULL){
        set_trie_package_error(MEMORY_ERROR);
        add_to_trie_package_error_message("unable to allocate memory for queue in trie_delete");
        return 0;
    }

    curr = trie;
    queue_len = 0;
    for (i = 0; i < value_size; i++){
        curr = curr->children[value[i]];
        if (curr == NULL){
            free(queue);
            return 0;
        }
        if (queue_len >= queue_size){
            queue_size *= 2;
            temp = realloc(queue, sizeof(Trie) * queue_size);
            if (temp == NULL){
                set_trie_package_error(MEMORY_ERROR);
                add_to_trie_package_error_message("unable to reallocate memory for queue in trie_delete");
                free(queue);
                return 0;
            }
        }
        queue[queue_len++] = curr;
    }

    for (i = queue_len - 1; i >= 0; i--){
        found = 0;
        curr = queue[i];
        for (j = 0; j < MAX_CHILDREN; j++){
            if (curr->children[j] != NULL){
                found = 1;
                break;
            }
        }

        if (curr->word_count > 1){
            curr->word_count--;
            break;
        }else{
            curr->word_count = 0;
        }

        if (found){
            continue;
        }

        if (i > 0){
            queue[i - 1]->children[curr->value] = NULL;
        }else{
            trie->children[curr->value] = NULL;
        }

        free(curr);
    }

    free(queue);
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

            printf("level start\nparent: %c, word_count: %d curr_parent: %c\n", trie->value, trie->word_count,
                   trie->parent ? trie->parent->value : '\0');
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
            int i, printed;

            printed = 0;
            for (i = 0; i < MAX_CHILDREN; i++){
                child = trie->children[i];
                if (child != NULL){
                    printed = 1;
                    printf("%c", trie->value);
                    trie_print(child, print_type);
                }
            }
            if (!printed){
                printf("%c", trie->value);
            }
            if (trie->word_count){
                printf("\n");
            }
            break;
        }
        default:{
            set_trie_package_error(INVALID_PRINT_TYPE);
            add_to_trie_package_error_message("invalid print type for trie");
            return;
        }
    }
}

void trie_free(Trie *trie){
    for (int i = 0; i < MAX_CHILDREN; i++){
        if (trie->children[i] != NULL){
            trie_free(trie->children[i]);
        }
    }

    free(trie);
}
