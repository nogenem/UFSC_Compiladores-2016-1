%{
#include "ast.h"

AST::Block *programRoot; /* the root node of our program AST:: */
extern int yylex();
extern void yyerror(const char* s, ...);

void binOpFound(AST::Node *&r,  AST::Node *a, AST::Operation op, AST::Node *b);
void print(const char *format, ...);
%}

/* yylval == %union
 * union informs the different ways we can store data
 */
%union {
    int integer;
    AST::Node *node;
    AST::Block *block;
}

/* token defines our terminal symbols (tokens).
 */
%token <integer> T_INT
%token T_PLUS T_NL T_MULTIPLY

/* type defines the type of our nonterminal symbols.
 * Types should match the names used in the union.
 * Example: %type<node> expr
 */
%type <node> expr line
%type <block> lines program

/* Operator precedence for mathematical operators
 * The latest it is listed, the highest the precedence
 */
%left T_PLUS
%left T_MULTIPLY
%nonassoc error

/* Starting rule
 */
%start program

%%

program : lines { programRoot = $1; printf("ProgramRoot assigned.\n\n"); }
        ;


lines   : line { $$ = new AST::Block(); $$->lines.push_back($1);
                      printf("New block created and new line found.\n\n"); }
        | lines line { if($2 != NULL) { $1->lines.push_back($2); printf("New line found.\n\n"); } }
        ;

line    : T_NL { $$ = NULL; printf("Nothing here to be used.\n"); } /*nothing here to be used */
        | expr T_NL /*$$ = $1 when nothing is said*/
        ;

expr    : T_INT { $$ = new AST::Integer($1);
                        print("Int value found (%d).\n", $$->computeTree()); }
        | expr T_PLUS expr { binOpFound($$, $1, AST::plus, $3); }
        | expr T_MULTIPLY expr { binOpFound($$, $1, AST::multiply, $3); }
        | expr error { yyerrok; $$ = $1; } /*just a point for error recovery*/
        ;

%%

void binOpFound(AST::Node *&r,  AST::Node *a, AST::Operation op, AST::Node *b){
  r = new AST::BinOp(a, op, b);
  switch (op) {
    case AST::plus:
      print("Plus operation found (%d + %d).\n", a->computeTree(), b->computeTree());
      break;
    case AST::multiply:
      print("Multiply operation found (%d * %d).\n", a->computeTree(), b->computeTree());
      break;
    default:
      break;
  }
}

//http://www.cplusplus.com/reference/cstdio/vprintf/
void print(const char *format, ...){
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    va_end (args);
}
