VMAJ = 0
VMIN = 1

C = gcc
CFLAGS = -Iinclude -DWINGC_VERSION=\"v${VMAJ}.${VMIN}\"
SRC = src/main.c src/include/lexer.c src/include/toasm.c src/include/linked.c src/include/parse.c src/include/fpemdas.c src/include/nametable.c
OUT = bin/wingc

all:
	mkdir -p bin
	${C} ${CFLAGS} ${SRC} -o ${OUT}