/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TCONST = 258,
     TVAR = 259,
     TPROC = 260,
     TCALL = 261,
     TBEGIN = 262,
     TIF = 263,
     TTHEN = 264,
     TWHILE = 265,
     TDO = 266,
     TEND = 267,
     ODD = 268,
     NE = 269,
     LE = 270,
     GE = 271,
     ASSIGN = 272,
     NEG = 273,
     ID = 274,
     NUM = 275,
     UM = 276
   };
#endif
/* Tokens.  */
#define TCONST 258
#define TVAR 259
#define TPROC 260
#define TCALL 261
#define TBEGIN 262
#define TIF 263
#define TTHEN 264
#define TWHILE 265
#define TDO 266
#define TEND 267
#define ODD 268
#define NE 269
#define LE 270
#define GE 271
#define ASSIGN 272
#define NEG 273
#define ID 274
#define NUM 275
#define UM 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 9 "s_pl0Ast.y"
{
	myAstNode * ast;
	int num;
	char* ident;
}
/* Line 1529 of yacc.c.  */
#line 97 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

