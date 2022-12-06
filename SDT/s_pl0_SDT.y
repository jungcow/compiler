/*
	SDT Parser for PL0 Language
*/
%{
#include <stdio.h>
#include <string.h>

#include "interpreter.c"
FILE *fp;
void yyerror(char*);
int yylex();

#define CONST 	0
#define VAR 	1
#define PROC 	2
#define IDENT 	3  /* CONST + VAR */

#define TBSIZE 100	// symbol table size
#define LVLmax 20		// max. level depth

struct {	// symbol table
	char name[20];
	int type;		/* 0-CONST	1-VARIABLE	2-PROCEDURE */
	int lvl;
	int offst;
	} table[TBSIZE];
int block[LVLmax]; 	// Data for Set/Reset symbol table
typedef struct { // address format
	int l;
	int a;
	} Addr;

int Lookup(char *, int);  // lookup sym name for type, 
		      // if found setup LDiff/OFFSET and return 1 else return 0
void Enter(char *, int, int, int); // symbol insert
void SetBlock();
void ResetBlock();
void DisplayTable(); // symbol table dump
int GenLab(char *);	// label 생성(번호와 label) 및 현재 코드주소 저장
void EmitLab(int);	// label 출력 및 forward jump 주소 확정
void EmitLab1(int);	// label 출력 및 backward jump 주소 저장
void EmitPname(char *label);	// procedure 이름 출력
void Emit1(char *, fct, int, int);  // INT, LOD, LIT, STO 코드생성
void Emit2(char *, int, char *);  // CAL 코드생성
void Emit3(char *, fct, int);	// jmp, jpc 코드 생성
void Emit(char *, int);	// opr 코드생성

int ln=1, cp=0;
int lev=0;
int tx=0; // stack smbol table top pt.
int level=0; // block nesting level
int cdx=0; // code addr
int LDiff=0, OFFSET=0; // nesting level diff, offset(상대주소)
int Lno=0;
char Lname[10]; // 생성된 label
int Lab[20]; // 새로 생성된 label 에 대한 코드주소 저장

%}

%union {
	char ident[50];	// id lvalue
	int number;	// num lvalue
	}
%token TCONST TVAR TPROC TCALL TBEGIN TIF TTHEN TWHILE TDO TEND ODD NE LE GE ASSIGN
%token <ident> ID 
%token <number> NUM
%type <number> Dcl VarDcl Ident_list ProcHead
%left '+' '-'
%left '*' '/'
%left UM

%%
Program: Block '.' 
	{  Emit("END", 7); printf("\n ==== valid syntax ====\n"); } ;
Block: { Emit3("JMP", Jmp, $<number>$=GenLab(Lname) ); } 
	Dcl { EmitLab($<number>1); Emit1("INT", Int, 0, $2); } 
	Statement { DisplayTable(); } ;
Dcl: ConstDcl VarDcl ProcDef_list 	{	 } ;
ConstDcl:
	| TCONST Constdef_list ';' ;
Constdef_list: Constdef_list ',' ID '=' NUM 	{ 			 }
	| ID '=' NUM 	{ Enter($1, CONST, level, $3); }  ;
VarDcl: TVAR Ident_list ';'	{ 	 }
	|		{ $$=3; }  ;
Ident_list: Ident_list ',' ID	{ 			 }
	 | ID 		{ Enter($1, VAR, level, 3); $$=4; }  ;
ProcDef_list: ProcDef_list ProcDef
	| 	 ;
ProcDef: ProcHead	{ SetBlock(); } Block ';' { Emit("RET", 0); ResetBlock(); }  ;
ProcHead: TPROC ID ';' { Enter($2, PROC, level, cdx); EmitPname($2); }  ;
Statement: ID ASSIGN Expression { 					 }
	| TCALL ID		{ 				 }
	| TBEGIN Statement_list TEND
	| TIF Condition 		{ 					 }
		TTHEN Statement	{ 			 }
	| TWHILE 		{ EmitLab1($<number>$=GenLab(Lname) ); }    // backward jump addr
		Condition 	{ Emit3("JPC", Jpc, $<number>$=GenLab(Lname) ); }
		TDO Statement 	{ Emit3("JMP", Jmp, $<number>2); EmitLab($<number>4); }
	| error	 { /* yyerrok;*/ }
	|
	;
