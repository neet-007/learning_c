#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
       buf_len = 0;\
       break;\
\
    } while(false)

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
        return 0;
    }

    while(!flag){
        if (csv->same_row_count && (csv->max_row_count != 0 && fields_count == csv->max_row_count)){
            fprintf(stderr, "same_row_count enabled and row has more fields %d vs %d\n", fields_count + 1, csv->max_row_count);
            free(buf);
            free(fields);
            return -1;
        }
        
        switch (c){
            case ',':{
                 ADD_FIELD();
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
                    fprintf(stderr, "unterminated escape \"\n");
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
                if (c != ',' && c != EOF){
                    fprintf(stderr, "unsupported use of termination charector \"\n");
                    return -1;
                }
                ADD_FIELD();
                break;
            }
            default:{
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
    }
    
    Row row = {.fields=fields, .fields_count=fields_count, .fields_size=fields_size};
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
            free(row.fields[j]);
        }
        free(row.fields);
    }
    free(csv->rows);
}

char *parse_cli(int argc, char *argv[], bool *same_row_count, bool *trim_whitespace, bool *whitespace_empty){
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

int main(int argc, char *argv[]){
    bool same_row_count = false;
    bool trim_whitespace = false;
    bool whitespace_empty = false;
    char *filename = parse_cli(argc, argv, &same_row_count, &trim_whitespace, &whitespace_empty);
    if (filename == NULL){
        return 1;
    }
    
    Row *rows = malloc(sizeof(Row) * 8);
    Csv csv = {
        .same_row_count=same_row_count,
        .trim_whitespace=trim_whitespace,
        .whitespace_empty=whitespace_empty,
        .max_row_count=0,
        .row_count=0,
        .rows=rows,
        .filename=filename,
    };
    
    FILE *f = fopen(argv[1], "r");
    if (f == NULL){
        free(rows);
        free(filename);
        fprintf(stderr, "unable to open file %s\n", argv[1]);
        return 1;
    }

    int res = 1;
    while(res > 0){
        res = parse_row(f, &csv);
    }
    
    if (res == -1){
        free_csv(&csv);
        fclose(f);
        return 1;
    }
    
    print_csv(&csv);
    free_csv(&csv);
    fclose(f);

    return 0;
}
