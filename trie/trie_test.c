#include "trie.h"
#include <stdio.h>
#include <string.h>

int test_build_empty(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("", 0);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_build_empty\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_build_outside_char_range(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("你好", 2);
    if (trie != NULL){
        trie_free(trie);
        strcpy(error, "expected trie not to exists at test_build_outsize_char_range\n");
        return 0;
    }

    return 1;
}

int test_build_invalid_size_less(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    char *word = "hello";
    Trie *trie = trie_build(word, strlen(word) - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_build_invalid_size_less\n");
        trie_free(trie);
        return 0;
    }

    Trie *temp = trie_search(trie, word, strlen(word));
    if (temp != NULL){
        strcpy(error, "expected not to find %s at test_build_invalid_size_less\n");
        trie_free(trie);
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_build_invalid_size_greater(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    char *word = "hello";
    Trie *trie = trie_build(word, strlen(word) + 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_build_invalid_size_greater\n");
        return 0;
    }

    Trie *temp = trie_search(trie, word, strlen(word));
    if (temp != NULL){
        strcpy(error, "expected not to find %s at test_build_invalid_size_greater\n");
        trie_free(trie);
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_build_pass(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("hello", strlen("hello") - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_build_pass\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_add_outside_char_range(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    char *word = "";
    Trie *trie = trie_build(word, strlen(word));
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_add_outsize_char_range\n");
        return 0;
    }

    char *value = "你好";
    int res = trie_add(trie, value, strlen(value));
    if (res){
        strcpy(error, "expected res to be 0 when adding at test_add_outsize_char_range\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_add_same_word_multiple(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    char *word = "";
    Trie *trie = trie_build(word, strlen(word));
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_add_same_word_multiple\n");
        return 0;
    }

    char *value = "hello";
    int i, res, count;
    count = 3;
    for (i = 0; i < count; i++){
        res = trie_add(trie, value, strlen(value));
        if (!res){
            strcpy(error, "expected res to be 0 when adding at test_add_same_word_multiple\n");
            return 0;
        }
    }

    Trie *temp = trie_search(trie, value, strlen(value));
    if (temp == NULL){
        strcpy(error, "expected find word after insertion at test_add_same_word_multiple\n");
        trie_free(trie);
        return 0;
    }

    if (temp->word_count != count){
        strcpy(error, "expected find word %s times got %s after insertion at test_add_same_word_multiple\n");
        trie_free(trie);
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_add(char error[256]){
    Trie *trie = trie_build("hello", strlen("hello") - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_add\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_add_pass(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("hello", strlen("hello") - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_add_pass\n");
        return 0;
    }

    char *value = "hello";
    int res = trie_add(trie, value, strlen(value));
    if (!res){
        strcpy(error, "expected res to be 0 when adding at test_add_pass\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_delete_not_in_trie(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("hello", strlen("hello") - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_delete_not_in_trie\n");
        return 0;
    }

    char *value = "hello";
    int res = trie_add(trie, value, strlen(value));
    if (!res){
        trie_free(trie);
        strcpy(error, "expected res to be 0 when adding at test_delete_not_in_trie\n");
        return 0;
    }

    value = "notin";
    res = trie_delete(trie, value, strlen(value));
    if (res){
        trie_free(trie);
        strcpy(error, "expected res to be 0 when adding at test_delete_not_in_trie\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_delete_inserted_multiple(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("hello", strlen("hello") - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_delete_inserted_multiple\n");
        return 0;
    }

    char *value = "hello";
    int res;
    int count = 3;
    for (int i = 0; i < count; i++){
        res = trie_add(trie, value, strlen(value));
        if (!res){
            trie_free(trie);
            strcpy(error, "expected res to be 0 when adding at test_delete_inserted_multiple\n");
            return 0;
        }
    }

    res = trie_delete(trie, value, strlen(value));
    if (!res){
        strcpy(error, "expected res to be 1 when adding at test_delete_inserted_multiple\n");
        trie_free(trie);
        return 0;
    }
    Trie *temp = trie_search(trie, value, strlen(value));
    if (temp == NULL){
        strcpy(error, "expected to find word after deletion at test_delete_inserted_multiple\n");
        trie_free(trie);
        return 0;
    }

    if (temp->word_count != count - 1){
        strcpy(error, "expected to count to be got after deletion at test_delete_inserted_multiple\n");
        trie_free(trie);
        return 0;
    }

    trie_free(trie);
    return 1;
}

int test_delete_pass(char function_name[256], char error[256]){
    strcpy(function_name, __func__);
    Trie *trie = trie_build("hello", strlen("hello") - 2);
    if (trie == NULL){
        strcpy(error, "expected trie to exists got null at test_delete_pass\n");
        return 0;
    }

    char *value = "hello";
    int res = trie_add(trie, value, strlen(value));
    if (!res){
        trie_free(trie);
        strcpy(error, "expected res to be 1 when adding at test_delete_pass\n");
        return 0;
    }

    res = trie_delete(trie, value, strlen(value));
    if (!res){
        trie_free(trie);
        strcpy(error, "expected res to be 1 when adding at test_delete_pass\n");
        return 0;
    }

    trie_free(trie);
    return 1;
}

int runner_build(char function_name[256], char error[256], int *success_tests){
    int num_tests = 0;
    printf("BUILD TRIE TESTS\n\n");

    if (!test_build_empty(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_build_outside_char_range(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_build_invalid_size_less(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_build_invalid_size_greater(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_build_pass(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        (*success_tests)++;
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }

    return num_tests;
}

int runner_add(char function_name[256], char error[256], int *success_tests){
    int num_tests = 0;
    printf("BUILD ADD TESTS\n\n");

    if (!test_add_outside_char_range(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_add_same_word_multiple(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_add_pass(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }

    return num_tests;
}

int runner_delete(char function_name[256], char error[256], int *success_tests){
    int num_tests = 0;
    printf("BUILD DELETE TESTS\n\n");

    if (!test_delete_not_in_trie(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_delete_inserted_multiple(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }
    if (!test_delete_pass(function_name, error)){
        fprintf(stderr, "%s %s\n\n", function_name, error);
        num_tests++;
    }else{
        printf("%s test passed\n\n", function_name);
        (*success_tests)++;
        num_tests++;
    }

    return num_tests;
}

int runner(){
    char error[256];
    char function_name[256];
    error[0] = '\0';
    int res, num_tests, old_tests, success_tests, old_success_tests;
    res = 0, num_tests = 0, success_tests = 0, old_tests = 0, old_success_tests = 0;

    num_tests += runner_build(function_name, error, &success_tests);

    if (success_tests - old_success_tests == num_tests - old_tests){
        printf("all build tests passed %d\n\n", num_tests - old_tests);
    }else{
        printf("%d build tests failed from %d\n\n", num_tests - old_tests - (success_tests - old_success_tests), num_tests - old_tests);
    }
    old_tests = num_tests;
    old_success_tests = success_tests;

    num_tests += runner_add(function_name, error, &success_tests);

    if (success_tests - old_success_tests == num_tests - old_tests){
        printf("all add tests passed %d\n\n", num_tests - old_tests);
    }else{
        printf("%d add tests failed from %d\n\n", num_tests - old_tests - (success_tests - old_success_tests), num_tests - old_tests);
    }
    old_tests = num_tests;
    old_success_tests = success_tests;

    num_tests += runner_delete(function_name, error, &success_tests);

    if (success_tests - old_success_tests == num_tests - old_tests){
        printf("all delete tests passed %d\n\n", num_tests - old_tests);
    }else{
        printf("%d delete tests failed from %d\n\n", num_tests - old_tests - (success_tests - old_success_tests), num_tests - old_tests);
    }
    old_tests = num_tests;
    old_success_tests = success_tests;

    if (num_tests == success_tests){
        printf("all tests passed\n\n");
    }else{
        printf("%d tests failed from %d\n\n", num_tests - success_tests, num_tests);
    }

    return res;
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
                continue;;
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
            trie_free(trie);
            return 1;
        }

        temp = trie_search(trie, buf, buf_len);
        if (temp == NULL){
            fprintf(stderr, "test failed unable to find %s after inserstion\n", buf);
            continue;
        }
    }

    trie_free(trie);
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

