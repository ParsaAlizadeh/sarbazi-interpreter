#include "tokenizer.h"

#include <ctype.h>
#include <string.h>
#include "eprintf.h"

Token *NewToken(void) {
    Token *tok = emalloc(sizeof(Token));
    tok->next = NULL;
    bzero(tok->name, sizeof(tok->name));
    return tok;
}

void FreeToken(Token *tok) {
    if (tok == NULL)
        return;
    FreeToken(tok->next);
    free(tok);
}

int Tokenize(FILE *file, Token **ret) {
    Token **final = ret;
    Token *tok;
    int c;
    for (;;) {
        c = getc(file);
        if (c == EOF)
            break;
        if (isspace(c))
            continue;
        tok = NewToken();
        *ret = tok;
        if (c == ',') {
            tok->type = TOK_COMMA;
        } else if (c == 'R') {
            int nreg;
            if (fscanf(file, "%d", &nreg) != 1)
                goto fail;
            tok->type = TOK_OP_REG;
            tok->num = nreg;
        } else if (c == '#') {
            int data;
            if (fscanf(file, "%d", &data) != 1)
                goto fail;
            tok->type = TOK_OP_DATA;
            tok->num = data;
        } else if (c == '@') {
            tok->type = TOK_OP_ADDR;
            if (fscanf(file, "%12[^, \n]", tok->name) != 1)
                goto fail;
        } else {
            tok->type = TOK_COMMAND;
            char *name = tok->name;
            name[0] = c;
            if (fscanf(file, "%11[^, \n]", name+1) != 1)
                goto fail;
            int n = strlen(name);
            if (name[n-1] == ':') {
                tok->type = TOK_LABEL;
                name[--n] = '\0';
            }
            if (n == 0)
                goto fail;
        }
        ret = &tok->next;
    }
    return 0;
fail:
    FreeToken(*final);
    return -1;
}

void PrintTokens(Token *tok, FILE *file) {
    if (tok == NULL)
        return;
    switch (tok->type) {
    case TOK_COMMA:
        fprintf(file, " ,");
        break;
    case TOK_COMMAND:
        fprintf(file, "\n%s", tok->name);
        break;
    case TOK_LABEL:
        fprintf(file, "\n%s:", tok->name);
        break;
    case TOK_OP_ADDR:
        fprintf(file, " @%s", tok->name);
        break;
    case TOK_OP_DATA:
        fprintf(file, " #%d", tok->num);
        break;
    case TOK_OP_REG:
        fprintf(file, " R%d", tok->num);
        break;
    }
    PrintTokens(tok->next, file);
}
