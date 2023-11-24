#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "eprintf.h"
#include "code.h"

typedef struct List List;
struct List {
    void *head;
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

enum {
    NBUF = 1024,
};

char fmtbuf[NBUF], buf[NBUF];
List *formats = NULL;
CodeStream *code;

static void Process(FILE *stream) {
    for (;;) {
        if (fscanf(stream, fmtbuf, buf) < 0)
            break;
        List *format = NULL;
        for (List *l = formats; l != NULL; l = l->tail) {
            List *f = l->head;
            char *t = f->head;
            if (strcmp(t, buf) == 0) {
                format = f;
                break;
            }
        }
        if (format == NULL) {
            List *cmd = Cons(estrdup(buf), NULL);
            for (;;) {
                if (fscanf(stream, fmtbuf, buf) < 1)
                    break;
                if (strcmp(buf, ";") == 0)
                    break;
                cmd = Cons(estrdup(buf), cmd);
            }
            cmd = Reverse(cmd);
            for (List *l = cmd; l != NULL; l = l->tail) {
                printf("%s ", (char *)l->head);
            }
            printf("\n");
            formats = Cons(cmd, formats);
            continue;
        }
        char *error = NULL;
        List *fh = format->tail;
        for (; fh != NULL && fh->tail != NULL; fh = fh->tail->tail) {
            char *typ = fh->tail->head;
            int ntyp = strlen(typ);
            printf("%s (%s)? ", (char *)fh->head, typ);
            if (typ[ntyp-1] == 'D' || typ[ntyp-1] == 'X') {
                int d;
                if (fscanf(stream, (typ[ntyp-1] == 'D' ? "%d" : "%x"), &d) < 1) {
                    error = "failed reading numeric";
                    goto bail;
                }
                if (CodeWrite(code, typ, d) == -1) {
                    error = "failed writing numeric";
                    goto bail;
                }
                continue;
            }
            if (fscanf(stream, fmtbuf, buf) < 1) {
                error = "failed reading string";
                goto bail;
            }
            if (CodeWrite(code, typ, buf) == -1) {
                error = "failed writing string";
                goto bail;
            }
        }
        if (CodeFlush(code) == -1) {
            error = "number of bits is not a multiple of 4";
            CodeClear(code);
            goto bail;
        }
    bail:
        if (error != NULL)
            weprintf("error: %s", error);
    }
}

int main(int argc, char *argv[]) {
    snprintf(fmtbuf, NBUF, " %%%ds", NBUF);
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
    return EXIT_SUCCESS;
}
