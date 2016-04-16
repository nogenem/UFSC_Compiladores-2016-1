%{
#include "ast.h"
#include <string>
#include <map>
#include <typeinfo>

AST::Block *programRoot; /* the root node of our program AST:: */
extern int yylex();
extern void yyerror(const char* s, ...);

static std::map<std::string, AST::Node*> vars;

void buildVarBlock(AST::Node *&r, AST::Node *& a, AST::Node *& b);
void identifierFound(AST::Node *&r, std::string &id);

void useOfIdentifier(AST::Node *&r, std::string &id);
void assignmentFound(AST::Node *&r, std::string &id, AST::Node *value);

void binOpFound(AST::Node *&r,  AST::Node *a, AST::Operation op, AST::Node *b);
void print(const char *format, ...);
%}

/* yylval == %union
 * union informs the different ways we can store data
 */
%union {
    int integer;
    std::string *identifier;
    AST::Node *node;
    AST::Block *block;
}

/* token defines our terminal symbols (tokens).
 */
%token <integer> T_INT
%token <identifier> T_IDENT;
%token T_PLUS T_NL T_MULTIPLY T_ASSIGNMENT T_VIRGULA T_DEF

/* type defines the type of our nonterminal symbols.
 * Types should match the names used in the union.
 * Example: %type<node> expr
 */
%type <node> expr line def defexpr assign
%type <block> lines program

/* Operator precedence for mathematical operators
 * The latest it is listed, the highest the precedence
 */
%left T_ASSIGNMENT
%left T_PLUS
%left T_MULTIPLY
%nonassoc error

/* Starting rule
 */
%start program

//std::cout << $$ << " (" << (*$1) << ")\n\n";
%%

program : lines { programRoot = $1; printf("ProgramRoot assigned.\n\n"); }
        ;


lines   : line { $$ = new AST::Block(); $$->lines.push_back($1);
                      printf("New block created and new line found.\n\n"); }
        | lines line { if($2 != NULL) { $1->lines.push_back($2); printf("New line found.\n\n"); } }
        ;

line    : T_NL { $$ = NULL; printf("Nothing here to be used.\n"); } /*nothing here to be used */
        | expr T_NL /*$$ = $1 when nothing is said*/
        | def T_NL
        | assign T_NL
        ;

assign  : T_IDENT T_ASSIGNMENT expr { assignmentFound($$, *$1, $3); delete $1; }
        ;

def     : T_DEF defexpr { $$ = $2; printf("Block of definition found.\n"); }
        ;

defexpr : T_IDENT { identifierFound($$, *$1); delete $1; }
        | defexpr T_VIRGULA defexpr { buildVarBlock($$, $1, $3); }
        ;

expr    : T_INT { $$ = new AST::Integer($1);
                        print("Int value found (%d).\n", $$->computeTree()); }
        | T_IDENT { useOfIdentifier($$, *$1); delete $1; }
        | expr T_PLUS expr { binOpFound($$, $1, AST::plus, $3); }
        | expr T_MULTIPLY expr { binOpFound($$, $1, AST::multiply, $3); }
        | expr error { yyerrok; $$ = $1; } /*just a point for error recovery*/
        ;

%%

/*
  v0.1 = Soma dois inteiros
  v0.2 = multiplicação e debug
  v0.2.1 = uso de variaveis em expressoes [def var, var...]
  v0.2.2 = atribuição
  v0.4 = Definição de tipos [int a]
  v0.5 = Definição de escopos
*/

void buildVarBlock(AST::Node *&r, AST::Node *& a, AST::Node *& b){
  AST::Identifier * tmp = (AST::Identifier*)a;
  tmp->setNextIdent((AST::Identifier*)b);
  r = a;
}

void identifierFound(AST::Node *&r, std::string &id){
  if(vars.find(id) != vars.end())
    yyerror("Identifier (%s) already defined.\n", id.c_str());
  else{
    AST::Identifier *tmp = new AST::Identifier(id);
    vars[id] = tmp;
    r = tmp;

    printf("Identifier declared (%s).\n", id.c_str());
  }
}

void useOfIdentifier(AST::Node *&r, std::string &id){
  if(vars.find(id) != vars.end()){
    r = new AST::Integer(vars[id]->computeTree());
    print("Identifier found (%s).\n", id.c_str());
  }else
    yyerror("Identifier (%s) used before being declared!\n", id.c_str());
}

void assignmentFound(AST::Node *&r, std::string &id, AST::Node *value){
  if(vars.find(id) == vars.end())
    yyerror("Identifier (%s) used before being declared!\n", id.c_str());

  AST::Node *tmp = new AST::Identifier(id, value->computeTree());
  r = tmp;
  vars[id] = tmp;
  print("Assignment found (%s = %d).\n", id.c_str(), value->computeTree());
}

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
