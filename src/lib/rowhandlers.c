#include <stdio.h>
#include <string.h>
#include <libpq-fe.h>
//#include "c.h"
//#include "pqexpbuffer.h"
//#include "postgres_fe.h"
#include "sdb.h"

#define MAX_COPY_LINE 500
#define STMT_BUF_SIZE 8192

static void __sdb_pg_row_copyto(char **, int, sdb_conn *);
static void __sdb_my_row_copyto(char **, int, sdb_conn *);
static void __sdb_pg_row_insert(char **, int, sdb_conn *);
static void __sdb_my_row_insert(char **, int, sdb_conn *);

static void sdb_copyto_commit();

void sdb_row_print(char **row, int num_fields, sdb_conn *tdbconn) {        
        static int num_row_handled = 0;

        if (num_fields == SDB_QUERY_ROW_END || row == NULL) {
                return;                
        }

        int i;
                
        printf("|");
        for (i = 0; i < num_fields; i++) {
                printf("%s|", row[i] ? row[i] : "NULL");
        }
        printf("\n");
        num_row_handled++;
        return;
}

void sdb_row_insert(char **row, int num_fields, sdb_conn *tdbconn) {
        if (tdbconn->status == SDB_CONNECTION_BAD) {
                sdb_connect(tdbconn);
        }

        if (tdbconn->dbtype == SDB_POSTGRESQL) {
                __sdb_pg_row_insert(row, num_fields, tdbconn);
        } else if (tdbconn->dbtype == SDB_MYSQL) {
                __sdb_my_row_insert(row, num_fields, tdbconn);
        } else {
                printf("not implemented yet");
        }

        return;
}

void __sdb_pg_row_insert(char **row, int num_fields, sdb_conn *tdbconn) {
        /*
          PQExpBuffer buf;

          int j = 1;
          int i = 0;
          int buflen;

          printfPQExpBuffer(buf, "INSERT INTO %s VALUES '('", tdbconn->table);

          while (i++ < num_fields - 1) {                
          appendPQExpBuffer(buf, "%s,", row[i]);
          }
          appendBinaryPQExpBuffer(buf, "%s')'", row[i]);

          if (tdbconn->status == SDB_CONNECTION_OK) {
          sdb_query(tdbconn, buf->data, NULL, NULL);
          } else {
          sdb_connect(tdbconn);
          }
        */
        return;
}

void __sdb_my_row_insert(char **row, int num_fields, sdb_conn *tdbconn) {
        return;
}

void sdb_row_copyto(char **row, int num_fields, sdb_conn *tdbconn) {
        static int num_row_handled = 0;

        if (num_row_handled % MAX_COPY_LINE == 0 || num_fields == SDB_QUERY_ROW_END) {
                sdb_copyto_commit();
        }
        
        if (tdbconn->dbtype == SDB_POSTGRESQL) {
                __sdb_pg_row_copyto(row, num_fields, tdbconn);
        }
        if (tdbconn->dbtype == SDB_MYSQL) {
                __sdb_my_row_copyto(row, num_fields, tdbconn);
        }
        num_row_handled++;

        return;
}

void __sdb_pg_row_copyto(char **row, int num_fields, sdb_conn *tdbconn) {

        return;
}


void __sdb_my_row_copyto(char **row, int nu_fields, sdb_conn *tdbconn) {

        return;
}


void sdb_copyto_commit() {

        return;
}
