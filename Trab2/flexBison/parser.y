%{
#include "itr.hpp"
#include "ast.hpp"
#include "st.hpp"
#include "util.hpp"
#include "at.hpp"
#include "ft.hpp"
	
AST::Block *programRoot;

/* Guarda a ultima lista de parâmetros
 *  lidos de uma função
 */
AST::Node *lastParams=nullptr;

// Tables
ST::SymbolTable *symtab = new ST::SymbolTable(nullptr);
AT::ArrayTable arrtab;
FT::FuncTable functab;

extern int yylex();
extern void yyerror(const char* s, ...);

/*
	TODO:
		fazer print de arrays? [muito trampo...]
		
	Impossivel de arrumar:
		while a > 1 do return a; while a > 1 do return a; end end
*/

%}

%define parse.trace

%union {
  const char *value;
  AST::Node *node;
  AST::Block *block;
  Types::Type type;
}

%token<value> INT_V BOOL_V ID_V NIL_V
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

/* Chunk global
 * É formado por um bloco
 */
chunk	: block 	{ programRoot = $1; }
		;

/* Chunk local
 * É formado por um bloco local ou
 *  um bloco local com um return no final ou
 *  um simples return
 */
fchunk	: fblock 	{ $$ = $1; }
		| fblockend	{ $$ = $1; }
		| ret		{ $$ = new AST::Block(symtab);
		  	  	  	  $$->addLine($1); }  
		| ret error ';' 	
			{ yyerrok; $$ = nullptr; }
		| fblockend	error ';'		
			{ yyerrok; $$ = nullptr; }
		;

/* Bloco global
 * É formado por uma ou mais linhas
 */
block	: line				{ $$ = new AST::Block(symtab);
						  	  if($1 != nullptr) $$->addLine($1); }
		| block line		{ if($2 != nullptr) $1->addLine($2); }
		;

/* Bloco local
 * É formado por uma ou mais linhas locais ou
 *  linhas locais com um return no final
 */ 
fblock  : fline				{ $$ = new AST::Block(symtab);
  	  	  	  	  	  	  	  if($1 != nullptr) $$->addLine($1); }
		| fblock fline		{ if($2 != nullptr) $1->addLine($2); }
		| fblock fline ret	{ if($2 != nullptr) $1->addLine($2);
		  	  	  	  	  	  if($3 != nullptr) $1->addLine($3); }
		;

/* Bloco local com return no final
 * É formado por uma linha e um return
 */
fblockend	: fline ret { $$ = new AST::Block(symtab);
						  if($1 != nullptr) $$->addLine($1);
						  $$->addLine($2); }
			;

/* Return
 * É formado por um token RETURN e uma expressão ou
 *  apenas um token RETURN
 */
ret		: RETURN_T expr2 ';' { $$ = new AST::Return($2); }
		| RETURN_T ';'		 { $$ = new AST::Return(nullptr); }
		;

/* Linha local
 * É formada por uma declaração ou
 *  uma atribuição ou
 *  uma condição (IF) ou
 *  um laço (WHILE) ou
 *  uma função
 */
fline   : decl ';'    	{ $$ = $1; }
        | assign ';'  	{ $$ = $1; }
        | cond END_T  	{ $$ = $1; }
        | enqt END_T  	{ $$ = $1; }
        | func END_T 	{ $$ = $1; }
        | error ';'   	{ yyerrok; $$ = nullptr; }
        ;

/* Linha global 
 * É formada por uma linha local ou
 *  uma expressão ou
 *  um token RETURN e uma expressão ou
 *  apenas um token RETURN
 */
line    : fline       		 { $$ = $1;
							   ITR::execExpr($$,false); }
        | expr ';'  		 { $$ = $1;
        					   ITR::execExpr($$,true); }
        | RETURN_T expr2 ';' { $$ = new AST::Return($2);
        					   ITR::execExpr($$,true); }
        | RETURN_T ';'		 { $$ = new AST::Return(nullptr);
        					   ITR::execExpr($$,true); }
        ;

/* Declaração
 * É formada por uma lista de atribuições ou
 *  apenas por uma lista de nomes
 * É possivel ter mais expressões que nomes e vice-versa
 */
decl    : LOCAL_T namelist ASSIGN_OPT exprlist2	{ $$ = symtab->declVar($2, $4); }
		| LOCAL_T namelist						{ $$ = symtab->declVar($2,nullptr); }
        ;

