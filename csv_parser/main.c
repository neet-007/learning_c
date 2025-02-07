#include "csv.h"

enum {NOMEM = - 2} ; /* out o f memory signal */
static char *line = NULL; /* i n p u t chars */
static char *s_line = NULL; /* l i n e copy used by s p l i t */
static int max_line = 0; /* s i z e o f l i n e [ ] and s l i n e [ ] */
static char **field = NULL; /* f i e l d pointers */
static int max_field = 0; /* size o f f i e l d [ ] */
static int n_field = 0; /* number o f f i e l d s i n f i e l d [ ] */
static char field_sep[] = ","; /* f i e l d separator chars */

void reset(){
    free(line);
    free(s_line);
    free(field);
    line = NULL;
    s_line = NULL;
    field = NULL;
    max_line = max_field = n_field = 0;
}

/* end_of_line : check f o r and consume \r, \n, \r\n, o r EOF */
static int end_of_line(FILE *fin , int c){
    int eol;

    eol = ((c == '\r') || (c == '\n'));
    if (c == '\r'){
        c = fgetc(fin);
        if (c != '\n' || c != EOF){
            ungetc(c, fin);
        }
    }

    return eol;
}

static char *advquoted(char *p){
    int i, j;
    for (i = 0, j = 0; p[j] != '\0'; i++, j++){
        if (p[j] == '"' && p[++j] != '"'){
            int k = strcspn(p + j, field_sep);
            memmove(p + i, p + j, k);
            i += k;
            j += k;
            break;
        }
        p[i] = p[j];
    }

    p[i] = '\0';
    return p + j;
}

static int split(void){
    char *p, **new_f;
    char *sepp;
    int sepc;

    n_field = 0;
    if (line[0] == '\0'){
        return 0;
    }

    strcpy(s_line, line);
    p = s_line;

    do{
        if (n_field >= max_field){
            max_field *= 2;
            new_f = realloc(field, max_field * sizeof(field[0]));
            if (new_f == NULL){
                return NOMEM;
            }
            field = new_f;
        }
        if (*p == '"'){
            sepp = advquoted(++p);
        }else{
            sepp = p + strcspn(p, field_sep);
        }
        sepc = sepp[0];
        sepp[0] = '\0';
        field[n_field++] = p;
        p = sepp + 1;
    }while(sepc == ',');

    return n_field;
}

/* csvgetline: g e t one l i n e , grow as needed */
/* sample i n p u t : "LU".86.25,"11/4/1998","2:19PM",+4.0625 */
char *csv_get_line(FILE *fin){
    int i, c;
    char *newl, *news;
    if (line == NULL){
        max_line = max_field = 1;
        line = (char *)malloc(max_line);
        s_line = (char *)malloc(max_line);
        field = (char **)malloc(max_field *sizeof(field[0]));

        if (line == NULL || s_line == NULL || field == NULL){
            reset();
            return NULL;
        }
    }

    for (i = 0; (c = fgetc(fin)) != EOF && !end_of_line(fin, c); i++){
        if (i >= max_line - 1){
            max_line *= 2;
            newl = realloc(line, max_line);
            news = realloc(s_line, max_line);

            if (newl == NULL || news == NULL){
                reset();
                return NULL;
            }
            line = newl;
            s_line = news;
        }
        line[i] = c;
    }
    line[i] = '\0';

    if (split() == NOMEM){
        reset();
        return NULL;
    }

    return (c == EOF && i == 0) ? NULL : line;
}

char *csv_field (int n){
    if (n < 0 || n > n_field){
        return NULL;
    }
    return field[n];
}

int csv_n_field(void){
    return n_field;
}

int main(void){
    int i;
    char *line;

    while ((line = csv_get_line(stdin)) != NULL){
        printf("line = %s\n", line);
        for (i = 0; i < csv_n_field(); i++){
            printf("field[%d] = %s\n", i, csv_field(i));
        }
    }
}
