%{
extern int yylex();
extern void yyerror(const char* s, ...);
%}

%define parse.trace

%union {

}

%token INT_T INT_V REAL_T REAL_V BOOL_T BOOL_V ID_V
%token EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT
%token AND_OPT OR_OPT NOT_OPT

//%type

%right ASSIGN_OPT
%left '+' '-'
%left '*' '/'
%right U_MINUS
%left AND_OPT OR_OPT
%right NOT_OPT
%left EQ_OPT NEQ_OPT GRT_OPT GRTEQ_OPT LST_OPT LSTEQ_OPT

%nonassoc error

/* Starting rule
 */
%start program

%%

program :
		;

%%
