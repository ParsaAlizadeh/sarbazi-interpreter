#include <stdio.h>
#include "eprintf.h"
#include "structure.h"
#include "code.h"

int main() {
    Token *tok = NULL;
    if (Tokenize(stdin, &tok) == -1)
        eprintf("tokenizer failed");
    Program *p = CreateProgram(tok);
    if (p == NULL)
        eprintf("create program failed");
    PrintProgram(p, stdout);
    CodeStream *cs = NewCode(stdout);
    for (Instruct *ins = p->ins; ins != NULL; ins = ins->next) {
        int opcode;
        opcode = InstructMatchN(ins,
                                "mov R1, R2",
                                "add R1, R2",
                                "sub R1, R2",
                                "and R1, R2",
                                "or R1, R2",
                                "xor R1, R2",
                                (char *)NULL);
        if (opcode != -1) {
            CodeWrite(cs, "4D6D6D", opcode, InstructOp(ins, 0)->num, InstructOp(ins, 1)->num);
            CodeFlush(cs);
            continue;
        }
        opcode = InstructMatchN(ins,
                                "mova R1, @a, R2",
                                "mova @a, R2, R1",
                                "mov R1, R2, @a",
                                "mov @a, R1, R2",
                                "je R1, R2, @a",
                                "jne R1, R2, @a",
                                "jl R1, R2, @a",
                                "jh R1, R2, @a",
                                "call R1, @a, R2",
                                (char *)NULL);
        if (opcode != -1) {
            CodeWrite(cs, "4D6D6D16%",
                      opcode+6,
                      InstructOpReg(ins, 0)->num,
                      InstructOpReg(ins, 1)->num,
                      "?");
            CodeFlush(cs);
            continue;
        }
        opcode = InstructMatchN(ins,
                                "mov R1, #1",
                                "add R1, #1",
                                "sub R1, #1",
                                "add R1, #1",
                                "or R1, #1",
                                "xor R1, #1",
                                (char *)NULL);
        if (opcode != -1) {
            CodeWrite(cs, "4%6D6D16D",
                      "1",
                      opcode,
                      InstructOpReg(ins, 0)->num,
                      InstructOpData(ins, 0)->num);
            CodeFlush(cs);
            continue;
        }
    }
    FreeCode(cs);
    FreeProgram(p);
    return EXIT_SUCCESS;
}
