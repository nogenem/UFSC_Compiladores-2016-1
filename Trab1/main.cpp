#include <iostream>

extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();                  //parses whole data
    return 0;
}
