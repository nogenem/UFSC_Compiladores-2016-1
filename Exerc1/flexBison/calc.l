%{
#include <string.h>
#include <stdio.h>

#define YY_DECL int yylex()

#include "calc.tab.hpp"

%}

digito [0-9]

%%

[ \t]	; // ignore all whitespace
{digito}+"/"[1-9]{digito}*	{
	char* ch = strtok(yytext, "/");
	yylval.num.n = atoi(ch);
	ch = strtok(0, "/");
	yylval.num.d = atoi(ch);
	return T_FRAC;
}
"("     {return T_LEFT;}
")"     {return T_RIGHT;}
"*"		{return T_MULTIPLY;}
"/"     {return T_DIVIDE;}
"+"     {return T_PLUS;}
"-"     {return T_MINUS;}
"**"    {return T_EXP;}
\n		{return T_NEWLINE;}
"exit"		{return T_QUIT;}
"quit"		{return T_QUIT;}

%%
