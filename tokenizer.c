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

#define STR2(x) #x
#define STR(x) STR2(x)

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
        if (c == ';') {
            do
                c = getc(file);
            while (c != EOF && c != '\n');
            continue;
        }
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
            if (fscanf(file, "%"STR(TOKEN_NNAME)"[^, \n]", tok->name) != 1)
                goto fail;
        } else if (c == '(') {
            tok->type = TOK_OP_INDR;
            if (fscanf(file, "%"STR(TOKEN_NNAME)"[^)]", tok->name) != 1)
                goto fail;
            if ((c = getc(file)) != ')')
                goto fail;
        } else {
            tok->type = TOK_COMMAND;
            char *name = tok->name;
            name[0] = c;
            if (fscanf(file, "%"STR(TOKEN_NNAME)"[^, \n]", name+1) != 1)
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

int TokenizeStr(char *str, Token **ret) {
    FILE *f;
    if ((f = fmemopen(str, strlen(str), "r")) == NULL)
        eprintf("TokenizeStr: fmemopen failed:");
    int i = Tokenize(f, ret);
    fclose(f);
    return i;
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
    case TOK_OP_INDR:
        fprintf(file, " (%s)", tok->name);
        break;
    default:
        fprintf(file, " ?");
        break;
    }
    PrintTokens(tok->next, file);
}

int TokenIsOp(Token *tok) {
    TokenType type = tok->type;
    return type != TOK_LABEL && type != TOK_COMMAND && type != TOK_COMMA;
}
