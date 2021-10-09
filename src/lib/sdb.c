#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libpq-fe.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include <ctype.h>
#include "sdb_type.h"
#include "sdb.h"


static void __sdb_my_connect(sdb_conn *);
static void __sdb_pg_connect(sdb_conn *);
static void __sdb_my_disconn(sdb_conn *);
static void __sdb_pg_disconn(sdb_conn *);
static void __sdb_my_query(sdb_conn *, const char *, void (*row_handler)(char **, int, sdb_conn *), sdb_conn *);
static void __sdb_pg_query(sdb_conn *, const char *, void (*row_handler)(char **, int, sdb_conn *), sdb_conn *);
void sdb_free_conn(sdb_conn *);

/* caller's responsibility to call free() */
char *sdb_strdup(const char *in)
{
        char       *tmp;

        if (!in)
        {
                fprintf(stderr, "cannot duplicate null pointer (internal error)\n");
                exit(EXIT_FAILURE);
        }
        tmp = strdup(in);
        if (!tmp)
        {
                fprintf(stderr, "out of memory\n");
                exit(EXIT_FAILURE);
        }
        return tmp;
}

sdb_type sdb_get_dbtype(char *s, int len) {
        sdb_type dbtype;
        int i;
        for (i = 0; i < len; i++) {
                *(s + i) = tolower(*(s + i));
        }
        if (strcmp(s, "mysql") == 0 ) {
                dbtype = SDB_MYSQL;
        } else if (strcmp(s, "postgresql") == 0) {
                dbtype = SDB_POSTGRESQL;
        } else if (strcmp(s, "oracle") == 0) {
                dbtype = SDB_ORACLE;
        } else {
                dbtype = SDB_UNKNOWN;
        }

        return dbtype;
}

void sdb_connect(sdb_conn *dbconn) {
        if (!dbconn || !dbconn->port || !dbconn->dbname) {
                fprintf(stderr, "Error: No enough server information.\n");
                dbconn->status = SDB_CONNECTION_BAD;
                return;
        }

        if (dbconn->dbtype == SDB_MYSQL) {
                return __sdb_my_connect(dbconn);
        } else if (dbconn->dbtype == SDB_POSTGRESQL) {
                return __sdb_pg_connect(dbconn);
        } else {
                fprintf(stderr, "Error: Unknown database type.\n");
                dbconn->status = SDB_CONNECTION_BAD;
                return;
        }
}

void sdb_disconn(sdb_conn *dbconn) {
        if (!dbconn) {
                return;
        }

        if (dbconn->dbtype == SDB_MYSQL) {
                return __sdb_my_disconn(dbconn);
        } else if (dbconn->dbtype == SDB_POSTGRESQL) {
                return __sdb_pg_disconn(dbconn);
        }
	sdb_free_conn(dbconn);
        return;
}


/* Connect/Disconnect to MySQL server */
static void __sdb_my_connect(sdb_conn *dbconn) {
        MYSQL *mysql;
        mysql = mysql_init(NULL);
	unsigned long my_parm;
	if (dbconn->parm) {
                my_parm = strtoul(dbconn->parm, NULL, 10);
	} else {
                my_parm = 0;
	}
        
        mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, SDB_PROGNAME);
/*        printf("host: %s\tport: %s\n"
          "user: %s\tpass: %s\n"
          "dbnm: %s\tsock: %s\n", dbconn->host, dbconn->port,
          dbconn->user, dbconn->pass, dbconn->db, dbconn->sock);
*/
                
        if (mysql_real_connect(mysql, dbconn->host,     \
                               dbconn->user, dbconn->password, dbconn->dbname,  \
                               atoi(dbconn->port), dbconn->socket, my_parm)) {
                dbconn->conn = mysql;
                dbconn->status = SDB_CONNECTION_OK;
                return;
        }

        fprintf(stderr, "Error: Failed to connect database, %s\n",
                mysql_error(mysql));
        dbconn->status = SDB_CONNECTION_BAD;
        return;
}

static void __sdb_my_disconn(sdb_conn *dbconn) {
        if (dbconn->status == SDB_CONNECTION_OK) {
                mysql_close((MYSQL *)dbconn->conn);
                dbconn->status = SDB_CONNECTION_BAD;
        }
        return;
}


