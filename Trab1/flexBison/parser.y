%{
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(NULL);  /* main symbol table */

// Ultimo id usado na definição de uma função
std::string lastID = "";
// Ultima lista de parametros usada na
//  definição de uma função
AST::Node *lastFuncParams = nullptr;
// Gambiarra pra saber se estou começando
//  a ler os parametros do uso de uma função
bool useOfFunc = false;

extern int yylex();
extern void yyerror(const char* s, ...);

/*
  ASK:

  TODO:
    tratar os warning!? [problema ta no 'cond' e 'def']
    arrumar problema test((v)+2); !?
*/
%}

%define parse.trace

/* yylval == %union
 * union informs the different ways we can store data
 */
%union {
  const char *value;
  AST::Node *node;
  AST::Block *block;
  Types::Type type;
}

/* token defines our terminal symbols (tokens).
 */
%token<value> INT_V REAL_V BOOL_V ID_V
%token<type> INT_T REAL_T BOOL_T
%token RETURN_T DEF_T END_T DECL_T FUN_T ASSIGN_OPT
%token IF_T THEN_T ELSE_T WHILE_T DO_T
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

/* type defines the type of our nonterminal symbols.
 * Types should match the names used in the union.
 * Example: %type<node> expr
 */
%type <node> line fline decl def assign target expr term cond enqt
%type <node> varlist arraylist dparamlist uparamlist fdecl
%type <block> program lines flines
%type <type> vartype
%type <value> defid

/* Operator precedence for mathematical operators
 * The later it is listed, the higher the precedence
 */
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

/* Um programa é composto de linhas */
program : lines { programRoot = $1; symtab->checkFuncs(); }
        ;

/* As linhas de um programa são compostas de uma ou mais linhas */
lines   : line        { $$ = new AST::Block(symtab);
                        if($1 != NULL) $$->lines.push_back($1); }
        | lines line  { if($2 != NULL) $1->lines.push_back($2); }
        ;

/* As linhas do corpo de uma função são compostas de uma ou mais linhas */
flines  : fline         { $$ = new AST::Block(symtab);
                          if($1 != NULL) $$->lines.push_back($1); }
        | flines fline  { if($2 != NULL) $1->lines.push_back($2); }
        ;

/* Uma linha pode ter uma declaração de variaveis/arranjos
 *  ou uma definição/definição de função ou uma atribuição
 *  ou um IF ou um While ou um Return.
 */
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

/* Uma linha do corpo de uma função pode ter uma declaração
 *  de variaveis/arranjos ou uma atribuição ou um IF
 *  ou um While ou um Return.
 */
fline   : decl ';'                { $$ = $1; }
        | assign ';'              { $$ = $1; }
        | IF_T cond IF_T          { $$ = $2; }
        | WHILE_T enqt WHILE_T    { $$ = $2; }
        | RETURN_T expr ';'       { $$ = new AST::Return($2); }
        | error ';'               { yyerrok; $$ = NULL; }
        | IF_T error IF_T         { yyerrok; $$ = NULL; }
        | WHILE_T error WHILE_T   { yyerrok; $$ = NULL; }
        ;

/* A declaração de variaveis contem um tipo e uma lista de variaveis
 * A declaração de arranjos contem um tipo, um indice inteiro e
 *  uma lista de variaveis de arranjo
 */
decl    : vartype ':' varlist
          { $$ = $3; symtab->setType($$, $1); }
        | vartype '[' INT_V ']' ':' arraylist
          { $$ = $6; symtab->setType($$, $1);
            symtab->setArraySize($$, std::atoi($3)); }
        ;

/* A declaração de funções contem um tipo, o id da função
 * e a lista de seus parametros
 */
fdecl   : vartype ':' ID_V '(' dparamlist ')'
          { $$ = symtab->declFunction($3, $5, $1); }
        ;

/* A atribuição de valor para variaveis e arranjos contem
 *  um 'alvo', id ou id[indice], e uma expressão
 */
assign  : target ASSIGN_OPT expr  { $$ = new AST::BinOp($1, Ops::assign, $3); }
        ;

/* A definição de funções contem um tipo, o id da função,
 *  seus parametros e seu corpo
 * Uma função, sintaticamente, pode ter um corpo vazio
 */
