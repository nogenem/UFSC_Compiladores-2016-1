%{
#include "ast.hpp"
#include "st.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(NULL);  /* main symbol table */

extern int yylex();
extern void yyerror(const char* s, ...);

/*
  ASK:

  TODO:
    Repensar em como fazer as mensagens de erro...
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
  ST::Type type;
}

%token<int_v> INT_V
%token<real_v> REAL_V
%token<bool_v> BOOL_V
%token<name> ID_V
%token<type> INT_T REAL_T BOOL_T
%token RETURN_T
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

%type <node> line varlist expr arraylist
%type <block> program lines
%type <type> vartype

%nonassoc EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%right ASSIGN_OPT
%left '+' '-' AND_OPT OR_OPT
%left '*' '/'
%right NOT_OPT U_MINUS
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

line    : vartype ':' varlist { $$ = $3; symtab->setType($$, $1); }
        | vartype '[' INT_V ']' ':' arraylist { $$ = $6; symtab->setType($$, $1);
                                            symtab->setArraySize($$, $3); }
        | ID_V ASSIGN_OPT expr { AST::Node *var = symtab->assignVariable($1);
                                 $$ = new AST::BinOp(var, AST::assign, $3); }
        | ID_V '[' expr ']' ASSIGN_OPT expr { AST::Node *var = symtab->assignArray($1, $3);
                                              $$ = new AST::BinOp(var, AST::assign, $6); }
        ;

vartype : INT_T  { $$ = $1; }
        | REAL_T { $$ = $1; }
        | BOOL_T { $$ = $1; }
        ;

varlist : ID_V { $$ = symtab->newVariable($1, NULL, false); }
        | varlist ',' ID_V { $$ = symtab->newVariable($3, $1, false); }
        ;

arraylist : ID_V { $$ = symtab->newVariable($1, NULL, true); }
          | arraylist ',' ID_V { $$ = symtab->newVariable($3, $1, true); }
          ;

expr    : BOOL_V { $$ = new AST::Bool($1); }
        | INT_V { $$ = new AST::Integer($1); }
        | REAL_V { $$ = new AST::Real($1); }
        | ID_V { $$ = symtab->useVariable($1); }
        | ID_V '[' expr ']' { $$ = symtab->useArray($1, $3); }
        | '(' expr ')' { $$ = new AST::Parentheses($2); }
        | expr '+' expr { $$ = new AST::BinOp($1, AST::plus, $3); }
        | expr '-' expr { $$ = new AST::BinOp($1, AST::b_minus, $3); }
        | expr '*' expr { $$ = new AST::BinOp($1, AST::times, $3); }
        | expr '/' expr { $$ = new AST::BinOp($1, AST::division, $3); }
        | expr AND_OPT expr { $$ = new AST::BinOp($1, AST::b_and, $3); }
        | expr OR_OPT expr { $$ = new AST::BinOp($1, AST::b_or, $3); }
        | expr EQ_OPT expr { $$ = new AST::BinOp($1, AST::eq, $3); }
        | expr NEQ_OPT expr { $$ = new AST::BinOp($1, AST::neq, $3); }
        | expr GRT_OPT expr { $$ = new AST::BinOp($1, AST::grt, $3); }
        | expr LST_OPT expr { $$ = new AST::BinOp($1, AST::lst, $3); }
        | expr GRTEQ_OPT expr { $$ = new AST::BinOp($1, AST::grteq, $3); }
        | expr LSTEQ_OPT expr { $$ = new AST::BinOp($1, AST::lsteq, $3); }
        | '-' expr %prec U_MINUS { $$ = new AST::UniOp($2, AST::u_minus); }
        | NOT_OPT expr { $$ = new AST::UniOp($2, AST::u_not); }
        ;
%%
