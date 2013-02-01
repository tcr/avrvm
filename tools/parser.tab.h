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
     TOK_NUMBER = 258,
     TOK_USERFUNC = 259,
     TOK_ID = 260,
     TOK_IF = 261,
     TOK_ELSE = 262,
     TOK_DO = 263,
     TOK_FOR = 264,
     TOK_WHILE = 265,
     TOK_BREAK = 266,
     TOK_CONTINUE = 267,
     TOK_GOTO = 268,
     TOK_RETURN = 269,
     TOK_FUNCTION = 270,
     TOK_LOCAL = 271,
     TOK_GLOBAL = 272,
     TOK_GLOBAL_8U = 273,
     TOK_GLOBAL_8S = 274,
     TOK_ARRAY_8U = 275,
     TOK_ARRAY_8S = 276,
     TOK_ARRAY_16 = 277,
     TOK_EXTERN = 278,
     TOK_MEMADDR = 279,
     TOK_SECTION = 280,
     TOK_TRAMPOLINE = 281,
     TOK_LINE = 282,
     TOK_VMIP = 283,
     TOK_VMSP = 284,
     TOK_VMSFP = 285,
     TOK_PTR_8U = 286,
     TOK_PTR_8S = 287,
     TOK_PTR_16 = 288,
     TOK_PTR_F = 289,
     TOK_LOR = 290,
     TOK_LAND = 291,
     TOK_NE = 292,
     TOK_EQ = 293,
     TOK_GE = 294,
     TOK_LE = 295,
     TOK_SHR = 296,
     TOK_SHL = 297,
     NEG = 298,
     TOK_DEC = 299,
     TOK_INC = 300,
     TOK_ASSIGN_ADD = 301,
     TOK_ASSIGN_SUB = 302,
     TOK_ASSIGN_MUL = 303,
     TOK_ASSIGN_DIV = 304,
     TOK_ASSIGN_MOD = 305,
     TOK_ASSIGN_SHL = 306,
     TOK_ASSIGN_SHR = 307,
     TOK_ASSIGN_AND = 308,
     TOK_ASSIGN_OR = 309,
     TOK_ASSIGN_XOR = 310
   };
#endif
/* Tokens.  */
#define TOK_NUMBER 258
#define TOK_USERFUNC 259
#define TOK_ID 260
#define TOK_IF 261
#define TOK_ELSE 262
#define TOK_DO 263
#define TOK_FOR 264
#define TOK_WHILE 265
#define TOK_BREAK 266
#define TOK_CONTINUE 267
#define TOK_GOTO 268
#define TOK_RETURN 269
#define TOK_FUNCTION 270
#define TOK_LOCAL 271
#define TOK_GLOBAL 272
#define TOK_GLOBAL_8U 273
#define TOK_GLOBAL_8S 274
#define TOK_ARRAY_8U 275
#define TOK_ARRAY_8S 276
#define TOK_ARRAY_16 277
#define TOK_EXTERN 278
#define TOK_MEMADDR 279
#define TOK_SECTION 280
#define TOK_TRAMPOLINE 281
#define TOK_LINE 282
#define TOK_VMIP 283
#define TOK_VMSP 284
#define TOK_VMSFP 285
#define TOK_PTR_8U 286
#define TOK_PTR_8S 287
#define TOK_PTR_16 288
#define TOK_PTR_F 289
#define TOK_LOR 290
#define TOK_LAND 291
#define TOK_NE 292
#define TOK_EQ 293
#define TOK_GE 294
#define TOK_LE 295
#define TOK_SHR 296
#define TOK_SHL 297
#define NEG 298
#define TOK_DEC 299
#define TOK_INC 300
#define TOK_ASSIGN_ADD 301
#define TOK_ASSIGN_SUB 302
#define TOK_ASSIGN_MUL 303
#define TOK_ASSIGN_DIV 304
#define TOK_ASSIGN_MOD 305
#define TOK_ASSIGN_SHL 306
#define TOK_ASSIGN_SHR 307
#define TOK_ASSIGN_AND 308
#define TOK_ASSIGN_OR 309
#define TOK_ASSIGN_XOR 310




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 77 "parser.y"
{
	int number;
	char *string;
	struct evm_insn_s *insn;
	struct func_call_args_desc_s *fc;
	struct loopcontext_s *loopctx;
	struct array_init_s ainit;
	void *vp;
}
/* Line 1529 of yacc.c.  */
#line 169 "parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