/* Atribuição
 * É formada por uma lista de atribuições
 * É possivel ter mais expressões que nomes e vice-versa
 */
assign  : varlist ASSIGN_OPT exprlist2 { $$ = symtab->assignVar($1, $3); }
        ;

/* Condicional (IF)
 * É formado por expressões do tipo if-then ou
 *  if-then-else
 */
cond    : IF_T expr THEN_T newscope fchunk endscope   
			{ $$ = new AST::CondExpr($2, $5, nullptr); }
        | IF_T expr THEN_T newscope fchunk endscope ELSE_T newscope fchunk endscope 
        	{ $$ = new AST::CondExpr($2, $5, $9); }
        	
        /* TRATAMENTO DE ERROS EH UM SACO! */
        | IF_T error THEN_T newscope fchunk endscope  
        	{ yyerrok; $$ = nullptr; }
        | IF_T expr THEN_T newscope error endscope  
        	{ yyerrok; $$ = nullptr; }
        | IF_T error THEN_T newscope error endscope  
        	{ yyerrok; $$ = nullptr; }
        | IF_T error THEN_T newscope fchunk endscope ELSE_T newscope fchunk endscope 
        	{ yyerrok; $$ = nullptr; }
        | IF_T expr THEN_T newscope error endscope ELSE_T newscope fchunk endscope 
        	{ yyerrok; $$ = nullptr; }
        | IF_T expr THEN_T newscope fchunk endscope ELSE_T newscope error endscope 
        	{ yyerrok; $$ = nullptr; }
        | IF_T error THEN_T newscope error endscope ELSE_T newscope fchunk endscope 
        	{ yyerrok; $$ = nullptr; }
        | IF_T error THEN_T newscope fchunk endscope ELSE_T newscope error endscope 
        	{ yyerrok; $$ = nullptr; }
        | IF_T expr THEN_T newscope error endscope ELSE_T newscope error endscope 
        	{ yyerrok; $$ = nullptr; }
        | IF_T error THEN_T newscope error endscope ELSE_T newscope error endscope 
        	{ yyerrok; $$ = nullptr; }
        ;

/* Laço (WHILE)
 * É formado por expressões do tipo while-do
 */
enqt    : WHILE_T expr DO_T newscope fchunk endscope  
			{ $$ = new AST::WhileExpr($2, $5); }
		
		/* TRATAMENTO DE ERROS EH UM SACO! */
		| WHILE_T error DO_T newscope fchunk endscope
			{ yyerrok; $$ = nullptr; }
		| WHILE_T expr DO_T newscope error endscope
			{ yyerrok; $$ = nullptr; }
		| WHILE_T error DO_T newscope error endscope
			{ yyerrok; $$ = nullptr; }
        ;

/* Função (function)
 * É formada por um 'Sintax Sugar' para atribuição de uma função a uma variáveis ou
 *  um 'Sintax Sugar para' a declaração de uma variável e atribuição de 
 *  uma função a esta variável
 *
 * A primeira forma é equivalente a:
 * 		a = function(...) ... end
 * enquanto a segunda é equivalente a:
 * 		local a;
 *		a = function(...) ... end
 */
func    : FUN_T ID_V '(' namelist ')' newscope addparams fchunk endscope  
			{ auto var = new AST::Variable($2,nullptr,nullptr,false,Types::unknown_t,nullptr);
			  auto val = new AST::Function($4, $8);
			  $$ = symtab->assignVar(var, val); }          
        | LOCAL_T FUN_T ID_V '(' namelist ')' newscope addparams fchunk endscope  
        	{ auto var = new AST::Variable($3,nullptr,nullptr,false,Types::unknown_t,nullptr);
			  auto val = new AST::Function($5, $9);
			  $$ = symtab->declVar(var, val); }
			  
		/* TRATAMENTO DE ERROS EH UM SACO! */
		| FUN_T ID_V '(' error ')' newscope addparams fchunk endscope
			{ yyerrok; $$ = nullptr; }
		| FUN_T ID_V '(' namelist ')' newscope addparams error endscope
			{ yyerrok; $$ = nullptr; }
		| FUN_T ID_V '(' error ')' newscope addparams error endscope
			{ yyerrok; $$ = nullptr; }
		| LOCAL_T FUN_T ID_V '(' error ')' newscope addparams fchunk endscope  
			{ yyerrok; $$ = nullptr; }
		| LOCAL_T FUN_T ID_V '(' namelist ')' newscope addparams error endscope  
			{ yyerrok; $$ = nullptr; }
		| LOCAL_T FUN_T ID_V '(' error ')' newscope addparams error endscope  
			{ yyerrok; $$ = nullptr; }
        ;

