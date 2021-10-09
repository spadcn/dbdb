#ifndef SDB_H
#define SDB_H

#define SDB_VERSION "1.0.1"
#define SDB_PROGNAME "sdb"
#define SDB_QUERY_ROW_END -1

typedef enum {
        SDB_POSTGRESQL = 1,
        SDB_MYSQL,
        SDB_ORACLE,
        SDB_UNKNOWN
} sdb_type;

typedef enum {
        SDB_INSERT = 0,
        SDB_DELETE,
        SDB_UPDATE,
        SDB_SEARCH,
        SDB_MIGRATE
} sdb_ops;        

typedef enum {
        SDB_NUMBER = 0,
        SDB_STRING,
        SDB_BYTE,
        SDB_BOOL
} sdb_field_type;

typedef struct{
        char *field_name;
        unsigned int nfield;
        unsigned int field_size;
        sdb_field_type field_type;
} sdb_field;

typedef enum {
        SDB_CONNECTION_BAD = 0,
        SDB_CONNECTION_OK,
        /* Non-blocking mode only below here */

        /*
         * The existence of these should never be relied upon - they should only
         * be used for user feedback or similar purposes.
         */
        SDB_CONNECTION_STARTED,                     /* Waiting for connection to be made.  */
        SDB_CONNECTION_MADE,                        /* Connection OK; waiting to send.     */
        SDB_CONNECTION_AWAITING_RESPONSE,   /* Waiting for a response from the
                                             * postmaster.        */
        SDB_CONNECTION_AUTH_OK,                     /* Received authentication; waiting for
                                                     * backend startup. */
        SDB_CONNECTION_SETENV,                      /* Negotiating environment. */
        SDB_CONNECTION_SSL_STARTUP,         /* Negotiating SSL. */
        SDB_CONNECTION_NEEDED,                      /* Internal state: connect() needed */
        SDB_CONNECTION_CHECK_WRITABLE,      /* Check if we could make a writable
                                             * connection. */
        SDB_CONNECTION_CONSUME,                     /* Wait for any pending message and consume
                                                     * them. */
        SDB_CONNECTION_GSS_STARTUP,         /* Negotiating GSSAPI. */
        SDB_CONNECTION_CHECK_TARGET         /* Check if we have a proper target connection */
} sdb_conn_status;

typedef struct {
	char *dbname;
        char *table;
        char *host;
        char *port;
        char *user;
        char *password;
        char *socket;
	char *parm;
        char *conninfo;
        sdb_conn_status status;
        sdb_type dbtype;
        void * conn;
} sdb_conn;        


extern void sdb_connect(sdb_conn *dbconn);
extern void sdb_disconn(sdb_conn *dbconn);
extern void sdb_query(sdb_conn *dbconn, char* sdb_stmt, void(*row_handler)(char **, int, sdb_conn *), sdb_conn *tdbconn);
extern void sdb_free_conn(sdb_conn *dbconn);
extern void sdb_row_print(char **row, int num_fields, sdb_conn *tdbconn);
extern void sdb_row_copyto(char **row, int num_fields, sdb_conn *tdbconn);

extern void sdb_operation(sdb_ops ops, sdb_conn *src_db, sdb_conn *tgt_db);
extern char *sdb_strdup(const char *str);
extern sdb_type sdb_get_dbtype(char *s, int len);
#endif  /* END SDB_H */