Statement_list: Statement_list ';' Statement
	| Statement  
	;
Condition: ODD Expression		{ 		 }
	| Expression '=' Expression	{  		}
	| Expression NE Expression	{  		}
	| Expression '<' Expression	{ 		 }
	| Expression '>' Expression	{ 		}
	| Expression GE Expression	{ 		 }
	| Expression LE Expression	{ 		 }  ;
Expression: Expression '+' Term	{ 		 }
	| Expression '-' Term	{ Emit("SUB", 3);  }
	| '+' Term %prec UM
	| '-' Term %prec UM	{ 		 }
	| Term ;
Term: Term '*' Factor		{ 		 }
	| Term '/' Factor		{ 		 }
	| Factor ;
Factor: ID			{ /* ID lookup 결과로 LOD 또는 LIT 코드 생성 */ }
	| NUM		{ Emit1("LIT", Lit, 0, $1); }
	| '(' Expression ')' ;
	
%%
#include "lex.yy.c"
void yyerror(char* s) {
	printf("line: %d cp: %d %s\n", ln, cp, s);
}
int Lookup(char *name, int type) { 
// 심볼 검색후 찾으면 LDiff(level diff)와 OFFSET(상대주소)를 지정하고 1을 리턴
// 없으면 0을 리턴

}
void Enter(char *name, int type, int lvl, int offst) {
// 새로운 심볼 삽입, type:종류, lvl:nesting-level, offst:상대주소
}

void SetBlock() {
	block[level++]=tx;
	}
void ResetBlock() { 
	tx=block[--level];
	}

void DisplayTable() { int idx=tx;
	printf("\n======== sym tbl contents ==========\n");
	while (--idx>=0) { 
		printf("%s	%d	%d	%d\n", table[idx].name, 
			table[idx].type, table[idx].lvl, table[idx].offst);
		}
	printf("---------------------------------------------------\n");
}
int GenLab(char *label) {
	Lab[Lno]=cdx;	// save code addr. for backward jump
	sprintf(label, "LAB%d", Lno);
	return Lno++;
}
void EmitLab(int label) {	// resolve forward jump label
	Code[Lab[label]].a=cdx; // fixed up forward jump label
	printf("LAB%d\n", label);
}
void EmitLab1(int label) {
//	Lab[label]=cdx; /* GenLab() 에서 시행
	printf("LAB%d\n", label);
}
void EmitPname(char *label) {
	printf("%s\n", label);
}
void Emit1(char *code, fct op, int ld, int offst) {   // INT, LOD, LIT, STO 코드생성, ld: level_diff.
	Instruction i;
	printf("%d	%s	%d	%d\n", cdx, code, ld, offst);
	i.f=op; i.l=ld; i.a=offst;
	Code[cdx++]=i;
}
void Emit2(char *code, int ld, char *name) {	// CAL 코드생성, ld: level_diff., OFFSET:code_addr.
	Instruction i;
	printf("%d	%s	%d	%s\n", cdx, code, ld, name);
	i.f=Cal; i.l=ld; i.a=OFFSET; // ld: level_diff.
	Code[cdx++]=i;
}
void Emit3(char *code, fct op, int label) {  // jmp, jpc 코드생성
	Instruction i;
	printf("%d	%s	LAB%d\n", cdx, code, label);
	i.f=op; i.l=0; i.a=Lab[label]; 	// fixed up backward jump
	Code[cdx++]=i;
}
void Emit(char *code, int op) {	// Opr 코드생성
	Instruction i;
	printf("%d	%s\n", cdx, code);
	i.f=Opr; i.l=0; i.a=op;
	Code[cdx++]=i;
}

void main() {
	if (yyparse()) return;
	printf("===== Binary Code =====\n");
	fp=fopen("pl0.code", "w");
	for (int i=0; i<=cdx; i++) {
		printf("%d	%d	%d	%d\n", i, Code[i]);
		fprintf(fp, "%d	%d	%d	%d\n", i, Code[i]);
}
	fclose(fp);
	printf("------------------------------\n");
	interprete();
}
