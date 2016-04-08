%{
#include <string>
#include "ast.h"
#include "parser.h"

extern int yylineno;

void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

digs [0-9]+

%%
[ \t]               ; //ignores spaces
{digs}              {yylval.integer = std::atoi(yytext); return T_INT;}
"exit"              {return 0;}
"def"               {return T_DEF;}
[a-z_][a-z0-9_]*    {yylval.identifier = new std::string(yytext); return T_IDENT;}
"+"                 {return T_PLUS;}
","                 {return T_VIRGULA;}
"*"					        {return T_MULTIPLY;}
"="                 {return T_ASSIGNMENT;}
[\n\r]              {return T_NL;}
.                   {yyerror("Unknown token! %c\n", *yytext);} /*complains about every unknown character*/

%%

void yyerror(const char *s, ...){
    va_list ap;
    va_start(ap, s);
    std::fprintf(stderr, "Line %d: error: ", yylineno);
    std::vfprintf(stderr, s, ap);
    exit(0);
}
