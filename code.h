#ifndef CODE_H
#define CODE_H

#include <stdio.h>

typedef struct CodeStream CodeStream;
struct CodeStream {
    FILE *out;
    FILE *line;
    char buf[257];
};

extern CodeStream *NewCode(FILE *out);
extern void FreeCode(CodeStream *);
extern void CodeInitLine(CodeStream *);
extern int CodeWriteBits(CodeStream *, char *str);
extern int CodeWriteInt(CodeStream *, int, int nbit);
extern int CodeWrite(CodeStream *, char *fmt, ...);
extern int CodeFlush(CodeStream *);
extern void CodeClear(CodeStream *);

#endif
