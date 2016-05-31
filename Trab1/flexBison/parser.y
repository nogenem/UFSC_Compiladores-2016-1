%{
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(nullptr);  /* main symbol table */

/* guarda os ultimos dados lidos
 */
struct TmpInfo {
  Types::Type type;
  std::string compType;
  std::string id;
  int size;
  AST::Node *params;
  AST::Node *index;
} last;

bool isArray = false;
// Gambiarra pra saber se estou começando
//  a ler os parametros do uso de uma função
bool useOfFunc = false;
// Gambiarra para saber se estou dentro
//  do corpo de um tipo
bool insideType = false;
// Gambiarra para saber se deu syntax error
//  na leitura do tamanho de um array
// Ex: int[2.0]: a;
bool syntaxError = false;

extern int yylex();
extern void yyerror(const char* s, ...);

void handlerIDorARR(std::string id, AST::Node *index);
void handlerTorTARR(AST::Node *index);
void handlerDecl(AST::Node *&r, AST::Node *vars);
void handlerDParams(AST::Node *&r, std::string id, AST::Node *type, AST::Node *next);

/*
  ASK:
    test input3_v1.2
    declFunction -> redeclaration ? [st.cpp declFunction]

  TODO:

  OBS:
    test input4_v1.0
      ordem trocada
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
%token IF_T THEN_T ELSE_T WHILE_T DO_T TYPE_T
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

/* type defines the type of our nonterminal symbols.
 * Types should match the names used in the union.
 * Example: %type<node> expr
 */
%type <block> program lines flines tlines
%type <type> vartype
%type <value> defid
%type <node> line fline tline decl fdecl assign def cond enqt
%type <node> idORarr2 complist expr term dparamlist dparamlist2
%type <node> uparamlist uparamlist2 varlist tORtarr2

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
lines   : line        { $$ = new AST::Block();
                        if($1 != NULL) $$->addLine($1); }
        | lines line  { if($2 != NULL) $1->addLine($2); }
        ;

/* As linhas do corpo de uma função são compostas de uma ou mais linhas */
flines  : fline         { $$ = new AST::Block();
                          if($1 != NULL) $$->addLine($1); }
        | flines fline  { if($2 != NULL) $1->addLine($2); }
        ;

/* As linhas do corpo da definição de um tipo são compostas de
 *  uma ou mais linhas
 */
tlines  : tline         { $$ = new AST::Block();
                          if($1 != NULL) $$->addLine($1); }
        | tlines tline  { if($2 != NULL) $1->addLine($2); }
        ;

/* Uma linha pode ter uma declaração de variaveis/arranjos
 *  ou uma declaração/definição de função
 *  ou uma declaração de tipo ou uma atribuição
 *  ou um IF ou um While ou um Return.
 */
line    : assign ';'              { $$ = $1; }
        | decl ';'                { $$ = $1; }
        | DECL_T FUN_T fdecl ';'  { $$ = $3; }
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
fline   : assign ';'              { $$ = $1; }
        | decl ';'                { $$ = $1; }
        | IF_T cond IF_T          { $$ = $2; }
        | WHILE_T enqt WHILE_T    { $$ = $2; }
        | RETURN_T expr ';'       { $$ = new AST::Return($2); }
        | error ';'               { yyerrok; $$ = NULL; }
        | IF_T error IF_T         { yyerrok; $$ = NULL; }
        | WHILE_T error WHILE_T   { yyerrok; $$ = NULL; }
        ;

/* Uma linha do corpo de uma declaração de tipo só pode
 *  ter declarações de variaveis ou arranjos
 */
tline   : decl ';'                { $$ = $1; }
        | error ';'               { yyerrok; $$ = NULL; }
        ;

/* A atribuição de valor para variaveis e arranjos contem
 *  uma 'lista de componentes' -id, id[indice] ou uma sequencia deles-
 *  e uma expressão
 */
assign  : complist ASSIGN_OPT expr
            { symtab->assignVariable($1);
              $$ = new AST::BinOp($1, Ops::assign, $3); }
        ;

/* A declaração de variaveis e arranjos contem um tipo,
 *  composto ou primitivo, e uma lista de identificadores
 */
decl    : idORarr ':' varlist   { handlerDecl($$, $3); }
        | tORtarr ':' varlist   { handlerDecl($$, $3); }
        ;

/* A declaração de funções contem um tipo, o id da função
 * e a lista de seus parametros
 */
fdecl   : vartype ':' ID_V '(' dparamlist ')'
            { $$ = symtab->declFunction($5, $3, $1); }
        ;

/* A definição de funções contem um tipo, o id da função,
 *  seus parametros e seu corpo
 * Uma função, sintaticamente, pode ter um corpo vazio
 * A definição de tipos contem o id do tipo e seu corpo
 */
def     : FUN_T vartype ':' defid '(' dparamlist ')' fnewscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = symtab->defFunction($6, $9, $4, $2); }
        | FUN_T vartype ':' defid '(' dparamlist ')' END_T
            { $$ = symtab->defFunction($6, new AST::Block(), $4, $2); }
        | TYPE_T ':' ID_V newscope insidetype tlines END_T
            { auto st = symtab;
              symtab = symtab->getPrevious();
              insideType = false;
              $$ = symtab->defCompType($3, $6, st); }
        | TYPE_T ':' ID_V error END_T
            { yyerrok; $$ = NULL; }/* shift/reduce conflict */
        ;