def     : FUN_T vartype ':' defid '(' dparamlist ')' fnewscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = symtab->defFunction($4, $6, $9, $2); }
        | FUN_T vartype ':' defid '(' dparamlist ')' END_T
            { $$ = symtab->defFunction($4, $6, new AST::Block(), $2); }
        | FUN_T vartype ':' defid '(' dparamlist ')' error END_T
            { yyerrok; $$ = NULL; }
        ;

/* Uma condição, IF, contem uma expresão condicional,
 *  um ramo THEN e, opcionalmente, um ramo ELSE,
 *  funções só podem ser declaradas/definidas no escopo
 *  global, por isso o uso do 'flines' e não o 'lines'
 */
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

/* Um Laço While contem uma expressão e um corpo,
 *  funções só podem ser declaradas/definidas no escopo
 *  global, por isso o uso do 'flines' e não o 'lines'
 */
enqt    : expr DO_T newscope flines END_T
            { $$ = new AST::WhileExpr($1, $4); }
        | expr DO_T error END_T
            { yyerrok; $$ = NULL; }
        ;

/* Uma expressão consiste um apenas um termo ou uma operação
 *  binaria ou uma operação unária
 */
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

/* Um tipo pode ser inteiro, real ou booleano
 */
vartype : INT_T   { $$ = $1; }
        | REAL_T  { $$ = $1; }
        | BOOL_T  { $$ = $1; }
        ;

/* Uma lista de variaveis consiste de apenas um id
 *  ou uma sequencia de ids separados por ','
 */
varlist : ID_V              { $$ = symtab->newVariable($1, NULL, false); }
        | varlist ',' ID_V  { $$ = symtab->newVariable($3, $1, false); }
        ;

/* Uma lista de arranjos consiste de apenas um id
 *  ou uma sequencia de ids separados por ','
 */
arraylist : ID_V                { $$ = symtab->newVariable($1, NULL, true); }
          | arraylist ',' ID_V  { $$ = symtab->newVariable($3, $1, true); }
          ;

/* Um 'target' pode ser apenas uma variavel ou a indexação
 *  de um arranjo
 */
target  : ID_V              { $$ = symtab->assignVariable($1); }
        | ID_V '[' expr ']' { $$ = symtab->assignArray($1, $3); }
        ;

/* Regra extra apenas usada para salvar o ultimo id
 *  usado em uma função
 */
defid   : ID_V  { lastID = $1; $$ = $1; }
        ;

/* A lista de parametros da declaração/definição de funções
 *  consiste de NULL ou apenas uma variavel ou apenas um
 *  arranjo ou de uma lista de variaveis/arranjos
 */
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

/* Regra extra apenas usada para criar um novo 'escopo'
 */
newscope    : { symtab = new ST::SymbolTable(symtab); }
            ;

/* Regra extra usada para criar um novo 'escopo' para o corpo
 *  das funções e tambem para adicionar os parametros da função
 *  neste novo 'escopo'
 */
fnewscope   : { auto symbol = symtab->getSymbol(lastID);
                symtab = new ST::SymbolTable(symtab);
                symtab->addFuncParams(symbol!=nullptr?symbol->params:nullptr,
                  lastFuncParams); }
            ;

/* Regra extra para voltar um 'escopo' e criar um novo
 * Esta regra é usada no Else do IF, para sair do corpo
 *  do ramo THEN e criar um novo 'escopo' para o ramo ELSE
 */
enewscope   : { symtab = symtab->getPrevious();
                symtab = new ST::SymbolTable(symtab); }
            ;

/* Um termo pode ser um valor, booleano, inteiro ou real, ou
 *  uma variavel ou a indexação de um arranjo ou o uso de uma função
 */
term    : BOOL_V            { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V             { $$ = new AST::Value($1, Types::integer_t); }
        | REAL_V            { $$ = new AST::Value($1, Types::real_t); }
        | ID_V              { $$ = symtab->useVariable($1,useOfFunc); }
        | ID_V '[' expr ']' { $$ = symtab->useArray($1, $3); }
        | ID_V '(' useoffunc uparamlist ')'
          { $$ = symtab->useFunc($1, $4); useOfFunc=false; }
        ;

/* Regra extra usada para saber se estou começando a ler
 *  os parametros do uso de uma função
 */
useoffunc : { useOfFunc = true; }
          ;

/* A lista de parametros do uso de uma função consiste de
 * NULL ou uma expressão ou uma lista de expressões
 */
uparamlist  :                     { $$ = NULL; }
            | expr                { $$ = $1; }
            | expr ',' uparamlist { $$ = $1; $$->next = $3; }
            ;
%%
