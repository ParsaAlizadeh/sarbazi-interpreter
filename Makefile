main: eprintf.c tokenizer.c structure.c code.c main.c
	cc -g eprintf.c tokenizer.c structure.c code.c main.c -o main
repl: eprintf.c code.c repl.c
	cc -g eprintf.c code.c repl.c -o repl
