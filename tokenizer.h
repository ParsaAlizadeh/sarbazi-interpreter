#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

typedef enum TokenType TokenType;
enum TokenType {
    TOK_LABEL,      // label:
    TOK_COMMAND,    // mov
    TOK_OP_REG,     // R12
    TOK_OP_ADDR,    // @label
    TOK_OP_DATA,    // #124
    TOK_COMMA       // ,
};

typedef struct Token Token;
struct Token {
    TokenType type;
    int num;
    char name[13];
    Token *next;
};

extern Token *NewToken(void);
extern void FreeToken(Token *);
extern int Tokenize(FILE *, Token **ret);
extern void PrintTokens(Token *, FILE *);

#endif
