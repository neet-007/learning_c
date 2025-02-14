#include <stdio.h>
#include <string.h>

int match_here(char *regexp, char *text);

int match_star_greedy(int c, char *regexp, char *text){
    char *t;
    for (t = text; *t != '\0' && (*t == c || c == '.'); t++);

    do {
        if (match_here(regexp, text)){
            return 1;
        }
    }while (t-- > text);

    return 0;
}

int match_star(int c, char *regexp, char *text){
    do {
        if (match_here(regexp, text)){
            return 1;
        }
    }while (*text != '\0' && (*text++ == c || c == '.'));

    return 0;
}

int match_here(char *regexp, char *text){
    if (regexp[0] == '\0'){
        return 1;
    }
    if (regexp[1] == '*'){
        return match_star(regexp[0], regexp + 2, text);
    }
    if (regexp[0] == '$' && regexp[1] == '\0'){
        return *text == '\0';
    }
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text)){
        return match_here(regexp + 1, text + 1);
    }

    return 0;
}

int match(char *regexp, char *text){
    if (regexp[0] == '^'){
        return match_here(regexp + 1, text);
    }

    do {
        if (match_here(regexp, text)){
            return 1;
        }
    }while (*text++ != '\0');

    return 0;
}

int grep(char *regexp, FILE *stream, char *name){
    int n, n_match;
    char buf[BUFSIZ];

    n_match = 0;

    while (fgets(buf, sizeof(buf), stream)) {
        n = strlen(buf);

        if (n > 0 && buf[n - 1] == '\n'){
            buf[n - 1] = '\0';
        }

        if (match(regexp, buf)){
            n_match++;
            if (name){
                printf("%s:", name);
            }
            printf("%s\n", buf);
        }
    }

    return n_match;
}

int main(int argc, char *argv[]){
    int i, n_match;
    FILE *f;

    if (argc < 2){
        fprintf(stderr, "usage: grep rexgep [file ...]\n");
        return 1;
    }

    if (argc == 2){
        if (grep(argv[1], stdin, NULL)){
            n_match++;
        }
    }else{
        for (i = 2; i < argc; i++){
            f = fopen(argv[i], "r");
            if (f == NULL){
                fprintf(stderr, "unable to open file %s\n", argv[i]);
                continue;
            }

            if (grep(argv[1], f, argc > 3 ? argv[i] : NULL)){
                n_match++;
            }
            fclose(f);
        }
    }


    return n_match == 0;
}
