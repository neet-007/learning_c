#include "trie.h"

int test_add(){

    return 0;
}

int test_build_empty(char error[256]){
    Trie *trie = trie_build("", 0);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_build_empty\n");
        return 0;
    }
    free_trie(trie);
    return 1;
}

int test_build_outside_char_range(){

    return 1;
}

int test_build_invalid_size(){

    return 1;
}

int test_build_pass(){

    return 1;
}

int test_interactive(){
    printf("values passed here should not be longer that 255 characters\n");
    char buf[256];
    int res;
    Trie *temp;
    int i;
    size_t buf_len, print_len, delete_len;
    print_len = strlen("print");
    delete_len = strlen("delete");

    Trie *trie = trie_build("", 0);
    if (trie == NULL){
        fprintf(stderr, "error while building trie: %s", get_trie_package_error_message());
        return 1;
    }

    while (fgets(buf, sizeof(buf) - 1, stdin)){
        i = 0;
        while (buf[i] != '\n' && buf[i] != '\0'){
            i++;
        }

        if (buf[i] == '\n'){
            buf[i] = '\0';
        }

        if (strncmp(buf, "print", print_len) == 0){
            if (strcmp(buf + print_len + 1, "string") == 0){
                trie_print(trie, PRINT_STRING);
            }else if (strcmp(buf + print_len + 1, "levels") == 0){
                trie_print(trie, PRINT_LEVELS);
            }else{
                fprintf(stderr, "invalid option for print [string] or [levels]\n");
            }
            continue;
        }
        if (strncmp(buf, "delete", delete_len) == 0){
            buf_len = strlen(buf + delete_len + 1);
            res = trie_delete(trie, buf + delete_len + 1, buf_len);
            if (!res){
                fprintf(stderr, "unable to delete %s to trie error: %s\n", buf, get_trie_package_error_message());
                free_trie(trie);
                return 1;
            }

            temp = trie_search(trie, buf, buf_len);
            if (temp != NULL){
                fprintf(stderr, "test failed unable to unfind %s after inserstion\n", buf);
                continue;
            }
            continue;
        }
        buf_len = strlen(buf);

        res = trie_add(trie, buf, buf_len);
        if (!res){
            fprintf(stderr, "unable to add %s to trie error: %s\n", buf, get_trie_package_error_message());
            free_trie(trie);
            return 1;
        }

        temp = trie_search(trie, buf, buf_len);
        if (temp == NULL){
            fprintf(stderr, "test failed unable to find %s after inserstion\n", buf);
            continue;
        }
    }

    free_trie(trie);
    return 0;
}

int runner(){
    char error[256];
    error[0] = '\0';
    if (!test_build_empty(error)){
        fprintf(stderr, "%s", error);
    }else{
        printf("test_build_empty test passed\n");
    }
    return 0;
}

int main(int argc, char *argv[]){
    char *normal = "normal";
    char *interactive = "interactive";

    if (argc < 2){
        fprintf(stderr, "test runner requiers type argument [%s] or [%s]\n", normal, interactive);
        return 0;
    }

    if (strcmp(argv[1], normal) == 0){
        return runner();
    }
    if (strcmp(argv[1], interactive) == 0){
        return test_interactive();
    }

    fprintf(stderr, "test runner requiers type argument [%s] or [%s]\n", normal, interactive);
    return 0;
}

