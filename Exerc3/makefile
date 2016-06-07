# My Little Makefile
FLEX=flex
BISON=bison
CC=clang++
PARAM=-std=c++11 -fcxx-exceptions -Wno-deprecated-register -O3
LLVM=`llvm-config --cppflags --ldflags --libs core jit native`
OUTPUT=myexerc

all: main

main: exerc3_main.cpp
	${CC} -o myexerc exerc3_main.cpp ${PARAM} ${LLVM}

clean:
	rm myexerc
