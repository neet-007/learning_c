#include "ini_parser.h"

int valid_char(char c){
    return (isalnum(c) || c == '_' || c == '-' || c == ' ' || c == '\t' || c == ':' || c == '.');
}

char *strip_str(char *s) {
    size_t size = strlen(s);
    char *end;

    if (size == 0) {
        return s;
    }

    end = s + size - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    return s;
}

char *parse_comment(FILE *file, int *len){
    char buf;   
    ssize_t buf_read;

    int size = 128;
    char *comment = malloc(sizeof(char) * size);
    if (comment == NULL){
        return NULL;
    }

    while ((buf = fgetc(file)) != EOF) {
        switch (buf) {
            case '\r':
            case '\n':{
                if (*len >= size){
                    size += 2;
                    comment = realloc(comment, sizeof(char) * size);
                    if (comment == NULL){
                        return NULL;
                    }
                }
                comment[(*len)++] = buf;
                comment[(*len)] = '\0';
                return comment;
            }

            default:{
                if (*len >= size){
                    size *= 2;
                    comment = realloc(comment, sizeof(char) * size);
                    if (comment == NULL){
                        return NULL;
                    }
                }
                comment[(*len)++] = buf;
            }
        }
    }

    if (comment){
        free(comment);
    }
    return NULL;
}

char *parse_section(FILE *file, int *len){
    char buf;   
    ssize_t buf_read;

    int size = 128;
    char *section = malloc(sizeof(char) * size);
    if (section == NULL){
        return NULL;
    }
    while ((buf = fgetc(file)) != EOF) {
        switch (buf) {
            case '#':
            case ';':
            case '[':{
                free(section);
                return NULL;
            }

            case ']':{
                if (*len >= size){
                    size += 2;
                    char *temp = realloc(section, size);
                    if (temp == NULL) {
                        free(section);
                        return NULL;
                    }
                    section = temp;

                }
                section[(*len)++] = '\n';
                section[(*len)] = '\0';
                return strip_str(section);
            }

            default:{
                if (!valid_char(buf)){
                    free(section);
                    return NULL;
                }
                if (*len >= size){
                    size *= 2;
                    char *temp = realloc(section, size);
                    if (temp == NULL) {
                        free(section);
                        return NULL;
                    }
                    section = temp;
                }
                section[(*len)++] = buf;
            }
        }       
    }

    if (section){
        free(section);
    }
    return NULL;
}

int parse_key_val(FILE *file, int *key_len, int *val_len, char **key, char **val) {
    char buf;
    int is_key = 1;
    int key_size = 128;
    int val_size = 128;

    int ret = fseek(file, -1, SEEK_CUR);
    if (ret != 0){
        return 0;
    }

    *key = malloc(sizeof(char) * key_size);
    if (*key == NULL) {
        return 0;
    }

    *val = malloc(sizeof(char) * val_size);
    if (*val == NULL) {
        free(*key);
        return 0;
    }

    char q = -1;
    while ((buf = fgetc(file)) != EOF) {
        switch (buf) {
            case '"':
            case '\'':{
                if (q == -1){
                    q = buf;
                }else if (q != buf){
                    if (*key != NULL){
                        free(*key);
                    }
                    if (*val != NULL){
                        free(*val);
                    }
                    return 0;
                }else{
                    q = -1;
                }

                break;
            }

            case ']': {
                if (*key != NULL){
                    free(*key);
                }
                if (*val != NULL){
                    free(*val);
                }
                return 0;
            }

            case '#':
            case ';':
            case '\n':
            case '\r': {
                if (q != -1){
                    if (*key != NULL){
                        free(*key);
                    }
                    if (*val != NULL){
                        free(*val);
                    }
                    return 0;
                }
                if (buf == '#' || buf == ';'){
                    int comment_len = 0;
                    char *comment = parse_comment(file, &comment_len);
                    if (comment == NULL){
                        if (*key != NULL){
                            free(*key);
                        }
                        if (*val != NULL){
                            free(*val);
                        }
                        return 0;
                    }
                    //printf("found inlince comment %s\n", comment);
                    free(comment);
                }
                if (*key_len >= key_size){
                    *key = realloc(*key, ++key_size);
                    if (*key == NULL){
                        free(*val);
                        return 0;
                    }
                }
                (*key)[*key_len] = '\0';

                if (*val_len >= val_size){
                    val_size += 2;
                    *val= realloc(*val, val_size);
                    if (*val == NULL){
                        free(*key);
                        return 0;
                    }
                }
                (*val)[(*val_len)++] = '\n';
                (*val)[*val_len] = '\0';

                *key = strip_str(*key);
                *val = strip_str(*val);
                return 1;
            }

            case '=': {
                if (!is_key){
                    if (*val_len >= val_size) {
                        *val = realloc(*val, ++val_size);
                        if (*val == NULL) {
                            free(*key);
                            return 0;
                        }
                    }
                    (*val)[(*val_len)++] = buf;
                    break;
                }
                if (q != -1){
                    if (*key != NULL){
                        free(*key);
                    }
                    if (*val != NULL){
                        free(*val);
                    }
                    return 0;
                }
                is_key = 0;
                q = -1;
                break;
            }

            default: {
                if (is_key) {
                    if (*key_len >= key_size) {
                        *key = realloc(*key, ++key_size);
                        if (*key == NULL) {
                            free(*val);
                            return 0;
                        }
                    }
                    (*key)[(*key_len)++] = buf;
                } else {
                    if (*val_len >= val_size) {
                        *val = realloc(*val, ++val_size);
                        if (*val == NULL) {
                            free(*key);
                            return 0;
                        }
                    }
                    (*val)[(*val_len)++] = buf;
                }
                break;
            }
        }
    }

    if (*key != NULL){
        free(*key);
    }
    if (*val != NULL){
        free(*val);
    }
    return 0;
}

