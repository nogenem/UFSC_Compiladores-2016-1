%{
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(NULL);  /* main symbol table */

std::string lastID = "";
AST::Node *lastFuncParams = nullptr;

extern int yylex();
extern void yyerror(const char* s, ...);

/*
  ASK:

  TODO:
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
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

%type <node> line fline decl def assign target expr term
%type <node> varlist arraylist dparamlist
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

lines   : line { $$ = new AST::Block(symtab);
                 if($1 != NULL) $$->lines.push_back($1); }
        | lines line { if($2 != NULL) $1->lines.push_back($2); }
        ;

flines  : fline { $$ = new AST::Block(symtab);
                 if($1 != NULL) $$->lines.push_back($1); }
        | flines fline { if($2 != NULL) $1->lines.push_back($2); }
        ;

line    : decl ';'                      {$$ = $1;}
        | assign ';'                    {$$ = $1;}
        | DECL_T FUN_T vartype ':' ID_V '(' dparamlist ')' ';'
              { $$ = symtab->declFunction($5, $7, $3); }
        | DEF_T FUN_T def END_T DEF_T   { $$ = $3; }
        | RETURN_T expr ';'             { $$ = new AST::Return($2); }
        | error ';'                     { yyerrok; $$ = NULL; }
        ;

fline   : decl ';'                      {$$ = $1;}
        | assign ';'                    {$$ = $1;}
        | RETURN_T expr ';'             { $$ = new AST::Return($2); }
        | error ';'                     { yyerrok; $$ = NULL; }
        ;

decl    : vartype ':' varlist { $$ = $3; symtab->setType($$, $1); }
        | vartype '[' INT_V ']' ':' arraylist
              { $$ = $6; symtab->setType($$, $1);
                symtab->setArraySize($$, std::atoi($3)); }
        ;

def     : vartype ':' defid '(' dparamlist ')' newscope flines
            { symtab = symtab->getPrevious();
              $$ = symtab->defFunction($3, $5, $8, $1); }
        ;

defid   : ID_V { lastID = $1; $$ = $1; }
        ;

assign  : target ASSIGN_OPT expr { $$ = new AST::BinOp($1, Ops::assign, $3); }
        ;

target  : ID_V                { $$ = symtab->assignVariable($1); }
        | ID_V '[' expr ']'   { $$ = symtab->assignArray($1, $3); }
        ;

vartype : INT_T  { $$ = $1; }
        | REAL_T { $$ = $1; }
        | BOOL_T { $$ = $1; }
        ;

varlist : ID_V              { $$ = symtab->newVariable($1, NULL, false); }
        | varlist ',' ID_V  { $$ = symtab->newVariable($3, $1, false); }
        ;

arraylist : ID_V                { $$ = symtab->newVariable($1, NULL, true); }
          | arraylist ',' ID_V  { $$ = symtab->newVariable($3, $1, true); }
          ;

dparamlist  : { $$ = NULL; lastFuncParams = nullptr; }
            | vartype ':' ID_V
                { $$ = new AST::Variable($3, NULL, AST::param, $1); }
            | vartype '[' INT_V ']' ':' ID_V
                { $$ = new AST::Array($6, NULL, AST::param, std::atoi($3), $1); }
            | vartype ':' ID_V ',' dparamlist
                { $$ = new AST::Variable($3, $5, AST::param, $1);
                  lastFuncParams = $$; }
            | vartype '[' INT_V ']' ':' ID_V ',' dparamlist
                { $$ = new AST::Array($6, $8, AST::param, std::atoi($3), $1);
                  lastFuncParams = $$; }
            ;

newscope    : { auto symbol = symtab->getSymbol(lastID);
                symtab = new ST::SymbolTable(symtab);
                symtab->addFuncParams(symbol!=nullptr?symbol->params:nullptr,
                  lastFuncParams); }
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

term    : BOOL_V              { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V               { $$ = new AST::Value($1, Types::integer_t); }
        | REAL_V              { $$ = new AST::Value($1, Types::real_t); }
        | ID_V                { $$ = symtab->useVariable($1); }
        | ID_V '[' expr ']'   { $$ = symtab->useArray($1, $3); }
        ;
%%
