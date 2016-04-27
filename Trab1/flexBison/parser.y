%{
#include "ast.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */

extern int yylex();
extern void yyerror(const char* s, ...);
%}

%define parse.trace

%union {
  int int_v;
  double real_v;
  bool bool_v;
  const char *name;
  AST::Node *node;
  AST::Block *block;
}

%token<int_v> INT_V
%token<real_v> REAL_V
%token<bool_v> BOOL_V
%token<name> ID_V
%token INT_T REAL_T BOOL_T RETURN_T
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

%type <node> line varlist
%type <block> program lines

%right ASSIGN_OPT
%left '+' '-'
%left '*' '/'
%right U_MINUS
%left AND_OPT OR_OPT
%right NOT_OPT
%left EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%nonassoc error

/* Starting rule
 */
%start program

%%

program : lines { programRoot = $1; }
		    ;

lines   : line ';' { $$ = new AST::Block();
                     if($1 != NULL) $$->lines.push_back($1); }
        | lines line ';' { if($2 != NULL) $1->lines.push_back($2); }
        | lines error { yyerrok; }
        ;

line    : INT_T ':' varlist { $$ = $3; }
        | REAL_T ':' varlist { $$ = $3; }
        | BOOL_T ':' varlist { $$ = $3; }
        ;

varlist : ID_V { $$ = new AST::Variable($1, NULL); }
        | varlist ',' ID_V { $$ = new AST::Variable($3, $1); }
        ;
%%