Ini *ini_parse(const char *filename){
    FILE *file = fopen(filename, "r");
    if (!file){
        return NULL;
    }
    
    HashTable *current_section = NULL;
    char buf;   
    ssize_t buf_read;

    Ini *ini = malloc(sizeof(Ini));
    if (ini == NULL){
        fclose(file);
        return NULL;
    }

    ini->sections = create_table(CAPACITY);
    if (ini->sections == NULL){
        free(ini);
        fclose(file);
        return NULL;
    }

    ini->filename = (char *) malloc(sizeof(char) * strlen(filename) + 1);
    if (ini->filename == NULL){
        fclose(file);
        free_table(ini->sections);
        free(ini);
        return NULL;
    }

    strcpy(ini->filename, filename);
    while ((buf = fgetc(file)) != EOF) {
        switch (buf) {
            case '\n':
            case '\r':{
                break;
            }

            case '#':
            case ';':{
                int len = 0;
                char *comment = parse_comment(file, &len);
                if (comment == NULL){
                    fclose(file);
                    if (current_section){
                        free(current_section);
                    }
                    free(ini->filename);
                    free_table(ini->sections);
                    free(ini);
                    return NULL;
                }
                //printf("comment:%s", comment);
                free(comment);
                break;
            }

            case '[':{
                int len = 0;
                char *section = parse_section(file, &len);
                if (section == NULL){
                    fclose(file);
                    if (current_section){
                        free(current_section);
                    }
                    free(ini->filename);
                    free_table(ini->sections);
                    free(ini);
                    return NULL;
                }
                void *ret = ht_search(ini->sections, section);
                if (ret != NULL){
                    printf("duplicate section error %s\n", section);
                    fclose(file);
                    free(ret);
                    free(section);
                    if (current_section){
                        free(current_section);
                    }
                    free(ini->filename);
                    free_table(ini->sections);
                    free(ini);
                    return NULL;
                }
                HashTable *section_table = create_table(CAPACITY);
                ht_insert(ini->sections, section, section_table, sizeof(HashTable), TYPE_HASH_TABLE);
                current_section = section_table;
                if (section){
                    free(section);
                }
                break;
            }

            default:{
                if (current_section == NULL){
                    fclose(file);
                    free(ini->filename);
                    free_table(ini->sections);
                    free(ini);
                    return NULL;
                }

                char *key;
                char *val;
                int key_len = 0;
                int val_len = 0;
                int res = parse_key_val(file, &key_len, &val_len, &key, &val);
                if (!res){
                    fclose(file);
                    if (key){
                        free(key);
                    }
                    if (val){
                        free(val);
                    }
                    if (current_section){
                        free(current_section);
                    }
                    free(ini->filename);
                    free_table(ini->sections);
                    free(ini);
                    return NULL;
                }

                ht_insert(current_section, key, val, val_len + 1 , TYPE_STR);
                if (key != NULL){
                    free(key);
                }
                if (val != NULL){
                    free(val);
                }
                break;
            }
        }       
    }

    if (current_section){
        free(current_section);
    }
    fclose(file);
    return ini;
}

Ini *ini_new(char *filename){
    Ini *ini = malloc(sizeof(Ini));
    if (ini == NULL){
        return NULL;
    }
    char *filename_copy = malloc(sizeof(char) * (strlen(filename) + 1));
    if (filename_copy == NULL){
        free(ini);
        return NULL;
    }
    ini->filename = filename_copy;
    ini->sections = create_table(CAPACITY);
    if (ini->sections == NULL){
        free(ini);
        free(filename_copy);
        return NULL;
    }

    return ini;
}

