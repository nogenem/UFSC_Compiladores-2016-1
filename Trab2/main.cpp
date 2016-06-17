#include <string>

extern int yyparse();
extern int yydebug;

int main(int argc, char **argv){
  if(argc > 1){
	  if(std::string(argv[1]) == "--debug")
		  yydebug = 1;
  }
  yyparse();                  //parses whole data
  return 0;
}
