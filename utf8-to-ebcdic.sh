#!/bin/bash

while read line; do
    echo -n "$line" | iconv -f UTF-8 -t EBCDIC-IT | xxd -E
done
