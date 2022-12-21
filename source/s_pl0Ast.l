%{
#include <string.h>
#include "s_mypl0Ast.h"
//#include "s_pl0Ast.tab.h"
Leaf nodeinfo;
int clev=0;
%}
%option noyywrap
%x	CMNT
%%
[ \t] | 
"(*"([^*]|"*"+[^*/])*"*)" ;
"//".*	 ;
"/*"		{ BEGIN CMNT; clev++; }
<CMNT>"/*"   {clev++;}
<CMNT>.		|
<CMNT>\n 	;
<CMNT>"*/" 	{ if(!--clev) BEGIN INITIAL; }
"odd" 		{cp+=3; return(ODD);}
"const"	 {cp+=5; return(TCONST); }
"var" 		{cp+=3; return(TVAR); }
"procedure" 	{cp+=8; return(TPROC); }
"call"		{cp+=4; return(TCALL); }
"begin"	{cp+=5; return(TBEGIN); }
"end"		{cp+=3; return(TEND); }
"if"		{cp+=2; return(TIF); }
"then"		{cp+=4; return(TTHEN); }
"else"		{cp+=4; return(TELSE); }
"while"		{cp+=5; return(TWHILE); }
"do"		{cp+=2; return(TDO); }
":="		{cp+=2; return(ASSIGN); }
"!="		{cp+=2; return(NE); }
"<="		{cp+=2; return(LE); }
">="		{cp+=2; return(GE); }
[0-9]+  	{ nodeinfo.num=atoi(yytext); yylval.ast=buildNode(-1,nodeinfo); cp+=yyleng; return NUM; } ;
[a-zA-Z][a-zA-Z0-9]*	{cp+=yyleng; strcpy(nodeinfo.ident, yytext); yylval.ast=buildNode(-2,nodeinfo); return(ID); }
.		{ cp++; return yytext[0]; } ;
[\n]		{ ln++; cp=0; }
%%

	
