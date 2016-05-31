%{
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"

AST::Block *programRoot; /* the root node of our program AST:: */
ST::SymbolTable *symtab = new ST::SymbolTable(nullptr);  /* main symbol table */

/* ideia temporaria para guardar
 *  os ultimos dados lidos
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
bool useOfFunc = false;
bool insideType = false;
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

  TODO:
    declFunction -> redeclaration ? [st.cpp 73]

  OBS:
    test input4_v1.0
      ordem trocada
    test v1.2
      Parametro -> parametro
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

program : lines { programRoot = $1; symtab->checkFuncs(); }
        ;

lines   : line        { $$ = new AST::Block();
                        if($1 != NULL) $$->addLine($1); }
        | lines line  { if($2 != NULL) $1->addLine($2); }
        ;

flines  : fline         { $$ = new AST::Block();
                          if($1 != NULL) $$->addLine($1); }
        | flines fline  { if($2 != NULL) $1->addLine($2); }
        ;

tlines  : tline         { $$ = new AST::Block();
                          if($1 != NULL) $$->addLine($1); }
        | tlines tline  { if($2 != NULL) $1->addLine($2); }
        ;

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

fline   : assign ';'              { $$ = $1; }
        | decl ';'                { $$ = $1; }
        | IF_T cond IF_T          { $$ = $2; }
        | WHILE_T enqt WHILE_T    { $$ = $2; }
        | RETURN_T expr ';'       { $$ = new AST::Return($2); }
        | error ';'               { yyerrok; $$ = NULL; }
        | IF_T error IF_T         { yyerrok; $$ = NULL; }
        | WHILE_T error WHILE_T   { yyerrok; $$ = NULL; }
        ;

tline   : decl ';'                { $$ = $1; }
        | error ';'               { yyerrok; $$ = NULL; }
        ;

assign  : complist ASSIGN_OPT expr
            { symtab->assignVariable($1);
              $$ = new AST::BinOp($1, Ops::assign, $3); }
        ;

/* declaração */
decl    : idORarr ':' varlist   { handlerDecl($$, $3); }
        | tORtarr ':' varlist   { handlerDecl($$, $3); }
        ;

/* declaração de função */
fdecl   : vartype ':' ID_V '(' dparamlist ')'
            { $$ = symtab->declFunction($5, $3, $1); }
        ;

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

enqt    : expr DO_T newscope flines END_T
            { $$ = new AST::WhileExpr($1, $4); }
        | expr DO_T error END_T
            { yyerrok; $$ = NULL; }/* shift/reduce conflict [ñ newscope] */
        ;

/* lista de vars para tipos compostos */
complist  : idORarr2              { $$ = $1; }
          | idORarr2 '.' complist { $$ = $1; AST::Variable::cast($$)->setNextComp($3); }
          ;

/* id ou array geral, soh salva os valores */
idORarr : ID_V              { isArray = false; last.type = Types::composite_t;
                              last.compType = $1; last.index = NULL; last.size = -1; }
        | ID_V '[' expr ']' { handlerIDorARR($1, $3); }
        ;

/* id ou array para atribuicao/lista de params, gera os nodos dumb */
idORarr2  : ID_V              { $$ = new AST::Variable($1, Types::composite_t); }
          | ID_V '[' expr ']' { $$ = new AST::Array($3, $1, Types::composite_t); }
          ;

/* type ou type array */
tORtarr : vartype               { isArray = false; last.size = -1; }
        | vartype '[' expr ']'  { handlerTorTARR($3); }
        ;

/* type ou type array para lista de params, gera os nodos dumb */
tORtarr2  : vartype               { $$ = new AST::Variable("", $1); }
          | vartype '[' expr ']'  { $$ = new AST::Array($3, "", $1); }
          ;

/* tipos */
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

term    : BOOL_V            { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V             { $$ = new AST::Value($1, Types::integer_t); }
        | REAL_V            { $$ = new AST::Value($1, Types::real_t); }
        | complist          { $$ = symtab->useVariable($1, useOfFunc); }
        | ID_V '(' useoffunc uparamlist ')'
            { $$ = symtab->useFunc($4, $1); useOfFunc = false; }
        ;

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

uparamlist  :             { $$ = NULL; }
            | uparamlist2 { $$ = $1; }
            ;

uparamlist2 : expr                 { $$ = $1; }
            | expr ',' uparamlist2 { $$ = $1; $$->setNext($3); }
            ;

newscope    : { symtab = new ST::SymbolTable(symtab); }
            ;

fnewscope   : { auto symbol = symtab->getSymbol(last.id);
                symtab = new ST::SymbolTable(symtab);
                symtab->addFuncParams(symbol!=nullptr?symbol->getParams():nullptr,
                  last.params); }
            ;

enewscope   : { symtab = symtab->getPrevious();
                symtab = new ST::SymbolTable(symtab); }
            ;

defid   : ID_V  { $$ = $1; last.id = $1; }
        ;

useoffunc : { useOfFunc = true; }
          ;

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
