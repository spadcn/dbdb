/* Companion source code for "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1
 * Copyright (c) 2009, Taughannock Networks. All rights reserved.
 * See the README file for license conditions and contact info.
 * $Header: /home/johnl/flnb/code/RCS/fb1-5.l,v 2.1 2009/11/08 02:53:18 johnl Exp $
 */

/* recognize tokens for the calculator and print them out */

%{
#include <stdio.h>
#include "gram.h"
%}

%option 8bit reentrant bison-bridge
%option warn noyywrap
%option case-insensitive
%option header-file="scan.h"

%%
"+"	{ return ADD; }
"-"     { return SUB; }
"*"	{ return MUL; }
"/"	{ return DIV; }
"|"     { return ABS; }
"[)0-9] *|"     { return OR; }
"&"     { return AND; }
"("     { return OP; }
")"     { return CP; }
"0x"[a-f0-9]+	{ *yylval = strtod(yytext, NULL); return HEXNUM; }
[0-9]+	{ *yylval = strtod(yytext, NULL); return NUMBER; }

"//".*  
[\n\r]  { return EOL; }
[ \t]   { /* ignore white space */ }
.	{ yyerror("Mystery character %c\n", NULL, yytext); }
%%

void yyerror(yyscan_t scanner, sdb_conn *dbconn, const char *msg) {
        fprintf(stderr, "%s\n", msg);
}