/* Lista de nomes
 * É formada por nada ou
 *  uma lista de nomes
 */
namelist  : 		 	{ $$ = nullptr; lastParams = nullptr; }
		  | namelist2	{ $$ = $1; lastParams = $$; }
		  ;

namelist2  : ID_V               { $$ = new AST::Variable($1,nullptr,nullptr,false,Types::unknown_t,nullptr); }
           | ID_V ',' namelist2 { $$ = new AST::Variable($1,nullptr,nullptr,false,Types::unknown_t,$3); }
           ;

/* Lista de variaveis
 * É formada por uma combinação de ids e
 *  indexação de arranjos
 */
varlist   : ID_V                          { $$ = new AST::Variable($1,nullptr,nullptr,false,Types::unknown_t,nullptr); }
          | ID_V '[' expr ']'             { $$ = new AST::Variable($1,$3,nullptr,false,Types::unknown_t,nullptr); }
          | ID_V ',' varlist              { $$ = new AST::Variable($1,nullptr,nullptr,false,Types::unknown_t,$3); }
          | ID_V '[' expr ']' ',' varlist { $$ = new AST::Variable($1,$3,nullptr,false,Types::unknown_t,$6); }
          ;
          
/* Lista de expressões
 * É formada por um ou mais expressões
 */
exprlist2  : expr2               { $$ = $1; }
           | expr2 ',' exprlist2 { $$ = $1; $$->setNext($3); }
           ;
 
/* Expressão 'simples'
 * É formada por um termo ou
 *  uma expressão aritmética ou
 *  uma expressão relacional ou
 *  uma expressão booleana ou
 *  uma parênteses 
 */        
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

/* Expressão mais 'geral'
 * É formada por uma expressão 'simples' ou
 *  um arranjo ou
 *  uma função
 */
expr2	  : expr 		{ $$ = $1; }
		  | arrterm 	{ $$ = $1; }
		  | functerm	{ $$ = $1; }
		  ;

/* Termo
 * É formado por um valor booleano ou
 *  uma valor inteiro ou
 *  uma valor nil (nulo) ou
 *  um id ou
 *  uma indexação de um arranjo ou
 *  uma chamada de função
 */
term    : BOOL_V                 { $$ = new AST::Value($1, Types::bool_t); }
        | INT_V                  { $$ = new AST::Value($1, Types::int_t); }
        | NIL_V					 { $$ = new AST::Value($1, Types::unknown_t); }
        | ID_V                   { $$ = symtab->useVar($1, nullptr, nullptr); }
        | ID_V '[' expr ']'      { $$ = symtab->useVar($1, $3, nullptr); }
        | ID_V '(' exprlist2 ')' { $$ = symtab->useVar($1, nullptr, $3, true); }
        | ID_V '(' ')'			 { $$ = symtab->useVar($1, nullptr, nullptr, true); }
        ;

/* Arranjo
 * É formado por uma lista de expressões dentro de chaves ou
 *  apenas as chaves
 */
arrterm : '{' exprlist2 '}'       { $$ = new AST::Array($2); }
        | '{' '}'                 { $$ = new AST::Array(nullptr); }
        ;

/* Função
 * É formada por uma lista de parâmetros e um bloco
 */  
functerm	: FUN_T '(' namelist ')' newscope addparams fchunk endscope END_T 
				{ $$ = new AST::Function($3, $7); }
			;

/* Regra 'extra' que adiciona os ultimos parâmetros lidos
 *  no escopo atual
 * É usada após a regra 'newscope' para adicionar os valores
 *  dos parâmetros antes de iniciar a leitura do bloco da função
 */
addparams	: { if(lastParams != nullptr) 
					symtab->declVar(lastParams, nullptr); }
			;

/* Regra 'extra' que cria um novo escopo
 */
newscope	: { symtab = new ST::SymbolTable(symtab); }
			;

/* Regra 'extra' que volta um escopo
 */	
endscope	: { if(!symtab->isGlobal())
					symtab = symtab->getPrevious(); }
			;
 
%%
