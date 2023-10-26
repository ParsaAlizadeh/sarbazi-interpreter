#ifndef STRUCTURE_H
#define STRUCTURE_H

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

extern SymTab *NewSymTab(SymTab *next);
extern Program *CreateProgram(Token *tok);
extern char *InstructMnem(Instruct *);
extern Token *TokenNextOperand(Token *);
extern int InstructNOp(Instruct *);
extern Token *InstructOp(Instruct *, int n);
extern Token *InstructOpType(Instruct *, int n, TokenType type);
extern Token *InstructOpAddr(Instruct *, int n);
extern Token *InstructOpReg(Instruct *, int n);
extern Token *InstructOpData(Instruct *, int n);
extern int InstructMatchToken(Instruct *, Token *);
extern int InstructMatch(Instruct *, char *str);
extern int InstructMatchN(Instruct *ins, ...);
extern void PrintProgram(Program *, FILE *);
extern void FreeSymTab(SymTab *);
extern void FreeInstruct(Instruct *);
extern void FreeProgram(Program *);

#endif