/* Uma condição, IF, contem uma expresão condicional,
 *  um ramo THEN e, opcionalmente, um ramo ELSE
 */
cond    : expr THEN_T newscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = new AST::CondExpr($1, $4, nullptr); }
        | expr THEN_T newscope flines ELSE_T enewscope flines END_T
            { symtab = symtab->getPrevious();
              $$ = new AST::CondExpr($1, $4, $7); }
        | expr THEN_T error END_T
            { yyerrok; $$ = NULL; }/* shift/reduce conflict [ñ newscope] */
        | expr THEN_T newscope flines ELSE_T error END_T
            { yyerrok; $$ = NULL;
              symtab = symtab->getPrevious(); }/* shift/reduce conflict [s newscope] */
        ;

/* Um Laço While contem uma expressão e um corpo
 */
enqt    : expr DO_T newscope flines END_T
            { $$ = new AST::WhileExpr($1, $4); }
        | expr DO_T error END_T
            { yyerrok; $$ = NULL; }/* shift/reduce conflict [ñ newscope] */
        ;

/* lista de vars para tipos compostos
 */
complist  : idORarr2              { $$ = $1; }
          | idORarr2 '.' complist { $$ = $1; AST::Variable::cast($$)->setNextComp($3); }
          ;

/* id ou array geral, soh salva os valores
 */
idORarr : ID_V              { isArray = false; last.type = Types::composite_t;
                              last.compType = $1; last.index = NULL; last.size = -1; }
        | ID_V '[' expr ']' { handlerIDorARR($1, $3); }
        ;

/* id ou array para atribuicao/lista de params, gera os nodos dumb
 */
idORarr2  : ID_V              { $$ = new AST::Variable($1, Types::composite_t); }
          | ID_V '[' expr ']' { $$ = new AST::Array($3, $1, Types::composite_t); }
          ;

/* type ou type array geral, soh salva os valores
 */
tORtarr : vartype               { isArray = false; last.size = -1; }
        | vartype '[' expr ']'  { handlerTorTARR($3); }
        ;

/* type ou type array para lista de params, gera os nodos dumb
 */
tORtarr2  : vartype               { $$ = new AST::Variable("", $1); }
          | vartype '[' expr ']'  { $$ = new AST::Array($3, "", $1); }
          ;

/* Um tipo pode ser inteiro, real ou booleano
 */
vartype : INT_T   { $$ = $1; last.type = $1; }
        | REAL_T  { $$ = $1; last.type = $1; }
        | BOOL_T  { $$ = $1; last.type = $1; }
        ;

/* lista de vars para declaracao */
varlist : ID_V              { if(syntaxError) $$ = nullptr;
                              else $$ = symtab->newVariable($1, NULL, isArray, insideType); }
        | ID_V ',' varlist  { if(syntaxError) $$ = nullptr;
                              else $$ = symtab->newVariable($1, $3, isArray, insideType); }
        ;

/* Uma expressão consiste de apenas um termo ou uma operação
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

/* Um termo pode ser um valor booleano, inteiro ou real ou
 *  uma lista componentes ou o uso de uma função
 */
