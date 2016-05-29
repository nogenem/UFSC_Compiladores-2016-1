%{
#include <string>
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"
#include "parser.hpp"

extern int yylineno;
void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

%%

[ \t\n\r]                       ; //ignores spaces and new lines
 /* int value/type */
[0-9]+                          {yylval.value = strdup(yytext); return INT_V;}
"int"                           {yylval.type = Types::integer_t; return INT_T;}
 /* real value/type */
[0-9]+\.[0-9]* |
[0-9]*\.[0-9]+                  {yylval.value = strdup(yytext); return REAL_V;}
"real"                          {yylval.type = Types::real_t; return REAL_T;}
 /* bool value/type */
"TRUE"                          {yylval.value = strdup(yytext); return BOOL_V;}
"FALSE"                         {yylval.value = strdup(yytext); return BOOL_V;}
"bool"                          {yylval.type = Types::bool_t; return BOOL_T;}
 /* assignment */
":="                            {return ASSIGN_OPT;}
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
"."                             {return yytext[0];}
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
 /* return */
"return"                        {return RETURN_T;}
 /* functions */
"def"                           {return DEF_T;}
"fun"                           {return FUN_T;}
"end"                           {return END_T;}
"decl"                          {return DECL_T;}
 /* IF */
"if"                            {return IF_T;}
"then"                          {return THEN_T;}
"else"                          {return ELSE_T;}
 /* while */
"while"                         {return WHILE_T;}
"do"                            {return DO_T;}
 /* Complex */
"type"                          {return TYPE_T;}
 /* ids */
[a-z]+                          {yylval.value = strdup(yytext); return ID_V;}
 /* any other thing */
.                               {Errors::print(Errors::unknown_symbol, *yytext);} /*complains about every unknown character*/

%%

void yyerror(const char *s, ...){
    va_list ap;
    va_start(ap, s);
    std::fprintf(stderr, "[Linha %d] Erro ", yylineno);
    std::vfprintf(stderr, s, ap);
    std::fprintf(stderr, "\n");
}
