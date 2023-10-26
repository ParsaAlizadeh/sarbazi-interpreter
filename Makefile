a.out: eprintf.c tokenizer.c structure.c code.c main.c
	cc -g eprintf.c tokenizer.c structure.c code.c main.c -o a.out