/* Connect/Disconnect to MySQL server */
static void __sdb_pg_connect(sdb_conn *dbconn) {
        PGconn *pg;
        pg = PQsetdbLogin(dbconn->host, dbconn->port, dbconn->parm, NULL, dbconn->dbname, dbconn->user, dbconn->password);
        if (pg) {
                if(PQstatus(pg) == CONNECTION_OK) {
                        dbconn->conn = pg;
                        dbconn->status = SDB_CONNECTION_OK;
                        return;
                }
        }
        dbconn->status = SDB_CONNECTION_BAD;
        fprintf(stderr, "%s", PQerrorMessage(pg));
        return;
}

static void __sdb_pg_disconn(sdb_conn *dbconn) {
        if(dbconn->status == SDB_CONNECTION_OK) {
                PQfinish(dbconn->conn);
                dbconn->status = SDB_CONNECTION_BAD;
        }
        return;
}

void sdb_query(sdb_conn *dbconn, char *sdb_stmt,      \
               void (*row_handler)(char **, int, sdb_conn *), sdb_conn *tdbconn) {
        if (dbconn->status != SDB_CONNECTION_OK || !sdb_stmt) {
                return;
        }
        if (dbconn->dbtype == SDB_MYSQL) {
                return __sdb_my_query(dbconn, sdb_stmt, row_handler, tdbconn);
        } else if (dbconn->dbtype == SDB_POSTGRESQL) {
                return __sdb_pg_query(dbconn, sdb_stmt, row_handler, tdbconn);
        } else {
                printf("Not implemented yet.");
                return;
        }
}

static void __sdb_my_query(sdb_conn *dbconn, const char *sdb_stmt, void \
                           (*row_handler)(char **, int, sdb_conn *), sdb_conn *tdbconn) {
        MYSQL_RES *result;
        MYSQL_ROW row;
        unsigned int num_fileds, num_rows;
        unsigned int i;
        MYSQL *mysql = (MYSQL *)dbconn->conn;
        
        if (mysql_query(mysql, sdb_stmt) != 0 ) {
                fprintf(stderr, "Failed to query database: %s\n",
                        mysql_error((MYSQL *)dbconn->conn));
                return;
        } else {
                if ((result = mysql_use_result(mysql))) {
                        num_fileds = mysql_num_fields(result);
                        while ((row = mysql_fetch_row(result))) {
                                if (row_handler) {
                                        row_handler(row, num_fileds, tdbconn);
                                }
                        }
                        /* 告诉rowhandlers行处理结束 num_fileds = ROW_END */
                        row_handler(NULL, SDB_QUERY_ROW_END, tdbconn);
                } else {
                        if (mysql_field_count(mysql) == 0) {
                                num_rows = mysql_affected_rows(mysql);
                        } else {
                                fprintf(stderr, "Error: %s\n", mysql_error(mysql));
                        }
                }
        }
                  
        return;
}
static void __sdb_pg_get_field(sdb_conn *dbconn, const char *sdb_stmt, sdb_field  **sdbfield, unsigned int *nfield) {
        PGresult *result;
        int len  = strlen(sdb_stmt);
        int sublen;
        int field;
        char *pos;
        char * query;
        pos = strcasestr(sdb_stmt, "limit");
        if (pos) {
                sublen = strlen(pos);
                query = calloc(len, 1);
                strncpy(query, sdb_stmt, len - sublen);
                strcpy(query + len - sublen, "limit 0");
        } else {
                query = calloc(len + sizeof(" limit 0"), 1);
                strcpy(query, sdb_stmt);
                if (sdb_stmt[len - 1] == ';') {
                        strcpy(query + len - 1, " limit 0");
                } else {
                        strcpy(query + len, " limit 0");
                }
        }
        
        result = PQexec(dbconn->conn, query);
        if (!result) {
                fprintf(stderr, "%s\n", PQerrorMessage(dbconn->conn));
                return;
        }
        *nfield = PQnfields(result);
        *sdbfield = calloc(sizeof(sdb_field),  *nfield);

        char *field_name[*nfield];

        for (field = 0; field < *nfield; field++) {
                (*sdbfield + field)->field_name = sdb_strdup(PQfname(result, field));
                (*sdbfield + field)->field_size = PQfsize(result, field);
                /* XXX This code is partially duplicated in ruleutils.c */
                switch (PQftype(result, field))
                {
                case INT2OID:
                case INT4OID:
                case INT8OID:
                case OIDOID:
                case FLOAT4OID:
                case FLOAT8OID:
                case NUMERICOID:
                        (*sdbfield + field)->field_type = SDB_NUMBER;
                        break;
                case BITOID:
                case VARBITOID:
                        (*sdbfield + field)->field_type = SDB_BYTE;
                        break;
                case BOOLOID:
                        (*sdbfield + field)->field_type = SDB_BOOL;
                        break;

                default:
                        (*sdbfield + field)->field_type = SDB_STRING;
                        break;
                }
        }
        free(query);
        PQclear(result);
}

