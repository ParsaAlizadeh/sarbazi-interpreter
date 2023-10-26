#include "structure.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "eprintf.h"

SymTab *NewSymTab(SymTab *next) {
    SymTab *s = emalloc(sizeof(SymTab));
    s->next = next;
    return s;
}

Program *CreateProgram(Token *tok) {
    Program *p = emalloc(sizeof(Program));
    memset(p, 0, sizeof(*p));
    Instruct **next = &p->ins;
    for (; tok != NULL; tok = tok->next) {
        if (tok->type == TOK_LABEL) {
            p->symtab = NewSymTab(p->symtab);
            p->symtab->label = tok->name;
            p->symtab->insp = next;
        } else if (tok->type == TOK_COMMAND) {
            Instruct *ins = emalloc(sizeof(Instruct));
            ins->token = tok;
            ins->next = NULL;
            *next = ins;
            next = &ins->next;
            while (tok->next != NULL && tok->next->type != TOK_LABEL && tok->next->type != TOK_COMMAND)
                tok = tok->next;
        } else {
            eprintf("Bad token");
        }
    }
    return p;
}

char *InstructMnem(Instruct *ins) {
    return ins->token->name;
}

Token *TokenNextOperand(Token *tok) {
    if (tok == NULL)
        return NULL;
    tok = tok->next;
    while (tok != NULL && tok->type == TOK_COMMA)
        tok = tok->next;
    if (tok == NULL)
        return NULL;
    if (tok->type == TOK_LABEL || tok->type == TOK_COMMAND)
        return NULL;
    return tok;
}

int InstructNOp(Instruct *ins) {
    int n = 0;
    Token *tok = ins->token;
    while (tok != NULL) {
        tok = TokenNextOperand(tok);
        n++; /* also counts last NULL */
    }
    return n-1;
}

Token *InstructOp(Instruct *ins, int n) {
    Token *tok = ins->token;
    for (; n >= 0; n--) {
        tok = TokenNextOperand(tok);
    }
    return tok;
}

Token *InstructOpType(Instruct *ins, int n, TokenType type) {
    Token *tok = ins->token;
    for (; n >= 0; n--) {
        do
            tok = TokenNextOperand(tok);
        while (tok != NULL && tok->type != type);
    }
    return tok;
}

Token *InstructOpAddr(Instruct *ins, int n) {
    return InstructOpType(ins, n, TOK_OP_ADDR);
}

Token *InstructOpReg(Instruct *ins, int n) {
    return InstructOpType(ins, n, TOK_OP_REG);
}

Token *InstructOpData(Instruct *ins, int n) {
    return InstructOpType(ins, n, TOK_OP_DATA);
}

int InstructMatchToken(Instruct *ins, Token *tok) {
    Token *a = ins->token;
    if (tok == NULL)
        return 0;
    if (tok->type != TOK_COMMAND)
        return 0;
    if (strncmp(a->name, tok->name, sizeof(tok->name)) != 0)
        return 0;
    do {
        if (tok == NULL)
            return 0;
        if (a->type != tok->type)
            return 0;
        a = TokenNextOperand(a);
        tok = TokenNextOperand(tok);
    } while (a != NULL);
    if (tok != NULL)
        return 0;
    return 1;
}

int InstructMatch(Instruct *ins, char *str) {
    Token *tok = NULL;
    if (TokenizeStr(str, &tok) < 0) {
        weprintf("InstructMatch: invalid string: %s", str);
        return 0;
    }
    int i = InstructMatchToken(ins, tok);
    FreeToken(tok);
    return i;
}

int InstructMatchN(Instruct *ins, ...) {
    va_list ap;
    va_start(ap, ins);
    int n = -1, i = 0;
    for (;;) {
        char *str = va_arg(ap, char *);
        if (str == NULL)
            break;
        if (n == -1 && InstructMatch(ins, str))
            n = i;
        i++;
    }
    va_end(ap);
    return n;
}

void PrintProgram(Program *p, FILE *f) {
    for (Instruct *ins = p->ins; ins != NULL; ins = ins->next) {
        int n = InstructNOp(ins);
        fprintf(f, "%s/%d ", InstructMnem(ins), n);
        for (int i = 0; i < n; i++) {
            if (i > 0)
                fprintf(f, ", ");
            Token *tok = InstructOp(ins, i);
            switch (tok->type) {
            case TOK_OP_REG:
                fprintf(f, "R%d", tok->num);
                break;
            case TOK_OP_ADDR:
                fprintf(f, "@%s", tok->name);
                break;
            case TOK_OP_DATA:
                fprintf(f, "#%d", tok->num);
                break;
            default:
                fprintf(f, "?");
            }
        }
        fprintf(f, "\n");
    }
}

void FreeSymTab(SymTab *s) {
    if (s == NULL)
        return;
    FreeSymTab(s->next);
    free(s);
}

void FreeInstruct(Instruct *ins) {
    if (ins == NULL)
        return;
    FreeInstruct(ins->next);
    free(ins);
}

void FreeProgram(Program *p) {
    if (p == NULL)
        return;
    FreeSymTab(p->symtab);
    FreeInstruct(p->ins);
    free(p);
}
