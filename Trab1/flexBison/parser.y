%{
#include "ast.hpp"
#include "st.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(NULL);  /* main symbol table */

extern int yylex();
extern void yyerror(const char* s, ...);

/*
  Perguntar ao professor sobre erros
*/
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

%type <node> line varlist expr
%type <block> program lines

%right ASSIGN_OPT
%left '+' '-'
%left '*' '/'
%left AND_OPT OR_OPT
%right NOT_OPT
%left EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%right U_MINUS
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
        | lines error ';' { yyerrok; std::cout << "\n"; }
        ;

line    : INT_T ':' varlist { $$ = $3; symtab->setType($$, ST::integer_t); }
        | REAL_T ':' varlist { $$ = $3; symtab->setType($$, ST::real_t); }
        | BOOL_T ':' varlist { $$ = $3; symtab->setType($$, ST::bool_t); }
        | ID_V ASSIGN_OPT expr { AST::Node *var = symtab->assignVariable($1);
                                 $$ = new AST::BinOp(var, AST::assign, $3); }
        ;

varlist : ID_V { $$ = symtab->newVariable($1, NULL, true); }
        | varlist ',' ID_V { $$ = symtab->newVariable($3, $1); }
        ;

expr    : BOOL_V { $$ = new AST::Bool($1); }
        | INT_V { $$ = new AST::Integer($1); }
        | REAL_V { $$ = new AST::Real($1); }
        | ID_V { $$ = symtab->useVariable($1); }
        | '(' expr ')' { $$ = new AST::Parentheses($2); }
        | expr '+' expr { $$ = new AST::BinOp($1, AST::plus, $3); }
        | expr '-' expr { $$ = new AST::BinOp($1, AST::b_minus, $3); }
        | expr '*' expr { $$ = new AST::BinOp($1, AST::times, $3); }
        | expr '/' expr { $$ = new AST::BinOp($1, AST::division, $3); }
        | '-' expr %prec U_MINUS { $$ = new AST::UniOp($2, AST::u_minus); }
        ;
%%