static void __sdb_pg_print_field(sdb_conn *dbconn, const char *sdb_stmt) {
        sdb_field *sdbfield = NULL;
        unsigned int nfield = 0;
        __sdb_pg_get_field(dbconn, sdb_stmt, &sdbfield, &nfield);
        int i;
        for (i = 0; i < nfield; i++) {
                printf("|%s", (sdbfield + i)->field_name);
        }
        printf("|\n");
        return;
}

static void __sdb_pg_query(sdb_conn *dbconn, const char *sdb_stmt,    \
                           void (*row_handler)(char **, int, sdb_conn *), sdb_conn *tdbconn) {
        PGresult *result;
        unsigned int num_rows = 0;
        unsigned int num_fields;
        unsigned int i, j;
        ExecStatusType qtype;

        __sdb_pg_print_field(dbconn, sdb_stmt);

        if (!PQsendQuery(dbconn->conn, sdb_stmt)) {
                fprintf(stderr, "%s\n", PQerrorMessage(dbconn->conn));
                return;
        }
                        
        /*
          单行模式容易导致内存耗尽而被杀死
          PQsetSingleRowMode(conn);
          PQexec在大量返回结果时可能会被杀死，所以使用异步非阻塞方式。
        */
        if (!(result = PQgetResult(dbconn->conn))) {
                fprintf(stderr, "%s\n", PQresultErrorMessage(result));
                return;
        }                        

        if ((qtype = PQresultStatus(result)) == PGRES_TUPLES_OK) {
                num_fields = PQnfields(result);
                
                char *row[num_fields];
               
                while (result) {
                        num_rows = PQntuples(result);
                        for (i = 0; i < num_rows; i++) {
                                for (j = 0; j < num_fields; j++) {
                                        /*
                                          不能free row[j]，因为 PQgetvalue返回的结果指针是指向PGresult的一部分
                                          具体参考: https://www.postgresql.org/docs/current/libpq-exec.html
                                          如果free row[j], 在关闭连接时的PQfinish 会报如下错误:
                                          -------------------------------
                                          munmap_chunk(): invalid pointer
                                          Aborted (core dumped)
                                          -------------------------------
                                          
                                        */
                                        row[j] = PQgetvalue(result, i, j);
                                        
                                }

                                if (row_handler) {
                                        row_handler(row, num_fields, tdbconn);
                                }
                        }

                        
                        /* 告诉rowhandlers行处理结束 num_fileds = ROW_END */
                        row_handler(NULL, SDB_QUERY_ROW_END, tdbconn);
                        PQclear(result);
                        result = PQgetResult(dbconn->conn);
                }

        } else {
                PQclear(result);
                fprintf(stderr, "%s\n", PQerrorMessage(dbconn->conn));                
        }
        /*
          if (!(result = PQexec((PGconn *)dbconn->conn, sdb_stmt))) {

          if (!(result = PQsendQuery((PGconn *)dbconn->conn, sdb_stmt)) {
          fprintf(stderr, "%s\n", PQresultErrorMessage(result));
          return NULL;
          } else {
          if ((qtype = PQresultStatus(result)) == PGRES_TUPLES_OK) {
          num_fileds = PQnfields(result);
          num_rows = PQntuples(result);
          for (i = 0; i < num_rows; i++) {
          for (j = 0; j < num_fileds; j++) {
          char *field = PQgetvalue(result, i, j);
          printf("[%s]", field? field : "NULL");
          }
          printf("\n");
          }
          } else {
          fprintf(stderr, "%s\n", PQresultErrorMessage(result));
          }
        */
        return;
}

void sdb_free_conn(sdb_conn *dbconn) {
        free(dbconn->dbname);
        dbconn->dbname = NULL;
        
        free(dbconn->host);
        dbconn->host = NULL;
        
        free(dbconn->port);
        dbconn->port = NULL;
        
        free(dbconn->user);
        dbconn->user = NULL;
        
        free(dbconn->password);
        dbconn->password = NULL;
        
        free(dbconn->parm);
        dbconn->parm = NULL;
        
        free(dbconn->socket);
        dbconn->socket = NULL;

        free(dbconn->conninfo);
        dbconn->conninfo = NULL;
        
        free(dbconn->table);
        dbconn->table = NULL;
        
        return;
}
