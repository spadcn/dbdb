%{
#include "conninfo_gram.h"
%}

%option 8bit reentrant bison-bridge
%option warn noyywrap
%option case-insensitive
%option header-file="conninfo_scan.h"
%x noeqs
space			[[:space:]]
newline			[\n\r]
number                  [0-9]+
/* 需要更好的方法去掉首字符是=的字符串。[= 现在的方法去掉了所有=的字符串, 导致密码中不能有等号 */
/*print                   ([[:print:]]{-}[=[[:space:]])([[:print:]]{-}[[:space:]])* */
print                   ([[:print:]]{-}[=[[:space:]])+
/*                      print                   ([[:print:]]{-}[=[[:space:]])+ */
equal                   "="
imatch			{space}*"="
non_newline		[^\n\r]
comment			("--"{non_newline}*)
/*
horiz_space		[ \t\f]
string			[[:print:]]
whitespace		({space}+|{comment})
unknow                  ({print}{space}+=)               
*/                       
                        
%%
"host"                    { return HOST; }
"port"                    { return PORT; }
"socket"                  { return SOCKET; }
"dbname"                  { return DBNAME; }
"user"                    { return USER; }
"password"                { return PASSWORD; }
"dbtype"                 { return DBTYPE; }
"stmt"                    { return STMT; }
{equal}			{ return EQUAL; }
{print}                 { yylval->str = sdb_strdup(yytext); return PRINT; }
{comment} |
{space}                           /* return nothing */
.                       { printf("Unknown characters....\n"); return LEX_ERROR; }

%%

/*{print}                 { yylval->str = sdb_strdup(yytext); printf("YYLVAL->STR: %s\n", yylval->str); return PRINT; } */
void yyerror(yyscan_t scanner, sdb_conn *dbconn, const char *msg) {
	fprintf(stderr, "%s\n", msg);
}
