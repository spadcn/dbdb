%{
#include "gram.h"
#include "scan.h"
%}

%union{
  int  num;
  char *str;
  sdb_type dbtype;
}

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

%token HOST SOCKET PORT
%token DBNAME USER PASSWORD
%token DBTYPE STMT 
%token EQUAL
%token <str> PRINT

%token LEX_ERROR

%%

conninfo: %empty
	| conninfo item
	;
item: 	  HOST EQUAL PRINT { dbconn->host = $3; }
	| PORT EQUAL PRINT { dbconn->port = $3; }
	| SOCKET EQUAL PRINT { dbconn->socket = $3; }
	| DBNAME EQUAL PRINT { dbconn->dbname = $3; }
	| USER EQUAL PRINT { dbconn->user = $3; }
	| PASSWORD EQUAL PRINT { dbconn->password = $3; }
	| DBTYPE EQUAL PRINT { dbconn->dbtype = sdb_get_dbtype($3, strlen($3)); free($3);}
	| PRINT EQUAL PRINT { fprintf(stderr, "Dismiss unknown parameter: %s.\n", $1);}
	;

%%
