#!/bin/bash
nasm -f elf64 test/bin/test_c.a -o test/obj/test_c.o
x86_64-linux-gnu-ld test/obj/test_c.o -o test/bin/test_c