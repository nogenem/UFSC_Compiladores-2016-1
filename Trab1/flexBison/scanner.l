%{
#include "parser.h"
extern int yylineno;
void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

%%

[ \t\n\r]                       ; //ignores spaces and new lines
 /* int value/type */
[0-9]+                          {/*yylval. = std::atoi(yytext);*/ return INT_V;}
"int"                           {return INT_T;}
 /* real value/type */
[0-9]+\.[0-9]* |
[0-9]*\.[0-9]+                  {/*yylval. = std::atof(yytext);*/ return REAL_V;}
"real"                          {return REAL_T;}
 /* bool value/type */
"TRUE"                          {/*yylval. = true;*/ return BOOL_V;}
"FALSE"                         {/*yylval. = false;*/ return BOOL_V;}
"bool"                          {return BOOL_T;}
 /* exit program */
"exit"                          {return 0;}
 /* single character */
"+"|"-"|"*"|"/"|
","|";"|"("|")"|
":"                             {return yytext[0];}
 /* relational opts */
"="                             {/*yylval. = ;*/ return EQ_OPT;}
"~="                            {/*yylval. = ;*/ return NEQ_OPT;}
">"                             {/*yylval. = ;*/ return GRT_OPT;}
"<"                             {/*yylval. = ;*/ return LST_OPT;}
">="                            {/*yylval. = ;*/ return GRTEQ_OPT;}
"<="                            {/*yylval. = ;*/ return LSTEQ_OPT;}
 /* bool opts */
"AND"                           {/*yylval. = ;*/ return AND_OPT;}
"OR"                            {/*yylval. = ;*/ return OR_OPT;}
"~"                             {/*yylval. = ;*/ return NOT_OPT;}
 /* assignment */
":="                            {/*yylval. = ;*/ return ASSIGN_OPT;}
 /* ids */
[a-z]+                          {/*yylval. = strdup(yytext);*/ return ID_V;}
 /* any other thing */
.                               {yyerror("Unknown token! %c\n", *yytext);} /*complains about every unknown character*/

%%

void yyerror(const char *s, ...){
    va_list ap;
    va_start(ap, s);
    std::fprintf(stderr, "Line %d: error: ", yylineno);
    std::vfprintf(stderr, s, ap);
}