int write_ini_section(HashTable *section, FILE *f) {
    size_t val_size = 64;
    char *val = calloc(val_size, sizeof(char));
    if (val == NULL) {
        return 0;
    }

    for (int i = 0; i < section->size; i++) {
        void *item = section->items[i];
        if (item != NULL) {
            Ht_item *ht_item = (Ht_item *)item;
            switch (ht_item->value_type) {
                case TYPE_BOOL: {
                    if (*(bool *)ht_item->value) {
                        snprintf(val, val_size, "%s = true\n", ht_item->key);
                    } else {
                        snprintf(val, val_size, "%s = false\n", ht_item->key);
                    }

                    fwrite(val, sizeof(char), strlen(val), f);
                    break;
                }
                case TYPE_INT: {
                    snprintf(val, val_size, "%s = %d\n", ht_item->key, *(int *)ht_item->value);

                    fwrite(val, sizeof(char), strlen(val), f);
                    break;
                }
                case TYPE_STR: {
                    snprintf(val, val_size, "%s = %s\n", ht_item->key, (char *)ht_item->value);

                    fwrite(val, sizeof(char), strlen(val), f);
                    break;
                }
                case TYPE_HASH_TABLE: {
                    break;
                }
                default: {
                    free(val);
                    return 0;
                }
            }
        }
    }

    free(val);
    return 1;
}

int ini_write(Ini *ini, FILE *f){
    int res;
    size_t val_size = 64;
    size_t val_len = 0;
    int digits_count;
    char *val;

    val = malloc(sizeof(char) * val_size);
    if (val == NULL){
        return 0;
    }

    for (int i = 0; i < ini->sections->size; i++) {
        Ht_item *item = ini->sections->items[i];
        if (item != NULL) {
            char *str_val = item->key;
            val_len = 2 + strlen(str_val) + 1;
            if (val_len > val_size){
                val_size *= 2;
                char *temp = realloc(val, sizeof(char) * val_size);
                if (temp == NULL){
                    free(val);
                    return 0;
                }
            }

            sprintf(val, "[%s]\n", str_val);
            fwrite(val, sizeof(char), val_len, f);
            res = write_ini_section((HashTable *)ini->sections->items[i]->value, f);
            if (res < 1){
                free(val);
                return 0;
            }
        }
    }

    free(val);
    return 1;
}

int ini_add_section(Ini *ini, char *section_name){
    void *ret = ht_search(ini->sections, section_name);
    if (ret != NULL){
        fprintf(stderr, "duplicate section error %s\n", section_name);
        free(ret);
        return 0;
    }

    HashTable *section_table = create_table(CAPACITY);
    ht_insert(ini->sections, section_name, section_table, sizeof(HashTable), TYPE_HASH_TABLE);
    free(section_table);
    return 1;
}

Ht_item *ini_get_key(Ini *ini, char *section_name, char *key){
    Ht_item *ret = ht_search(ini->sections, section_name);
    if (ret == NULL || ret->value_type != TYPE_HASH_TABLE){
        fprintf(stderr, "section does not exists error %s\n", section_name);
        free(ret);
        return 0;
    }
    HashTable *section = (HashTable *)ret->value;

    return ht_search(section, key);
}

int ini_set_key(Ini *ini, char *section_name, char *key, char *value){
    Ht_item *ret = ht_search(ini->sections, section_name);
    if (ret == NULL || ret->value_type != TYPE_HASH_TABLE){
        fprintf(stderr, "section does not exists error %s\n", section_name);
        free(ret);
        return 0;
    }

    HashTable *section = (HashTable *)ret->value;

    ht_insert(section, key, value, sizeof(char) * (strlen(value) + 1), TYPE_STR);
    return 1;
}

void free_ini(Ini *ini){
    free(ini->filename);
    free_table(ini->sections);
    free(ini);
}

void test_ini_parser(const char *directory){
    struct dirent *entry;
    DIR *dir = opendir(directory);

    if (!dir) {
        perror("Unable to open directory");
        return;
    }

    printf("INI files in directory %s:\n", directory);
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        const char *ext = strrchr(name, '.');

        if (ext && strcmp(ext, ".ini") == 0) {
            printf("- %s\n", name);

            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory, name);

            Ini *ini = ini_parse(file_path);
            if (ini == NULL){
                printf("failed\n");
                continue;
            }

            printf("filename %s\n", ini->filename);
            free(ini->filename);
            free_table(ini->sections);
            free(ini);
        }
    }

    closedir(dir);
}

void test_ini_parser_write(const char *directory){
    struct dirent *entry;
    DIR *dir = opendir(directory);

    if (!dir) {
        perror("Unable to open directory");
        return;
    }

    printf("INI files in directory %s:\n", directory);
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        const char *ext = strrchr(name, '.');

        if (ext && strcmp(ext, ".ini") == 0) {
            printf("- %s\n", name);

            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory, name);

            Ini *ini = ini_parse(file_path);
            if (ini == NULL){
                printf("failed\n");
                continue;
            }

            printf("filename %s\n", ini->filename);

            FILE *f = fopen("test_write.ini", "w+");
            if (f == NULL){
                fprintf(stderr, "could not open file %s\n", ini->filename);
                free(ini->filename);
                free_table(ini->sections);
                free(ini);
                fclose(f);
                continue;
            }

            int res = ini_write(ini, f);
            if (res < 1){
                fprintf(stderr, "test failed file %s\n", ini->filename);
            }

            free(ini->filename);
            free_table(ini->sections);
            free(ini);
            fclose(f);
        }
    }

    closedir(dir);
}
