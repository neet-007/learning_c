#include <stdio.h>
#include <string.h>
char buf[200] ; /* input line buffer */
char *field[20] ; /* fie1ds */

/* unquote: remove leading and trailing quote */
char *unquote(char *p){
    if (p[0] == '"'){
        if (p[strlen(p) - 1] == '"'){
            p[strlen(p) - 1] = '\0';
        }
        p++;
    }

    return p;
}

/* csvgetline: read and parse lin e , return field count */
/* sample input : "LU",86.25,"11/4/1998","2:19PM",+4.0625 */
int csv_get_line(FILE * fin){
    if (fgets(buf, sizeof(buf) - 1, fin) == NULL){
        return -1;
    }

    int num_fields = 0;
    char *p, *q;
    for (q = buf; (p=strtok(q, ",\n\r")) != NULL; q = NULL){
        field[num_fields++] = unquote(p);
    }

    return num_fields;
}

int main(void){
    int i, nf;

    while ((nf = csv_get_line(stdin)) != -1) {
        for (i = 0; i < nf; i++){
            printf("field[%d]= '%s'\n", i, field[i]);
        }
    }

    return 0;
}
