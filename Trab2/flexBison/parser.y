%{
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"

AST::Block *programRoot;
ST::SymbolTable *symtab = new ST::SymbolTable(nullptr);

extern int yylex();
extern void yyerror(const char* s, ...);

/*
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

%token<value> INT_V BOOL_V ID_V
%token RETURN_T END_T FUN_T ASSIGN_OPT LOCAL_T
%token IF_T THEN_T ELSE_T WHILE_T DO_T
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

%type <block> program lines flines
%type <node> line fline assign decl namelist varlist
%type <node> exprlist exprlist2 expr expr2 term

%left OR_OPT
%left AND_OPT
%left EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%left '+' '-'
%left '*' '/'
%nonassoc NOT_OPT U_MINUS
%nonassoc error

%start program

%%

program : lines { programRoot = $1; }
        ;

lines   : line        { $$ = new AST::Block(symtab);
                        if($1 != nullptr) $$->addLine($1); }
        | lines line  { if($2 != nullptr) $1->addLine($2); }
        ;

flines  : fline			{ $$ = new AST::Block(symtab);
						  if($1 != nullptr) $$->addLine($1); }
        | flines fline  { if($2 != nullptr) $1->addLine($2); }
        ;

fline   : decl ';'    	{ $$ = $1; }
        | assign ';'  	{ $$ = $1; }
        | RETURN_T expr ';' { $$ = new AST::Return($2); }
        | cond END_T  	{ $$ = nullptr; }
        | enqt END_T  	{ $$ = nullptr; }
        | func END_T 	{ $$ = nullptr; }
        | error ';'   	{yyerrok;}
        | error END_T 	{yyerrok;}
        ;

line    : fline       	{ $$ = $1; }
        | expr ';'  	{ $$ = $1; }
        ;

decl    : LOCAL_T namelist ASSIGN_OPT exprlist2	{ $$ = symtab->declVar($2, $4); }
		| LOCAL_T namelist						{ $$ = symtab->declVar($2,nullptr); }
        ;

assign  : varlist ASSIGN_OPT exprlist2 { $$ = symtab->assignVar($1, $3); }
        ;

cond    : IF_T expr THEN_T flines   {}
        | IF_T expr THEN_T flines ELSE_T flines {}
        ;

enqt    : WHILE_T expr DO_T flines  {}
        ;

func    : FUN_T ID_V '(' namelist ')' flines  {}
        | FUN_T ID_V '(' ')' flines           {}
        ;

namelist  : ID_V              { $$ = new AST::Variable($1,nullptr,AST::unknown_u,Types::unknown_t,nullptr); }
          | ID_V ',' namelist { $$ = new AST::Variable($1,nullptr,AST::unknown_u,Types::unknown_t,$3); }
          ;

varlist   : ID_V                          { $$ = new AST::Variable($1,nullptr,AST::unknown_u,Types::unknown_t,nullptr); }
          | ID_V '[' expr ']'             { $$ = nullptr; }
          | ID_V ',' varlist              { $$ = new AST::Variable($1,nullptr,AST::unknown_u,Types::unknown_t,$3); }
          | ID_V '[' expr ']' ',' varlist { $$ = nullptr; }
          ;

exprlist  : expr              { $$ = $1; }
          | expr ',' exprlist { $$ = $1; $$->setNext($3); }
          ;
          
exprlist2  : expr2               { $$ = $1; }
           | expr2 ',' exprlist2 { $$ = $1; $$->setNext($3); }
           ;
          
expr      : term                    { $$ = $1; }
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

expr2	  : expr 		{ $$ = $1; }
		  | arrterm 	{ $$ = nullptr; }
		  | functerm	{ $$ = nullptr; }
		  ;

term    : BOOL_V                 { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V                  { $$ = new AST::Value($1, Types::int_t); }
        | ID_V                   { $$ = new AST::Variable($1,nullptr,AST::unknown_u,Types::unknown_t,nullptr); }
        | ID_V '[' expr ']'      { $$ = nullptr; }
        | ID_V '(' exprlist2 ')' { $$ = nullptr; }
        ;

arrterm : '{' exprlist '}'        {}
        | '{' '}'                 {}
        ;
        
functerm	: FUN_T '(' exprlist2 ')' flines END_T {}
			| FUN_T '(' ')' flines END_T {}
			;
 
%%
