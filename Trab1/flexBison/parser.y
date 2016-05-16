%{
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(NULL);  /* main symbol table */

std::string lastID = "";
AST::Node *lastFuncParams = nullptr;
bool useOfFunc = false;

extern int yylex();
extern void yyerror(const char* s, ...);

/*
  ASK:

  TODO:
    tratar os warning [problema ta no 'cond' e 'def']
    arrumar problema test((v)+2);
*/
%}

%define parse.trace

%union {
  const char *value;
  AST::Node *node;
  AST::Block *block;
  Types::Type type;
}

%token<value> INT_V REAL_V BOOL_V ID_V
%token<type> INT_T REAL_T BOOL_T
%token RETURN_T DEF_T END_T DECL_T FUN_T ASSIGN_OPT
%token IF_T THEN_T ELSE_T WHILE_T DO_T
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

%type <node> line fline decl def assign target expr term cond enqt
%type <node> varlist arraylist dparamlist uparamlist fdecl
%type <block> program lines flines
%type <type> vartype
%type <value> defid

%left AND_OPT OR_OPT
%left NOT_OPT
%nonassoc EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%left '+' '-'
%left '*' '/'
%nonassoc U_MINUS
%nonassoc error

/* Starting rule
 */
%start program

%%

program : lines { programRoot = $1; symtab->checkFuncs(); }
        ;

lines   : line        { $$ = new AST::Block(symtab);
                        if($1 != NULL) $$->lines.push_back($1); }
        | lines line  { if($2 != NULL) $1->lines.push_back($2); }
        ;

flines  : fline         { $$ = new AST::Block(symtab);
                          if($1 != NULL) $$->lines.push_back($1); }
        | flines fline  { if($2 != NULL) $1->lines.push_back($2); }
        ;

line    : decl ';'                { $$ = $1; }
        | DECL_T FUN_T fdecl ';'  { $$ = $3; }
        | assign ';'              { $$ = $1; }
        | DEF_T def DEF_T         { $$ = $2; }
        | IF_T cond IF_T          { $$ = $2; }
        | WHILE_T enqt WHILE_T    { $$ = $2; }
        | RETURN_T expr ';'       { $$ = new AST::Return($2); }
        | error ';'               { yyerrok; $$ = NULL; }
        | IF_T error IF_T         { yyerrok; $$ = NULL; }
        | WHILE_T error WHILE_T   { yyerrok; $$ = NULL; }
        | DEF_T error DEF_T       { yyerrok; $$ = NULL; }
        ;

fline   : decl ';'                { $$ = $1; }
        | assign ';'              { $$ = $1; }
        | IF_T cond IF_T          { $$ = $2; }
        | WHILE_T enqt WHILE_T    { $$ = $2; }
        | RETURN_T expr ';'       { $$ = new AST::Return($2); }
        | error ';'               { yyerrok; $$ = NULL; }
        | IF_T error IF_T         { yyerrok; $$ = NULL; }
        | WHILE_T error WHILE_T   { yyerrok; $$ = NULL; }
        ;

decl    : vartype ':' varlist
          { $$ = $3; symtab->setType($$, $1); }
        | vartype '[' INT_V ']' ':' arraylist
          { $$ = $6; symtab->setType($$, $1);
            symtab->setArraySize($$, std::atoi($3)); }
        ;

fdecl   : vartype ':' ID_V '(' dparamlist ')'
          { $$ = symtab->declFunction($3, $5, $1); }
        ;

assign  : target ASSIGN_OPT expr  { $$ = new AST::BinOp($1, Ops::assign, $3); }
        ;

def     : FUN_T vartype ':' defid '(' dparamlist ')' fnewscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = symtab->defFunction($4, $6, $9, $2); }
        | FUN_T vartype ':' defid '(' dparamlist ')' END_T
            { $$ = symtab->defFunction($4, $6, new AST::Block(), $2); }
        | FUN_T vartype ':' defid '(' dparamlist ')' error END_T
            { yyerrok; $$ = NULL; }
        ;

cond    : expr THEN_T newscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = new AST::CondExpr($1, $4, nullptr); }
        | expr THEN_T newscope flines ELSE_T enewscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = new AST::CondExpr($1, $4, $7); }
        | expr THEN_T newscope flines ELSE_T error END_T
            { yyerrok; $$ = NULL; symtab = symtab->getPrevious(); }
        | expr THEN_T error END_T
            { yyerrok; $$ = NULL; }
        ;

