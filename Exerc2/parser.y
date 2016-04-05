%{
#include "ast.h"
AST::Block *programRoot; /* the root node of our program AST:: */
extern int yylex();
extern void yyerror(const char* s, ...);
void print(const char *msg, AST::Node *a, AST::Node *b);
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

program : lines { programRoot = $1; }
        ;
        

lines   : line { $$ = new AST::Block(); $$->lines.push_back($1); std::printf("New line found.\n"); }
        | lines line { if($2 != NULL) $1->lines.push_back($2); }
        ;

line    : T_NL { $$ = NULL; std::printf("Nothing here to be used.\n"); } /*nothing here to be used */
        | expr T_NL /*$$ = $1 when nothing is said*/
        ;

expr    : T_INT { $$ = new AST::Integer($1); std::printf("Int value found.\n"); }
        | expr T_PLUS expr { $$ = new AST::BinOp($1,AST::plus,$3); print("Plus operation found (%d + %d).\n", $1, $3); }
        | expr T_MULTIPLY expr { $$ = new AST::BinOp($1,AST::multiply,$3); print("Multiply operation found (%d * %d).\n", $1, $3); }
        | expr error { yyerrok; $$ = $1; } /*just a point for error recovery*/
        ;

%%

// fazer definição de variaveis
// usando HASH

void print(const char *msg, AST::Node *a, AST::Node *b){
    int n = a->computeTree(), m = b->computeTree();
    std::printf(msg, n, m);
}
