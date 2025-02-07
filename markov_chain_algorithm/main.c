#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {
    NPREF = 2,
    NHASH = 4093,
    MAXGEN = 10000,
    MULTIPLIER = 37,
};

char NONWORD[] = "\n";

typedef struct State State;
typedef struct Suffix Suffix;
struct State{
    char *pref[NPREF];
    Suffix *suf;
    State *next;
};

struct Suffix{
    char *word;
    Suffix *next;
};

State *statetab[NHASH];

unsigned int hash(char *s[NPREF]){
    unsigned int h;
    unsigned char *p;
    h = 0;
    for (size_t i = 0; i < NPREF; i++){
        for (p = (unsigned char *)s[i]; *p != '\0'; p++){
            h = MULTIPLIER * h + (*p);
        }
    }

    return h % NHASH;
}

/* lookup: search f o r prefix; create i f requested. */
/* returns pointer i f present or created; NULL i f not. */
/* creation doesn't strdup so strings mustn't change later*/
State* lookup(char *prefix[NPREF] , int create){
    int i, h ;
    h = hash(prefix);

    State *sp;
    for (sp = statetab[h]; sp != NULL; sp = sp->next){
        for (i = 0; i < NPREF; i++){
            if (strcmp(prefix[i], sp->pref[i]) != 0){
                break;
            }
        }
        if (i == NPREF){
            return sp;
        }
    }

    if (create){
        sp = (State *) malloc(sizeof(State));
        if (sp == NULL){
            fprintf(stderr, "unable to allocate memroy for sp in lookup\n");
            return NULL;
        }
        for (i = 0; i < NPREF; i++){
            sp->pref[i] = prefix[i];
        }
        sp->suf = NULL;
        sp->next = statetab[h];
        statetab[h] = sp;
    }

    return sp;
}

/* addsuffix: add tostate . suffix must not change later */
void add_suffix(State *sp, char *suffix){
    Suffix *suf;
    suf = (Suffix *)malloc(sizeof(Suffix));
    if (suf == NULL){
        fprintf(stderr, "unable to allocate memory for suf in add_suffix\n");
        return;
    }
    suf->word = suffix;
    suf->next = sp->suf;
    sp->suf = suf;
}

/* add: add word to suffix list , update prefix */
void add(char *prefix[NPREF], char *suffix){
    State *sp;
    sp = lookup(prefix, 1);
    add_suffix(sp, suffix);
    memmove(prefix, prefix + 1, (NPREF - 1)*sizeof(prefix[0]));
    prefix[NPREF - 1] = suffix;
}

/* build: read input, build prefix table a*/
void build(char *prefix[NPREF] , FILE *f){
    char *temp = NULL;
    char buf[100], fmt[10];
    sprintf(fmt, "%%%ds", sizeof(buf)-1);
    while(fscanf(f, fmt, buf) != EOF){
        temp = strdup(buf);
        if (temp == NULL){
            fprintf(stderr, "unable to dup string in build\n");
            return;
        }
        add(prefix, temp);
    }
}

/* generate: produce output, one word per line */
void generate(int nwords){
    State *sp;
    Suffix *suf;
    char *prefix[NPREF], *w;
    int i, nmatch;

    for (i = 0; i < NPREF; i++){
        prefix[i] = NONWORD;
    }

    for (i = 0; i < nwords; i++){
        sp = lookup(prefix, 0);

        nmatch = 0;
        for (suf = sp->suf; suf != NULL; suf = suf->next){
            if ((rand() % ++nmatch) == 0){
                w = suf->word;
            }
        }

        if (strcmp(w, NONWORD) == 0){
            break;
        }

        printf("%s\n", w);
        memmove(prefix, prefix + 1, (NPREF - 1) * sizeof(prefix[0]));
        prefix[NPREF - 1] = w;
    }
}

/* markov main: markov-chain random text generation */
int main (void){
    int i, nwords;
    nwords = MAXGEN;
    char *prefix[NPREF];
    for (i = 0; i < NPREF; i++){
        prefix[i] = NONWORD;
    }
    printf("Starting build...\n");
    build(prefix, stdin);
    printf("Starting add...\n");
    add(prefix, NONWORD);
    printf("Starting generate...\n");
    generate(nwords);

    return 0;
}
