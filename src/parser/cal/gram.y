/* Companion source code for "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1
 * Copyright (c) 2009, Taughannock Networks. All rights reserved.
 * See the README file for license conditions and contact info.
 * $Header: /home/johnl/flnb/code/RCS/fb1-5.y,v 2.1 2009/11/08 02:53:18 johnl Exp $
 */

/* simplest version of calculator */

%{
#include <stdio.h>
#include "gram.h"
#include "scan.h"
%}

/* https://github.com/dangerousben/jsonval/issues/5 */
%code requires {
#include "sdb_getopt.h"
typedef void *yyscan_t;
extern void yyerror(yyscan_t scanner, sdb_conn *dbconn, const char *msg);
}
	/* LAC(LookAhead Correction) https://www.gnu.org/software/bison/manual/html_node/LAC.html */
%define parse.lac full
%define parse.error detailed
%define api.pure

/* https://github.com/dangerousben/jsonval/issues/5 */
%parse-param {yyscan_t *scanner} {sdb_conn *dbconn}
%lex-param {yyscan_t *scanner}

/* declare tokens */
%token NUMBER HEXNUM
%token OR AND
%token ADD SUB MUL DIV ABS
%token OP CP

%%

calclist: %empty 
 | calclist exp YYEOF{ printf("= %d | 0x%x\n> ", $2, $2); }
 ;

exp: bit
 | exp ABS bit { $$ = $1 | $3; }
 | exp AND bit { $$ = $1 & $3; }
 ;

bit: factor
 | bit ADD factor { $$ = $1 + $3; }
 | bit SUB factor { $$ = $1 - $3; }
 ;

factor: term
 | factor MUL term { $$ = $1 * $3; }
 | factor DIV term { $$ = $1 / $3; }
 ;

term: NUMBER
 | HEXNUM
 | ABS term { $$ = $2 >= 0? $2 : - $2; }
 | OP exp CP { $$ = $2; }
 ;
%%
