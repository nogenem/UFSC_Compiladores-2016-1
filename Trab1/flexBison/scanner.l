%{
#include <string>
#include "ast.hpp"
#include "parser.h"
extern int yylineno;
void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

%%

[ \t\n\r]                       ; //ignores spaces and new lines
 /* int value/type */
[0-9]+                          {yylval.int_v = std::atoi(yytext); return INT_V;}
"int"                           {return INT_T;}
 /* real value/type */
[0-9]+\.[0-9]* |
[0-9]*\.[0-9]+                  {yylval.real_v = std::atof(yytext); return REAL_V;}
"real"                          {return REAL_T;}
 /* bool value/type */
"TRUE"                          {yylval.bool_v = true; return BOOL_V;}
"FALSE"                         {yylval.bool_v = false; return BOOL_V;}
"bool"                          {return BOOL_T;}
 /* exit program */
"exit"                          {return 0;}
 /* single character */
"+" |
"-" |
"*" |
"/" |
"," |
";" |
"(" |
")" |
":"                             {return yytext[0];}
 /* relational opts */
"="                             {return EQ_OPT;}
"~="                            {return NEQ_OPT;}
">"                             {return GRT_OPT;}
"<"                             {return LST_OPT;}
">="                            {return GRTEQ_OPT;}
"<="                            {return LSTEQ_OPT;}
 /* bool opts */
"AND"                           {return AND_OPT;}
"OR"                            {return OR_OPT;}
"~"                             {return NOT_OPT;}
 /* assignment */
":="                            {return ASSIGN_OPT;}
 /* return */
"return"                        {return RETURN_T;}
 /* ids */
[a-z]+                          {yylval.name = strdup(yytext); return ID_V;}
 /* any other thing */
.                               {yyerror("Unknown token! %c\n", *yytext);} /*complains about every unknown character*/

%%

void yyerror(const char *s, ...){
    va_list ap;
    va_start(ap, s);
    std::fprintf(stderr, "Line %d: error: ", yylineno);
    std::vfprintf(stderr, s, ap);
}
