%{
#include <string>
#include "itr.hpp"
#include "ast.hpp"
#include "at.hpp"
#include "ft.hpp"
#include "st.hpp"
#include "parser.hpp"

extern int yylineno;
void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

%%

[ \t\n\r]                       ; //ignores spaces and new lines
 /* int value/type */
[0-9]+                          {yylval.value = strdup(yytext); return INT_V;}
 /* bool value/type */
"true"                          {yylval.value = strdup(yytext); return BOOL_V;}
"false"                         {yylval.value = strdup(yytext); return BOOL_V;}
 /* assign */
"="                             {return ASSIGN_OPT;}
 /* single character */
"+" |
"-" |
"*" |
"/" |
"," |
";" |
"(" |
")" |
"[" |
"]" |
":" |
"." |
"{" |
"}"                             {return yytext[0];}
 /* relational opts */
"=="                            {return EQ_OPT;}
"~="                            {return NEQ_OPT;}
">"                             {return GRT_OPT;}
"<"                             {return LST_OPT;}
">="                            {return GRTEQ_OPT;}
"<="                            {return LSTEQ_OPT;}
 /* bool opts */
"and"                           {return AND_OPT;}
"or"                            {return OR_OPT;}
"not"                           {return NOT_OPT;}
 /* return */
"return"                        {return RETURN_T;}
 /* end */
"end"                           {return END_T;}
 /* functions */
"function"                      {return FUN_T;}
 /* IF */
"if"                            {return IF_T;}
"then"                          {return THEN_T;}
"else"                          {return ELSE_T;}
 /* while */
"while"                         {return WHILE_T;}
"do"                            {return DO_T;}
 /* declaração de variaveis */
"local"                         {return LOCAL_T;}
 /* ids */
[a-z]+                          {yylval.value = strdup(yytext); return ID_V;}
 /* any other thing */
.                               {yyerror("simbolo desconhecido: %c.", *yytext);}

%%

void yyerror(const char *s, ...){
   va_list ap;
   va_start(ap, s);
   std::fprintf(stderr, "[Linha %d] Erro ", yylineno);
   std::vfprintf(stderr, s, ap);
   std::fprintf(stderr, "\n");
}
