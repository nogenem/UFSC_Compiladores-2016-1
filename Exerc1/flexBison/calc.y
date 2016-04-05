//http://rosettacode.org
//http://stackoverflow.com/questions/10723464/bison-lex-yystype-declaration-as-struct
//http://stackoverflow.com/questions/1430390/include-struct-in-the-union-def-with-bison-yacc

// Código de MMC, MDC e soma/sub
    // http://www.hardware.com.br/comunidade/fracoes/1298658/

%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;
%}

%code requires{
    struct frac {
        int n;
        int d;
    };
}

%code{
void yyerror(const char* s);
int MDC(int a, int b);
int MMC(int a, int b);
void simplify(frac& r);
void sum_sub(frac& n1, frac& n2, frac& r, char type);
void mult_div(frac& n1, frac& n2, frac& r, char type);
void exp(frac& n1, frac& n2, frac& r);
}

%union {
    struct frac num;
}

%token<num> T_FRAC;
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_EXP T_LEFT T_RIGHT
%token T_NEWLINE T_QUIT
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE
%left T_EXP
%right U_MINUS

%type<num> expression

%start calculation

%%

calculation:
	   | calculation line
;

line: T_NEWLINE
    | expression T_NEWLINE          { printf("\tResult: %i / %i\n", $<num.n>1, $<num.d>1); }
    | T_QUIT T_NEWLINE              { printf("bye!\n"); exit(0); }
;

expression: T_FRAC				            { $$ = $1; 
                                              simplify($$);
                                              printf("found: %d / %d\n", $<num.n>$, $<num.d>$); }
	| expression T_MULTIPLY expression		{ mult_div($1, $3, $$, '*'); 
                                              printf("*: %d / %d\n", $<num.n>$, $<num.d>$); }
    | expression T_DIVIDE expression		{ mult_div($1, $3, $$, '/'); 
                                              printf("/: %d / %d\n", $<num.n>$, $<num.d>$); }
    | expression T_EXP expression           { exp($1, $3, $$); 
                                              printf("**: %d / %d\n", $<num.n>$, $<num.d>$); }
	| expression T_PLUS expression          { sum_sub($1, $3, $$, '+'); 
                                              printf("+: %d / %d\n", $<num.n>$, $<num.d>$); }
	| expression T_MINUS expression         { sum_sub($1, $3, $$, '-'); 
                                              printf("-: %d / %d\n", $<num.n>$, $<num.d>$); }
	| T_LEFT expression T_RIGHT             { $$ = $2; }
    | T_MINUS expression %prec T_MINUS      { $<num.n>$ = -$<num.n>2; 
                                              $<num.d>$ = $<num.d>2;
                                              printf("-x: %d / %d\n", $<num.n>$, $<num.d>$); }
;

%%

//

int main() {
	yyin = stdin;

	do {
		yyparse();
	} while(!feof(yyin));

	return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	//exit(1);
}

int MDC(int a, int b){
    if (b == 0)
        return a;
    else
        return MDC(b,fmod(a,b));
}

int MMC(int a, int b){
    if (a != 0 && b != 0)
        return a*b/MDC(a,b);
    else
        return 0;
}

void simplify(frac& r){
    int mdc = MDC(r.n, r.d);
    r.n = r.n/mdc;
    r.d = r.d/mdc;
}

void sum_sub(frac& n1, frac& n2, frac& r, char type){
    if(n1.d == n2.d){
        r.n = type=='+' ? n1.n + n2.n : n1.n - n2.n;
        r.d = n1.d;
    }else{
        int aux = MMC(n1.d, n2.d);

        int tmp_n1 = (aux/n1.d)*n1.n;
        int tmp_n2 = (aux/n2.d)*n2.n;

        r.n = type=='+' ? tmp_n1 + tmp_n2 : tmp_n1 - tmp_n2;
        r.d = aux;
    }
}

void mult_div(frac& n1, frac& n2, frac& r, char type){
    if(type == '*'){
        r.n = n1.n * n2.n;
        r.d = n1.d * n2.d;
    }else{
        r.n = n1.n * n2.d;
        r.d = n1.d * n2.n;
    }
}

void exp(frac& n1, frac& n2, frac& r){
    if(n2.n < 0){
        int tmp = n1.n;
        n1.n = n1.d;
        n1.d = tmp;

        n2.n = -n2.n;
    }

    long tmp1 = pow(n1.n, n2.n);
    long tmp2 = pow(n1.d, n2.n);

    r.n = pow(tmp1, 1/(float)n2.d);
    r.d = pow(tmp2, 1/(float)n2.d);
}
