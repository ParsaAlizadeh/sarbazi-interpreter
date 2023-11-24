#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "eprintf.h"
#include "code.h"

typedef struct List List;
struct List {
    void *head; /* mallocated string */
    List *tail;
};

static List *Cons(void *head, List* tail) {
    List *l = malloc(sizeof(List));
    l->head = head;
    l->tail = tail;
    return l;
}

static List *Reverse(List *l) {
    List *prv = NULL;
    List *nxt;
    for (; l != NULL; l = nxt) {
        nxt = l->tail;
        l->tail = prv;
        prv = l;
    }
    return prv;
}

static void FreeList(void *p) {
    if (p == NULL)
        return;
    FreeList(((List *)p)->tail);
    free(p);
}

static size_t Length(List *l) {
    if (l == NULL)
        return 0;
    return 1 + Length(l->tail);
}

static List *GetWords(char *line) {
    List *l = NULL;
    char *p, *tok;
    for (p = line; (tok = strtok(p, " \n\t")) != NULL; p = NULL) {
        l = Cons(estrdup(tok), l);
    }
    return Reverse(l);
}

char *line = NULL;
size_t n = 0;
List *formats = NULL;
CodeStream *code;

static void Process(FILE *stream) {
    for (;;) {
        errno = 0;
        if (getline(&line, &n, stream) == -1) {
            if (errno != 0)
                eprintf("Exit:");
            break;
        }
        List *cmd = GetWords(line);
        size_t ncmd = Length(cmd);
        if (cmd == NULL)
            continue;
        List *f = NULL;
        for (List *l = formats; l != NULL; l = l->tail) {
            if (strcmp(((List *)l->head)->head, cmd->head) == 0 && Length(l->head) == ncmd) {
                f = l->head;
                break;
            }
        }
        if (f == NULL) {
            formats = Cons(cmd, formats);
            for (List *l = cmd; l != NULL; l = l->tail) {
                printf("%s", (char *)l->head);
                if (l->tail != NULL)
                    printf(" ");
            }
            printf("\n");
            continue;
        }
        char *error = NULL;
        List *fmi = f->tail;
        List *cmi = cmd->tail;
        for (; fmi != NULL && cmi != NULL; fmi = fmi->tail, cmi = cmi->tail) {
            char *fmih = fmi->head;
            char *cmih = cmi->head;
            int nfmih = strlen(fmih);
            if (fmih[nfmih-1] == 'D' || fmih[nfmih-1] == 'X') {
                int d;
                if (sscanf(cmih, (fmih[nfmih-1] == 'X' ? "%x" :"%d"), &d) < 1) {
                    error = "Failed reading numeric";
                    goto bail;
                }
                if (CodeWrite(code, fmih, d) == -1) {
                    error = "Failed writing numeric";
                    goto bail;
                }
                continue;
            }
            if (CodeWrite(code, fmih, cmih) == -1) {
                error = "Failed writing misc";
                goto bail;
            }
        }
        if (CodeFlush(code) == -1) {
            error = "Line is not a multiple of 4";
            CodeClear(code);
            goto bail;
        }
    bail:
        if (error != NULL)
            weprintf("Error: %s", error);
        for (List *l = cmd; l != NULL; l = l->tail)
            free(l->head);
        FreeList(cmd);
    }
}

int main(int argc, char *argv[]) {
    code = NewCode(stdout);
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            FILE *f = fopen(argv[i], "r");
            if (f == NULL) {
                weprintf("Failed openning file %s, ignoring:", argv[i]);
                continue;
            }
            Process(f);
            fclose(f);
        }
    }
    Process(stdin);
    for (List *i = formats; i != NULL; i = i->tail) {
        for (List *j = i->head; j != NULL; j = j->tail)
            free(j->head);
        FreeList(i->head);
    }
    FreeCode(code);
    FreeList(formats);
    free(line);
    return EXIT_SUCCESS;
}
