/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     VARIABLE = 259,
     PLUS = 260,
     MINUS = 261,
     TIMES = 262,
     DIVIDE = 263,
     POWER = 264,
     LEFT_PARENTHESIS = 265,
     RIGHT_PARENTHESIS = 266,
     SQRT = 267,
     LN = 268,
     LOG = 269,
     EXP = 270,
     SIN = 271,
     COS = 272,
     TAN = 273,
     ABS = 274,
     END = 275,
     NEG = 276
   };
#endif
#define NUMBER 258
#define VARIABLE 259
#define PLUS 260
#define MINUS 261
#define TIMES 262
#define DIVIDE 263
#define POWER 264
#define LEFT_PARENTHESIS 265
#define RIGHT_PARENTHESIS 266
#define SQRT 267
#define LN 268
#define LOG 269
#define EXP 270
#define SIN 271
#define COS 272
#define TAN 273
#define ABS 274
#define END 275
#define NEG 276




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



