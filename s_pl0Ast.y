%{
#include <stdio.h>
#include "s_mypl0Ast.h"
FILE *fp;
void yyerror(char*);
int yylex();
int ln=1, cp=0;
%}
%union {
	myAstNode * ast;
	int num;
	char* ident;
}
%token TCONST TVAR TPROC TCALL TBEGIN TIF TTHEN TELSE TWHILE TDO TEND 
%token ODD NE LE GE ASSIGN NEG
%token <ast> ID NUM 
%type <ast> Block Decl ConstDec Constdef_list ConstDef VarDec IndexedDec ProcDef_list ProcDef
%type <ast> Ident_list Statement Statement_list Else_extended //If_extended 
%type <ast> Condition Expression Term Factor Var
		// Non Terminal�� ��� AST Ÿ���� ���� ������ ��

%right TELSE TTHEN
%left '+' '-'
%left '*' '/'
%left UM
%start Program
%%
Program: Block '.' {
		printf(" ==== valid syntax ====\n");
		printf("-- Print AST --\n"); printTree($1, 0);
		printf("-- CodeGen --\n"); Traverse($1); 
		} ;
Block: Decl Statement 		{ $$=buildTree(0, linking($1, $2)); } ;
Decl: ConstDec VarDec ProcDef_list 	{ $$ = linking($1, linking($2, $3)); } ;
ConstDec:			{ $$ = NULL; }
	| TCONST Constdef_list ';'	{ $$ = buildTree(TCONST, $2); } ;
Constdef_list: Constdef_list ',' ConstDef  { $$ = linking($1, $3); }
	| ConstDef ;
ConstDef: ID '=' NUM 		{ $$ = linking($1, $3); } ;
VarDec: TVAR Ident_list ';'		{ $$ = buildTree(TVAR, $2); } ;
	|			{ $$ = NULL; } ;
Ident_list: Ident_list ',' IndexedDec		{ $$ = linking($1, $3); }
	 | IndexedDec ;
ProcDef_list: ProcDef_list ProcDef	{ $$ = linking($1, $2); }
	| 	 		{ $$ = NULL; } ;
ProcDef: TPROC ID ';' Block ';' 	{ $$ = buildTree(TPROC, linking($2, $4)); } ;
Statement: Var ASSIGN Expression 	{ $$ = buildTree(ASSIGN, linking($1, $3)); }
	| TCALL ID		{ $$ = buildTree(TCALL, $2); }
	| TBEGIN Statement_list TEND { $$ = buildTree(TBEGIN, $2); }
	| TIF Condition TTHEN Statement TELSE Else_extended
		{ $$=buildTree(TIF,linking($2, linking($4, $6))); }
	| TIF Condition TTHEN Statement { $$ = buildTree(TIF, linking($2, $4)); }
	| TWHILE Condition TDO Statement { $$ = buildTree(TWHILE, linking($2, $4)); }
	| error { $$=NULL; }
	|  { $$=NULL; }  ;

Else_extended: Statement ;

Statement_list: Statement_list ';' Statement { $$ = linking($1, $3); }
	| Statement ;
Condition: ODD Expression		{ $$ = buildTree(ODD, $2); }
	| Expression '=' Expression	{ $$ = buildTree('=', linking($1, $3)); }
	| Expression NE Expression	{ $$ = buildTree(NE, linking($1, $3)); }
	| Expression '<' Expression	{ $$ = buildTree('<', linking($1, $3)); }
	| Expression '>' Expression	{ $$ = buildTree('>', linking($1, $3)); }
	| Expression GE Expression	{ $$ = buildTree(GE, linking($1, $3)); }
	| Expression LE Expression	{ $$ = buildTree(LE, linking($1, $3)); }  ;
Expression: Expression '+' Term	{ $$ = buildTree('+', linking($1, $3)); }
	| Expression '-' Term	{ $$ = buildTree('-', linking($1, $3)); }
	| '+' Term %prec UM { $$=$2; }
	| '-' Term %prec UM	{ $$ = buildTree(NEG, $2); }
	| Term ;
Term: 	Term '*' Factor		{ $$ = buildTree('*', linking($1, $3)); }
	| Term '/' Factor		{ $$ = buildTree('/', linking($1, $3)); }
	| Factor ;
Factor: 	Var	
	| NUM			
	| '(' Expression ')' 		{ $$=$2; } ;
Var:		ID
	| ID '[' Expression ']'		{ $$ = buildTree('[', linking($1, $3)); } ;
IndexedDec:	ID
	| ID '[' NUM ']'		{ $$ = buildTree('[', linking($1, $3)); } ;
%%
#include "lex.yy.c"
#include "s_mypl0Ast.c"
#include "s_CodeGen.c"
void yyerror(char* s) {
	printf("line: %d cp: %d %s\n", ln, cp, s);
}
int main() {
	if (yyparse()) return 1;
//		printf("===== Binary Code =====\n");
	fp=fopen("pl0.code", "w");
	for (int i=0; i<=cdx; i++) {
//		printf("%d	%d	%d	%d\n", i, Code[i].f, Code[i].l, Code[i].a);
		fprintf(fp, "%d	%d	%d	%d\n", i, Code[i].f, Code[i].l, Code[i].a);
		}
	fclose(fp);
	printf("------------------------------\n");
	interprete();
	return 0;
}
