# Sarbazi Interpreter

Assembly interpreter and machine code generator, for custom assembly
languages

## `main.c`: Interpreter

Define a custom language in `main.c`. Build with `make main`.

## `repl.c`: REPL

Define machine code formats and translate to hex code interactively. Build
with `make repl`. You can prepare a format file like `ibm360.txt`, and pass
it as argument: `./repl ./ibm360.txt`.

## `utf8-to-ebcdic.sh`: UTF-8 to EBCDIC Converter

Read from input one line at a time and convert it into ebcdic hex code.
