#include "code.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "eprintf.h"

CodeStream *NewCode(FILE *out) {
    CodeStream *cs = emalloc(sizeof(CodeStream));
    memset(cs, 0, sizeof(CodeStream));
    cs->out = out;
    return cs;
}

void FreeCode(CodeStream *cs) {
    if (cs == NULL)
        return;
    free(cs);
}

void CodeInitLine(CodeStream *cs) {
    if (cs->line == NULL) {
        memset(cs->buf, 0, sizeof(cs->buf));
        cs->line = fmemopen(cs->buf, sizeof(cs->buf)-1, "w");
    }
}

int CodeWriteBits(CodeStream *cs, char *str) {
    CodeInitLine(cs);
    for (char *s = str; *s != '\0'; s++)
        if (*s != '0' && *s != '1' && *s != '?')
            return -1;
    fprintf(cs->line, "%s", str);
    return 0;
}

int CodeWriteInt(CodeStream *cs, int x, int nbit) {
    for (int i = nbit-1; i >= 0; i--) {
        if (CodeWriteBits(cs, x & (1 << i) ? "1" : "0") < 0)
            return -1;
    }
    return 0;
}

int CodeWrite(CodeStream *cs, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *t;
    int x, nbit = 0;
    for (char *s = fmt; *s != '\0'; s++) {
        if (isdigit(*s)) {
            nbit = 10 * nbit + (*s - '0');
            continue;
        }
        switch (*s) {
        case '%':
            t = va_arg(ap, char *);
            if (nbit == 0)
                nbit = 1;
            for (int i = 0; i < nbit; i++)
                if (CodeWriteBits(cs, t) < 0)
                    goto fail;
            break;
        case 'D':
            x = va_arg(ap, int);
            if (nbit == 0)
                nbit = va_arg(ap, int);
            if (CodeWriteInt(cs, x, nbit) < 0)
                goto fail;
            break;
        default:
            goto fail;
        }
        nbit = 0;
    }
    va_end(ap);
    return 0;
 fail:
    va_end(ap);
    return -1;
}

int CodeFlush(CodeStream *cs) {
    fflush(cs->line);
    int n = strlen(cs->buf);
    if (n % 4 != 0)
        return -1;
    for (int i = 0; i < n; i += 4) {
        if (memchr(cs->buf+i, '?', 4) != NULL) {
            fprintf(cs->out, "?");
            continue;
        }
        int r = 0;
        for (int j = 0; j < 4; j++)
            if (cs->buf[i+3-j] == '1')
                r += (1 << j);
        if (r < 10)
            fprintf(cs->out, "%d", r);
        else
            fprintf(cs->out, "%c", 'A' + r - 10);
    }
    fprintf(cs->out, "\n");
    CodeClear(cs);
    return 0;
}

void CodeClear(CodeStream *cs) {
    fclose(cs->line);
    cs->line = NULL;
}
