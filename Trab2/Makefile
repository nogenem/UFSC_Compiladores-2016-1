# My Little Makefile
OBJ=./obj
SRC=./src
INC=./include
INC2=../Include/
LEX=./flexBison
SYN=./flexBison

FLEX=flex
BISON=bison
CC=clang++
PARAM=-std=gnu++11 -Wno-deprecated-register
LLVMLIB=
LLVMPARAM=
OUTPUT=myparse

all: $(OUTPUT)

$(OUTPUT): $(OBJ)/main.o $(OBJ)/util.o $(OBJ)/ast.o $(OBJ)/calctree.o $(OBJ)/castast.o $(OBJ)/st.o $(OBJ)/parser.o $(OBJ)/scanner.o
	${CC} -o $(OUTPUT) $(OBJ)/*.o -I$(INC) -I$(INC2) ${PARAM} ${LLVMLIB}

$(OBJ)/main.o: main.cpp $(INC)/ast.hpp
	${CC} -c -o $(OBJ)/main.o main.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}

$(OBJ)/util.o: $(SRC)/util.cpp
	${CC} -c -o $(OBJ)/util.o $(SRC)/util.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}
	
$(OBJ)/ast.o: $(SRC)/ast.cpp $(INC)/ast.hpp
	${CC} -c -o $(OBJ)/ast.o $(SRC)/ast.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}
	
$(OBJ)/calctree.o: $(SRC)/calctree.cpp $(INC)/ast.hpp
	${CC} -c -o $(OBJ)/calctree.o $(SRC)/calctree.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}
	
$(OBJ)/castast.o: $(SRC)/castast.cpp $(INC)/ast.hpp
	${CC} -c -o $(OBJ)/castast.o $(SRC)/castast.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}
	
$(OBJ)/st.o: $(SRC)/st.cpp $(INC)/ast.hpp $(INC)/st.hpp
	${CC} -c -o $(OBJ)/st.o $(SRC)/st.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}

$(OBJ)/parser.o: $(SYN)/parser.y $(INC)/ast.hpp $(INC)/st.hpp $(INC)/util.hpp
	${BISON} --defines=$(INC)/parser.hpp --output=$(SRC)/parser.cpp $(SYN)/parser.y
	${CC} -c -o $(OBJ)/parser.o $(SRC)/parser.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}
	
$(OBJ)/scanner.o: $(LEX)/scanner.l $(SYN)/parser.y $(INC)/ast.hpp $(INC)/st.hpp 
	${FLEX} -o $(SRC)/scanner.cpp $(LEX)/scanner.l
	${CC} -c -o $(OBJ)/scanner.o $(SRC)/scanner.cpp -I$(INC) -I$(INC2) ${PARAM} ${LLVMPARAM}

clean:
	rm $(OBJ)/*.o $(SRC)/scanner.cpp $(SRC)/parser.cpp $(INC)/parser.h $(OUTPUT)