#include "include/ast.hpp"

extern AST::Block* programRoot; //set on Bison file
extern int yyparse();
extern int yydebug;

int main(int argc, char **argv){
  //yydebug = 1;
  yyparse();                  //parses whole data
  if(programRoot != nullptr)
	  programRoot->printTree();
  return 0;
}
