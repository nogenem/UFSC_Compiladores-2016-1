# My Little Makefile
FLEX=flex
BISON=bison
CC=clang++
PARAM=-std=gnu++11 -Wno-deprecated-register -Iinclude/ -I../Include/ -Isrc/
OUTPUT=myparse

all: flex yacc clang

clang: *.cpp *.h
	${CC} -o ${OUTPUT} *.cpp ${LLVMPARAM} ${PARAM}

yacc: parser.y
	${BISON} --defines=parser.h --output=parser.cpp parser.y

flex: scanner.l
	${FLEX} -o scanner.cpp scanner.l

clean:
	rm parser.cpp parser.h scanner.cpp ${OUTPUT}

test: all
	./${OUTPUT} < input_example
