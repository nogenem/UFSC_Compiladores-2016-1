# My Little Makefile
FLEX=flex
FLEXFILE=flexBison/scanner.l

BISON=bison
BISONFILE=flexBison/parser.y

CC=clang++
PARAM=-std=gnu++11 -Wno-deprecated-register -Iinclude/ -I../Include/ -Isrc/
OUTPUT=myparse

all: flex yacc clang

clang: src/*.cpp include/*.h include/*.hpp
	${CC} -o ${OUTPUT} ${PARAM} src/*.cpp *.cpp ${LLVMPARAM}

yacc: ${BISONFILE}
	${BISON} --defines=include/parser.h --output=src/parser.cpp ${BISONFILE}

flex: ${FLEXFILE}
	${FLEX} -o src/scanner.cpp ${FLEXFILE}

clean:
	rm src/parser.cpp include/parser.h src/scanner.cpp ${OUTPUT}

test: all
	./${OUTPUT} < input_example
