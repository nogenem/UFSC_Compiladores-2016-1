%{
#include "itr.hpp"
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"
#include "at.hpp"
#include "ft.hpp"
	
AST::Block *programRoot;
AST::Node *lastParams=nullptr;

// Tables
ST::SymbolTable *symtab = new ST::SymbolTable(nullptr);
AT::ArrayTable arrtab;
FT::FuncTable functab;

extern int yylex();
extern void yyerror(const char* s, ...);

/*
	TODO:
		botar error no IF e While?
	
		adicionar token nil?
		
		fazer print de arrays? [muito trampo...]
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

%type <block> block fblock fblockend fchunk
%type <node> line fline assign decl namelist varlist ret
%type <node> exprlist2 expr expr2 term arrterm cond enqt
%type <node> functerm func namelist2

%left OR_OPT
%left AND_OPT
%left EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%left '+' '-'
%left '*' '/'
%nonassoc NOT_OPT U_MINUS
%nonassoc error

%start chunk

%%

chunk	: block 	{ programRoot = $1; }
		;

fchunk	: fblock 	{ $$ = $1; }
		| fblockend	{ $$ = $1; }
		| ret		{ $$ = new AST::Block(symtab);
		  	  	  	  $$->addLine($1); }  
		;

block	: line				{ $$ = new AST::Block(symtab);
						  	  if($1 != nullptr) $$->addLine($1); }
		| block line		{ if($2 != nullptr) $1->addLine($2); }
		;

fblock  : fline				{ $$ = new AST::Block(symtab);
  	  	  	  	  	  	  	  if($1 != nullptr) $$->addLine($1); }
		| fblock fline		{ if($2 != nullptr) $1->addLine($2); }
		| fblock fline ret	{ if($2 != nullptr) $1->addLine($2);
		  	  	  	  	  	  if($3 != nullptr) $1->addLine($3); }
		;
	
fblockend	: fline ret { $$ = new AST::Block(symtab);
						  if($1 != nullptr) $$->addLine($1);
						  $$->addLine($2); }
			;

ret		: RETURN_T expr ';' { $$ = new AST::Return($2); }
		| RETURN_T ';'		{ $$ = new AST::Return(nullptr); }
		;

fline   : decl ';'    	{ $$ = $1; }
        | assign ';'  	{ $$ = $1; }
        | cond END_T  	{ $$ = $1; }
        | enqt END_T  	{ $$ = $1; }
        | func END_T 	{ $$ = $1; }
        | error ';'   	{yyerrok; $$ = nullptr; }
        | error END_T 	{yyerrok; $$ = nullptr; }
        ;

line    : fline       		{ $$ = $1;
							  ITR::execExpr($$,false); }
        | expr ';'  		{ $$ = $1;
        					  ITR::execExpr($$,true); }
        | RETURN_T expr ';' { $$ = new AST::Return($2);
        					  ITR::execExpr($$,true); }
        ;

decl    : LOCAL_T namelist ASSIGN_OPT exprlist2	{ $$ = symtab->declVar($2, $4); }
		| LOCAL_T namelist						{ $$ = symtab->declVar($2,nullptr); }
        ;

assign  : varlist ASSIGN_OPT exprlist2 { $$ = symtab->assignVar($1, $3); }
        ;

cond    : IF_T expr THEN_T newscope fchunk endscope   
			{ $$ = new AST::CondExpr($2, $5, nullptr); }
        | IF_T expr THEN_T newscope fchunk endscope ELSE_T newscope fchunk endscope 
        	{ $$ = new AST::CondExpr($2, $5, $9); }
        ;

enqt    : WHILE_T expr DO_T newscope fchunk endscope  
			{ $$ = new AST::WhileExpr($2, $5); }
        ;

func    : FUN_T ID_V '(' namelist ')' newscope addparams fchunk endscope  
			{ auto var = new AST::Variable($2,nullptr,nullptr,Types::unknown_t,nullptr);
			  auto val = new AST::Function($4, $8);
			  $$ = symtab->assignVar(var, val); }          
        | LOCAL_T FUN_T ID_V '(' namelist ')' newscope addparams fchunk endscope  
        	{ auto var = new AST::Variable($3,nullptr,nullptr,Types::unknown_t,nullptr);
			  auto val = new AST::Function($5, $9);
			  $$ = symtab->declVar(var, val); }
        ;

namelist  : 		 	{ $$ = nullptr; lastParams = nullptr; }
		  | namelist2	{ $$ = $1; lastParams = $$; }
		  ;

namelist2  : ID_V               { $$ = new AST::Variable($1,nullptr,nullptr,Types::unknown_t,nullptr); }
           | ID_V ',' namelist2 { $$ = new AST::Variable($1,nullptr,nullptr,Types::unknown_t,$3); }
           ;

varlist   : ID_V                          { $$ = new AST::Variable($1,nullptr,nullptr,Types::unknown_t,nullptr); }
          | ID_V '[' expr ']'             { $$ = new AST::Variable($1,$3,nullptr,Types::unknown_t,nullptr); }
          | ID_V ',' varlist              { $$ = new AST::Variable($1,nullptr,nullptr,Types::unknown_t,$3); }
          | ID_V '[' expr ']' ',' varlist { $$ = new AST::Variable($1,$3,nullptr,Types::unknown_t,$6); }
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
		  | arrterm 	{ $$ = $1; }
		  | functerm	{ $$ = $1; }
		  ;

term    : BOOL_V                 { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V                  { $$ = new AST::Value($1, Types::int_t); }
        | ID_V                   { $$ = symtab->useVar($1, nullptr); }
        | ID_V '[' expr ']'      { $$ = symtab->useVar($1, $3); }
        | ID_V '(' exprlist2 ')' { $$ = nullptr; }
        ;

arrterm : '{' exprlist2 '}'       { $$ = new AST::Array($2); }
        | '{' '}'                 { $$ = new AST::Array(nullptr); }
        ;
        
functerm	: FUN_T '(' namelist ')' newscope addparams fchunk endscope END_T 
				{ $$ = new AST::Function($3, $7); }
			;

addparams	: { if(lastParams != nullptr) 
					symtab->declVar(lastParams, nullptr); }
			;

newscope	: { symtab = new ST::SymbolTable(symtab); }
			;
	
endscope	: { symtab = symtab->getPrevious(); }
			;
 
%%