enqt    : expr DO_T newscope flines END_T
            { $$ = new AST::WhileExpr($1, $4); }
        | expr DO_T error END_T
            { yyerrok; $$ = NULL; }
        ;

expr    : term                    { $$ = $1; }
        | expr '+' expr           { $$ = new AST::BinOp($1, Ops::plus, $3); }
        | expr '-' expr           { $$ = new AST::BinOp($1, Ops::b_minus, $3); }
    		| expr '*' expr           { $$ = new AST::BinOp($1, Ops::times, $3); }
    		| expr '/' expr           { $$ = new AST::BinOp($1, Ops::division, $3); }
    		| expr AND_OPT expr       { $$ = new AST::BinOp($1, Ops::b_and, $3); }
    		| expr OR_OPT expr        { $$ = new AST::BinOp($1, Ops::b_or, $3); }
    		| expr EQ_OPT expr        { $$ = new AST::BinOp($1, Ops::eq, $3); }
    		| expr NEQ_OPT expr       { $$ = new AST::BinOp($1, Ops::neq, $3); }
    		| expr GRT_OPT expr       { $$ = new AST::BinOp($1, Ops::grt, $3); }
    		| expr LST_OPT expr       { $$ = new AST::BinOp($1, Ops::lst, $3); }
    		| expr GRTEQ_OPT expr     { $$ = new AST::BinOp($1, Ops::grteq, $3); }
    		| expr LSTEQ_OPT expr     { $$ = new AST::BinOp($1, Ops::lsteq, $3); }
    		| '-' expr %prec U_MINUS  { $$ = new AST::UniOp(Ops::u_minus, $2); }
    		| NOT_OPT expr            { $$ = new AST::UniOp(Ops::u_not, $2); }
    		| '(' expr ')'            { $$ = new AST::UniOp(Ops::u_paren, $2); }
    		;

vartype : INT_T   { $$ = $1; }
        | REAL_T  { $$ = $1; }
        | BOOL_T  { $$ = $1; }
        ;

varlist : ID_V              { $$ = symtab->newVariable($1, NULL, false); }
        | varlist ',' ID_V  { $$ = symtab->newVariable($3, $1, false); }
        ;

arraylist : ID_V                { $$ = symtab->newVariable($1, NULL, true); }
          | arraylist ',' ID_V  { $$ = symtab->newVariable($3, $1, true); }
          ;

target  : ID_V              { $$ = symtab->assignVariable($1); }
        | ID_V '[' expr ']' { $$ = symtab->assignArray($1, $3); }
        ;

defid   : ID_V  { lastID = $1; $$ = $1; }
        ;

dparamlist  : { $$ = NULL; lastFuncParams = nullptr; }
            | vartype ':' ID_V
              { $$ = new AST::Variable($3, NULL, AST::param, $1);
                lastFuncParams = $$; }
            | vartype '[' INT_V ']' ':' ID_V
              { $$ = new AST::Array($6, NULL, AST::param, std::atoi($3), $1);
                lastFuncParams = $$; }
            | vartype ':' ID_V ',' dparamlist
              { $$ = new AST::Variable($3, $5, AST::param, $1);
                lastFuncParams = $$; }
            | vartype '[' INT_V ']' ':' ID_V ',' dparamlist
              { $$ = new AST::Array($6, $8, AST::param, std::atoi($3), $1);
                lastFuncParams = $$; }
            ;

newscope    : { symtab = new ST::SymbolTable(symtab); }
            ;

fnewscope   : { auto symbol = symtab->getSymbol(lastID);
                symtab = new ST::SymbolTable(symtab);
                symtab->addFuncParams(symbol!=nullptr?symbol->params:nullptr,
                  lastFuncParams); }
            ;

enewscope   : { symtab = symtab->getPrevious();
                symtab = new ST::SymbolTable(symtab); }
            ;

term    : BOOL_V            { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V             { $$ = new AST::Value($1, Types::integer_t); }
        | REAL_V            { $$ = new AST::Value($1, Types::real_t); }
        | ID_V              { $$ = symtab->useVariable($1,useOfFunc); }
        | ID_V '[' expr ']' { $$ = symtab->useArray($1, $3); }
        | ID_V '(' useoffunc uparamlist ')'
          { $$ = symtab->useFunc($1, $4); useOfFunc=false; }
        ;

useoffunc : { useOfFunc = true; }
          ;

uparamlist  :                     { $$ = NULL; }
            | expr                { $$ = $1; }
            | expr ',' uparamlist { $$ = $1; $$->next = $3; }
            ;
%%
