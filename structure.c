#include <stdio.h>
#include <string.h>
#include "eprintf.h"
#include "tokenizer.h"

typedef struct Instruct Instruct;
struct Instruct {
    Token *token;
    Instruct *next;
};

typedef struct SymTab SymTab;
struct SymTab {
    char *label;
    Instruct **insp;
    SymTab *next;
};

typedef struct Program Program;
struct Program {
    Instruct *ins;
    SymTab *symtab;
};

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

int InstructNOp(Instruct *ins) {
    Token *tok = ins->token->next;
    int n = 0;
    while (tok != NULL && tok->type != TOK_LABEL && tok->type != TOK_COMMAND) {
        n += (tok->type != TOK_COMMA);
        tok = tok->next;
    }
    return n;
}

Token *InstructOp(Instruct *ins, int n) {
    Token *tok = ins->token->next;
    while (tok != NULL && tok->type != TOK_LABEL && tok->type != TOK_COMMAND) {
        n -= (tok->type != TOK_COMMA);
        if (n < 0)
            return tok;
        tok = tok->next;
    }
    return NULL;
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

int main() {
    Token *tok = NULL;
    if (Tokenize(stdin, &tok) == -1)
        eprintf("tokenizer failed");
    Program *p = CreateProgram(tok);
    if (p == NULL)
        eprintf("create program failed");
    PrintProgram(p, stdout);
    return EXIT_SUCCESS;
}

