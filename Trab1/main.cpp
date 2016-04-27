#include <iostream>
#include "ast.hpp"

extern AST::Block* programRoot; //set on Bison file
extern int yyparse();
extern int yydebug;

int main(int argc, char **argv)
{
    yyparse();                  //parses whole data
    programRoot->printTree();
    return 0;
}