term    : BOOL_V            { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V             { $$ = new AST::Value($1, Types::integer_t); }
        | REAL_V            { $$ = new AST::Value($1, Types::real_t); }
        | complist          { $$ = symtab->useVariable($1, useOfFunc); }
        | ID_V '(' useoffunc uparamlist ')'
            { $$ = symtab->useFunc($4, $1); useOfFunc = false; }
        ;

/* A lista de parametros da declaração/definição de funções
 *  consiste de NULL ou apenas uma variavel ou apenas um
 *  arranjo ou de uma lista de variaveis/arranjos
 */
dparamlist  :             { $$ = NULL; last.params = NULL; }
            | dparamlist2 { $$ = $1;   last.params = $$; }
            ;

dparamlist2 : idORarr2 ':' ID_V
                { handlerDParams($$, $3, $1, nullptr); }
            | tORtarr2 ':' ID_V
                { handlerDParams($$, $3, $1, nullptr); }
            | idORarr2 ':' ID_V ',' dparamlist2
                { handlerDParams($$, $3, $1, $5); }
            | tORtarr2 ':' ID_V ',' dparamlist2
                { handlerDParams($$, $3, $1, $5); }
            ;

/* A lista de parametros do uso de uma função consiste de
 * NULL ou uma expressão ou uma lista de expressões
 */
uparamlist  :             { $$ = NULL; }
            | uparamlist2 { $$ = $1; }
            ;

uparamlist2 : expr                 { $$ = $1; }
            | expr ',' uparamlist2 { $$ = $1; $$->setNext($3); }
            ;

/* Regra extra apenas usada para criar um novo 'escopo'
 */
newscope    : { symtab = new ST::SymbolTable(symtab); }
            ;

/* Regra extra usada para criar um novo 'escopo' para o corpo
 *  das funções e tambem para adicionar os parametros da função
 *  neste novo 'escopo'
 */
fnewscope   : { auto symbol = symtab->getSymbol(last.id);
                symtab = new ST::SymbolTable(symtab);
                symtab->addFuncParams(symbol!=nullptr?symbol->getParams():nullptr,
                  last.params); }
            ;

/* Regra extra para voltar um 'escopo' e criar um novo
 * Esta regra é usada no Else do IF, para sair do corpo
 *  do ramo THEN e criar um novo 'escopo' para o ramo ELSE
 */
enewscope   : { symtab = symtab->getPrevious();
                symtab = new ST::SymbolTable(symtab); }
            ;

/* Regra extra apenas usada para salvar o ultimo id
 *  usado em uma função
 */
defid   : ID_V  { $$ = $1; last.id = $1; }
        ;

/* Regra extra usada para saber se estou começando a ler
 *  os parametros do uso de uma função
 */
useoffunc : { useOfFunc = true; }
          ;
/* Regra extra usada para saber se estou
 *  entrando no corpo de um tipo composto
 */
insidetype  : { insideType = true; }
            ;

%%

void handlerIDorARR(std::string id, AST::Node *index){
  isArray = true;
  last.type = Types::composite_t;
  last.compType = id;
  last.index = index;

  auto val = AST::Value::cast(index);
  if(val != nullptr &&
      val->getType() == Types::integer_t){
    last.size = std::atoi(val->getN());
  }else{
    last.size = -1;
    syntaxError = true;
  }
}

void handlerTorTARR(AST::Node *index){
  isArray = true;
  last.compType = "";
  last.index = nullptr;

  auto val = AST::Value::cast(index);
  if(val != nullptr &&
      val->getType() == Types::integer_t){
    last.size = std::atoi(val->getN());
  }else{
    last.size = -1;
    syntaxError = true;
  }
}

void handlerDecl(AST::Node *&r, AST::Node *vars){
  r = vars;
  if(isArray){
    if(last.size == -1){
      r = nullptr;
      Errors::print(Errors::syntax_error);
    }else{
      symtab->setType(r, last.type, last.compType);
      symtab->setArraySize(r, last.size);
    }
  }else
    symtab->setType(r, last.type, last.compType);

  syntaxError = false;
}

void handlerDParams(AST::Node *&r, std::string id, AST::Node *type, AST::Node *next){
  auto arr = AST::Array::cast(type);
  if(arr != nullptr){
    int size = -1;
    auto val = AST::Value::cast(arr->getIndex());
    if(val != nullptr &&
        val->getType() == Types::integer_t){
      size = std::atoi(val->getN());
    }
    if(size != -1){
      r = new AST::Array(nullptr, size, id, AST::param_u,
        arr->getId(), nullptr, next, arr->getType());
    }else{
      r = next;
      Errors::print(Errors::syntax_error);
    }
  }else{
    auto var = AST::Variable::cast(type);
    r = new AST::Variable(id, AST::param_u, var->getId(), nullptr,
      next, var->getType());
  }
  delete type;
}
