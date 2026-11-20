#!/bin/bash
if [[ "$1" == "64" ]]; then
    ./bin/wingc -i test/src/test64.w -o test/bin/test64.a
    nasm -f elf64 test/bin/test64.a -o test/obj/test64.o
    x86_64-linux-gnu-ld test/obj/test64.o -o test/bin/test64
    ./test/bin/test64
fi

if [[ "$1" == "32" ]]; then
    ./bin/wingc -i test/src/test32.w -o test/bin/test32.a
    nasm -f elf32 test/bin/test32.a -o test/obj/test32.o
    x86_64-linux-gnu-ld -m elf_i386 test/obj/test32.o -o test/bin/test32
    ./test/bin/test32
fi