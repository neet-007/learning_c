#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>

#define ALLOWED_FLAGS_MESSAGE "only flags allowed are s same_row_count t trim_whitespace e whitespace_empty\n"

#ifdef _WIN32
    #define STRCASECMP(s1, s2) stricmp(s1, s2)
#else
    #define STRCASECMP(s1, s2) strcasecmp(s1, s2) 
#endif

typedef struct Row{
    char **fields;
    int fields_count;
    int fields_size;
    int row_bytes_size;
}Row;

typedef struct Csv{
    bool same_row_count;
    bool trim_whitespace;
    bool whitespace_empty;
    int max_row_count;
    int row_count; 
    char *filename;
    Row *rows;
}Csv;

int parse_row(FILE *f, Csv *csv){
#define REALLOC(var_name, size, len, type) \
    do{\
       (size) *= 2;\
       void *new_var_name = realloc((var_name), sizeof(type) * (size));\
       if (new_var_name == NULL){\
           fprintf(stderr, "unable to allocate memory" #var_name "realloc\n");\
           free(buf);\
           free(fields);\
           return -1;\
       }\
       (var_name) = (type *)new_var_name;\
       memset((var_name) + (len), 0, sizeof(type) * ((size) - (len)));\
    } while(false)

#define ADD_TO_BUF() \
    do{ \
        if (buf_len >= buf_size){\
            REALLOC(buf, buf_size, buf_len, char);\
        }\
        \
        buf[buf_len++] = c;\
    } while(false)

#define ADD_FIELD()\
    do{\
       if (buf_len >= buf_size){\
           REALLOC(buf, buf_size, buf_len, char);\
       }\
       buf[buf_len] = '\0';\
       if (csv->whitespace_empty){\
           int i = 0;\
           for (i = 0; i < buf_len; i++){\
               if (!isspace(buf[i])){\
                   break;\
               }\
           }\
           if (i == buf_len){\
               buf_len = 0;\
               break;\
           }\
       }\
       char *field_copy = calloc(sizeof(char), (buf_len + 1));\
       if (field_copy == NULL){\
           fprintf(stderr, "unable to allocate memory field copy\n");\
           free(buf);\
           free(fields);\
           return -1;\
       }\
       strcpy(field_copy, buf);\
       if (fields_count >= fields_size){\
           REALLOC(fields, fields_size, fields_count, char *);\
       }\
       fields[fields_count++] = field_copy;\
       row_bytes_size += buf_len;\
       buf_len = 0;\
       break;\
\
    } while(false)

    int row_bytes_size = 0;
    size_t fields_count = 0;
    size_t fields_size = csv->max_row_count == 0 ? 8 : csv->max_row_count;
    char **fields = calloc(sizeof(char *), fields_size);
    if (fields == NULL){
        fprintf(stderr, "unable to allocate memory row\n");
        return -1;
    }
 
    size_t buf_size = 16;
    size_t buf_len = 0; 
    char *buf = calloc(sizeof(char), buf_size);
    if (buf == NULL){
        fprintf(stderr, "unable to allocate memory buf\n");
        free(fields);
        return -1;
    }

    int c;
    bool parse_escape = false;
    bool flag = false;
    c = fgetc(f);
    if (c == EOF){
        free(buf);
        free(fields);
        return 0;
    }

    while(!flag){
        if (csv->same_row_count && (csv->max_row_count != 0 && fields_count == csv->max_row_count)){
            fprintf(stderr, "same_row_count enabled and row has more fields %ld vs %d at row %d field %ld\n", fields_count + 1, csv->max_row_count, csv->row_count + 1, fields_count + 1);
            free(buf);
            free(fields);
            return -1;
        }

        switch (c){
            case ',':{
                 if (!parse_escape){
                     ADD_FIELD();
                     break;
                 }
                 ADD_TO_BUF();
                 break;
            }
            case '\r':
            case '\n':
            case EOF:{
                if (!parse_escape){
                    ADD_FIELD();
                    flag = true;
                    break;
                }
                if (c == EOF){
                    free(buf);
                    free(fields);
                    fprintf(stderr, "unterminated escape \" at row %d at field %ld\n", csv->row_count + 1, fields_count + 1);
                    return -1;
                }
                ADD_TO_BUF();
                break;
            }
            case '"':{
                if (!parse_escape){
                    parse_escape = true;
                    break;
                }

                c = fgetc(f);
                if (c == '"'){
                    ADD_TO_BUF();
                    break;
                }

                while(isspace(c) && c != '\r' && c != '\n'){
                    c = fgetc(f);    
                }
                if (c != ',' && c != '\r' && c != '\n' && c != EOF){
                    free(buf);
                    free(fields);
                    fprintf(stderr, "unsupported use of termination charector \" at row %d at field %ld\n", csv->row_count + 1, fields_count + 1);
                    return -1;
                }
                if (c == '\r' || c == '\n' || c == EOF){
                    flag = true;
                }
                parse_escape = false;
                ADD_FIELD();
                break;
            }
            default:{
                 if (csv->trim_whitespace && isspace(c) && !parse_escape){
                     break;
                 }
                 ADD_TO_BUF();
                 break;
            }
        }
        if (!flag){
            c = fgetc(f);
        }
    }

    if (csv->max_row_count == 0){
        csv->max_row_count = fields_count;
    } else if(csv->same_row_count && fields_count != csv->max_row_count){
        fprintf(stderr, "same_row_count enabled but row does not match %d vs %ld at row %d at field %ld\n", csv->max_row_count, fields_count, csv->row_count + 1, fields_count + 1);

        free(buf);
        free(fields);
        return -1;
    }

    Row row = {.fields=fields, .fields_count=fields_count, .fields_size=fields_size, .row_bytes_size=row_bytes_size};
    csv->rows[csv->row_count++] = row;
    free(buf);
    
#undef REALLOC
#undef ADD_TO_BUF
#undef ADD_FIELD
    
    if (c == EOF){
        return 0;
    }
    return 1;
}

char *csv_row_to_line(Row *row, int *curr);
void print_csv(Csv *csv){
    printf("filename: %s\n", csv->filename);
    printf("same_row_count %d\n", csv->same_row_count);
    printf("trim_whitespace %d\n", csv->trim_whitespace);
    printf("whitespace_empty %d\n", csv->whitespace_empty);
    printf("max_row_count %d\n", csv->max_row_count);
    for (int i = 0; i < csv->row_count; i++){
        Row row = csv->rows[i];
        printf("row %d ", i + 1);
        for (int j = 0; j < row.fields_count; j++){
            if (row.fields[j] == NULL){
                continue;
            }
            printf(" -- %s", row.fields[j]);
        }
        printf("\n");
    }
}

void free_csv(Csv *csv){
    free(csv->filename);

    for (int i = 0; i < csv->row_count; i++){
        Row row = csv->rows[i];
        for (int j = 0; j < row.fields_count; j++){
            if (row.fields[j] == NULL){
                continue;
            }
            free(row.fields[j]);
        }
        if (row.fields != NULL){
            free(row.fields);
        }
    }
    if (csv->rows != NULL){
        free(csv->rows);
    }
    free(csv);
}

char *csv_row_to_line(Row *row, int *curr){
    char *row_line = calloc(row->row_bytes_size + 2, sizeof(char));
    if (row_line == NULL){
        return NULL;
    }
    
    *curr = 0;
    int field_len = 0;
    for (int i = 0; i < row->fields_count; i++){
        field_len = strlen(row->fields[i]);
        memcpy(row_line + *curr, row->fields[i], field_len);
        (*curr) += field_len;
    }
    if (*curr != 0){
        row_line[(*curr)++] = '\n';
    }
    row_line[(*curr)] = '\0';
    
    return row_line;
}

Csv *parse_csv(char *filename, bool same_row_count, bool trim_whitespace, bool whitespace_empty);
void test_parser(char *test_dir){
    DIR *dir;
    struct dirent *entry;
    char s[3072];
    size_t exp_filename_size = 16;
    char *exp_filename;
    int count = 0;
    int success = 0;
    bool fail = false;

    dir = opendir(test_dir);
    if (dir == NULL){
        fprintf(stderr, "unable to open test dir %s\n", test_dir);
        return;
    }

    exp_filename = calloc(exp_filename_size, sizeof(char));
    if (exp_filename == NULL){
        closedir(dir);
        fprintf(stderr, "unable to allocate memory for exp filename intial\n");
        return;
    }

    printf("running tests\nwell run any file that has the pattern test*.csv\n\n");
    while((entry = readdir(dir)) != NULL){
        fail = false;
        if (strncmp("test", entry->d_name, 4) != 0){
            continue;
        }
        const char *ext = strrchr(entry->d_name, '.');
        if (ext == NULL || strcmp(ext,  ".csv") != 0){
            continue;
        }

        if (sizeof(char) * ((strlen(test_dir) + strlen(entry->d_name) + 6)) > exp_filename_size){
            exp_filename_size = (strlen(test_dir) + strlen(entry->d_name) + 6) * 2;
            char *temp = realloc(exp_filename, sizeof(char) * exp_filename_size);
            if (temp == NULL){
                closedir(dir);
                free(exp_filename);
                fprintf(stderr, "unabel to rallocate memory for exp filename %s\n", entry->d_name);
                return;
            }
            exp_filename = temp;
        }

        int res = sprintf(exp_filename, "%s/%s_exp",test_dir, entry->d_name);
        if (res < 0){
            free(exp_filename);
            closedir(dir);
            fprintf(stderr, "unable to format test name test %s\n", entry->d_name);
            return;
        }
        FILE *exp = fopen(exp_filename, "r");
        if (exp == NULL){
            fprintf(stderr, "unable to open test exp file test %s filename %s\n", entry->d_name, exp_filename);
            continue;
        }

        count++;
        printf("test %s exp %s started\n", entry->d_name, exp_filename);

        bool same_row_count = false;
        bool trim_whitespace= false;
        bool whitespace_empty = false;
        char *filename = calloc(strlen(test_dir) + strlen(entry->d_name) + 2, sizeof(char));
        if (filename == NULL){
            free(exp_filename);
            closedir(dir);
            fprintf(stderr, "unable to allocate memory test name test %s\n", entry->d_name);
            return;
        }
        res = sprintf(filename, "%s/%s",test_dir, entry->d_name);
        if (res < 0){
            free(exp_filename);
            free(filename);
            closedir(dir);
            fprintf(stderr, "unable to format test name test %s\n", entry->d_name);
            return;
        }
        Csv *csv = parse_csv(filename, same_row_count, trim_whitespace, whitespace_empty);
        if (csv == NULL){
            fgets(s, sizeof s, exp);
            if (s == NULL){
                fprintf(stderr, "cannot read line test %s test exp %s\n", entry->d_name, exp_filename);
                free(filename);
                continue;
            }
            if (strcmp(s, "error\n") != 0){
                fail = true;
                fprintf(stderr, "error at test %s test exp %s\n", entry->d_name, exp_filename);
            }else{
                success++;
                printf("test %s exp %s expeted to fail and failed\n", entry->d_name, exp_filename);
            }
            fclose(exp);
            free(filename);
            printf("end\n");
            continue;
        }

        for (int i = 0; i < csv->row_count; i++){
            int curr = 0;
            char *row_to_line = csv_row_to_line(&csv->rows[i], &curr);
            if (row_to_line == NULL){
                fprintf(stderr, "row_to_line is null test %s test exp %s row %d", entry->d_name, exp_filename, i + 1);
                fail = false;
                continue;
            }
            s[0] = '\0';
            fgets(s, sizeof s, exp);
            if (s == NULL){
                fprintf(stderr, "cannot read line test %s test exp %s row %d", entry->d_name, exp_filename, i + 1);
                free(row_to_line);
                fail = false;
                continue;
            }
            if (strcmp(s, row_to_line) != 0){
                fail = true;
                fprintf(stderr, "rows dont match test %s test exp %s row %d\n%s\nvs\n%s\n", entry->d_name, exp_filename, i + 1, row_to_line, s);
            }
            free(row_to_line);
        }
        if (!fail){
            success++;
        }
        printf("end\n");
        free(filename);
        free_csv(csv);
        fclose(exp);
    }
    
    free(exp_filename);
    closedir(dir);
    printf("run all tests found %d success %d fail %d\n", count, success, count - success);
}

char *parse_cli(int argc, char *argv[], bool *same_row_count, bool *trim_whitespace, bool *whitespace_empty){
    if (strcmp("test", argv[1]) == 0){
        test_parser(argv[2]);
        return "___test___";
    }
    if (argc < 1){
        fprintf(stderr, "usage: must atleast pass filename");
        return NULL;
    }
    if (argc > 6){
        fprintf(stderr, "usage: maximum arguments count is 5");
        return NULL;
    }
    
    *same_row_count = false;
    *trim_whitespace = false;
    *whitespace_empty = false;
    
    char *filename = NULL;
    
    for (int i = 1; i < argc; i++){
        size_t curr_len = strlen(argv[i]);
        if (argv[i][0] == '-'){
            if (curr_len > 2 && argv[i][1] == '-'){
                if (STRCASECMP(argv[i] + 2, "help") == 0){
                    printf("usage: csv_parser filename --flags same_row_count trim_whitesape whitespace_empty");
                    exit(EXIT_SUCCESS);
                }
                if (STRCASECMP(argv[i] + 2, "same_row_count") == 0){
                    *same_row_count = true;
                    continue;
                }
                if (STRCASECMP(argv[i] + 2, "trim_whitespace") == 0){
                    *trim_whitespace = true;
                    continue;
                }
                if (STRCASECMP(argv[i] + 2, "whitespace_empty") == 0){
                    *whitespace_empty = true;
                    continue;
                }
                fprintf(stderr, ALLOWED_FLAGS_MESSAGE);
                return NULL;
            }
            
            if (curr_len > 2){
                for (int j = 1; j < curr_len; j++){
                    switch (argv[i][j]){
                        case 's':
                        case 'S':{
                            *same_row_count = true;            
                            break;
                        }
                        case 't':
                        case 'T':{
                            *trim_whitespace = true;
                            break;
                        }
                        case 'e':
                        case 'E':{
                            *whitespace_empty = true;     
                            break;
                        }
                        default:{
                            fprintf(stderr, ALLOWED_FLAGS_MESSAGE);
                            return NULL;
                        }
                    }
                }
                continue;
            }
            
            switch (argv[i][1]){
                case 's':
                case 'S':{
                    *same_row_count = true;            
                    break;
                }
                case 't':
                case 'T':{
                    *trim_whitespace = true;
                    break;
                }
                case 'e':
                case 'E':{
                    *whitespace_empty = true;     
                    break;
                }
                default:{
                    fprintf(stderr, ALLOWED_FLAGS_MESSAGE);
                    return NULL;
                }
            }
            
            continue;
        }
        const char *ext = strrchr(argv[i], '.');

        if (ext == NULL || strcmp(ext, ".csv") != 0) {
            fprintf(stderr, "must pass file with extention .csv\n");
            return NULL;
        }
    
        filename = calloc(sizeof(char), (curr_len + 1));
        if (filename == NULL){
            fprintf(stderr, "unable to allocate memory filename\n");
            return NULL;
        }
        strncpy(filename, argv[i], curr_len);
    }
    
    return filename;
}

Csv *parse_csv(char *filename, bool same_row_count, bool trim_whitespace, bool whitespace_empty){
    if (filename == NULL){
        return NULL;
    }

    Row *rows = malloc(sizeof(Row) * 8);
    if (rows == NULL){
        fprintf(stderr, "unable to allocate memory for initial rows filename %s\n", filename);
        return NULL;
    }

    Csv *csv = malloc(sizeof(Csv));
    if (csv == NULL){
        fprintf(stderr, "unable to allocate memory for csv filename %s\n", filename);
        free(rows);
        return NULL;
    }

    csv->same_row_count = same_row_count;
    csv->trim_whitespace = trim_whitespace;
    csv->whitespace_empty = whitespace_empty;
    csv->max_row_count = 0;
    csv->row_count = 0;
    csv->rows = rows;
    char *csv_filename = calloc(strlen(filename) + 1, sizeof(char));
    if (csv_filename == NULL){
        fprintf(stderr, "unable to allocate memory for csv filename filename %s\n", filename);
        free(rows);
        free(csv);
        return NULL;
    }
    strcpy(csv_filename, filename);
    csv->filename = csv_filename;

    FILE *f = fopen(csv_filename, "r");
    if (f == NULL){
        free(rows);
        free(csv);
        fprintf(stderr, "unable to open file %s\n", csv_filename);
        return NULL;
    }

    int res = 1;
    while(res > 0){
        res = parse_row(f, csv);
    }

    if (res == -1){
        free_csv(csv);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return csv;
}

int main(int argc, char *argv[]){
    bool same_row_count = false;
    bool trim_whitespace = false;
    bool whitespace_empty = false;
    char *filename = parse_cli(argc, argv, &same_row_count, &trim_whitespace, &whitespace_empty);
    if (filename == NULL){
        return 1;
    }

    if (strcmp(filename, "___test___") == 0){
        return 0;
    }

    Csv *csv = parse_csv(filename, same_row_count, trim_whitespace, whitespace_empty);
    if (csv == NULL){
        if (filename != NULL){
            free(filename);
        }
        return 1;
    }

    print_csv(csv);
    free_csv(csv);

    return 0;
}
