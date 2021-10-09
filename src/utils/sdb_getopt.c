/*
 * 
 * Copyright (c) 1980-2021, Bing Liu(spadcn@gmail.com)
 *
 * src/lib/sdb_getopt.c
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pwd.h>
#include "sdb_getopt.h"
#include "conninfo_gram.h"
#include "conninfo_scan.h"


static char *__sdb_load_conninfo_file() {
        FILE *fp;
        char *buf = malloc(1024);
        int len = 0;
        char *fpath;
        char *home = getenv("HOME");
        if (home) {
                len = strlen(home);
                fpath = calloc(1, len + sizeof("/.dbdbrc"));
                sprintf(fpath, "%s/%s", home, ".dbdbrc");
        } else {
                fpath = calloc(1, sizeof(".dbdbrc") + 1);
                sprintf(fpath, "%s", ".dbdbrc");
        }
        fp = fopen(fpath, "r");
        free(fpath);
        len = 0;
        
        if (fp) {
                len = fread(buf, 1, 1023, fp);
                buf[len] = '\0';
                fclose(fp);
                return buf;
        }
        free(buf);
        return NULL;
}

static void __parse_conninfo(const char *s, sdb_conn *dbconn) {
        yyscan_t scanner;
        yylex_init(&scanner);
        yy_scan_string(s, scanner);
        yyparse(scanner, dbconn);
        yylex_destroy(scanner);
        
        return;
}

void parse_conninfo(const char *s, sdb_conn *dbconn) {
        char *buf = __sdb_load_conninfo_file();
        
        if (buf) {
                __parse_conninfo(buf, dbconn);
                free(buf);
        }
        if (s) {
                __parse_conninfo(s, dbconn);
        }
        return;        
}

 
struct sdb_opts *sdb_options_init() {
        struct sdb_opts *options;
        
        options = malloc(sizeof(*options));
        memset(options, 0, sizeof(*options));

        options->dbconn = malloc(sizeof(*options->dbconn));
        memset(options->dbconn, 0, sizeof(*options->dbconn));
        
        return options;
}

void sdb_options_destory(struct sdb_opts *options) {
        sdb_free_conn(options->dbconn);
        free(options->dbconn);
        free(options->stmt);
        free(options);
        return;
}

void sdb_get_options(int argc, char *argv[], struct sdb_opts *options) {
	static struct option long_options[] = {
		{"dbname", required_argument, NULL, 'd'},
		{"table", required_argument, NULL, 'T'},
		{"host", required_argument, NULL, 'h'},
                {"port", required_argument, NULL, 'p'},
                {"sock", required_argument, NULL, 's'},
                {"conninfo", required_argument, NULL, 'c'},
                {"username", required_argument, NULL, 'u'},
                {"password", required_argument, NULL, 'P'},
		{"opt-separator", required_argument, NULL, 'o'},
		{"dbtype", required_argument, NULL, 't'},
		{"stmt", required_argument, NULL, 0},
		{"verbose", no_argument, NULL, 'v'},
		{"version", no_argument, NULL, 'V'},
                {"help", no_argument, NULL, '?'},
		{NULL, 0, NULL, 0}
        };
        
	int			optindex;
	int			c;

	while ((c = getopt_long(argc, argv, "d:h:p:s:c:u:P:o:t:T:vV?",
                                long_options, &optindex)) != -1)
	{
                
		switch (c)
		{
                case 0:
                        if (!strcmp(long_options[optindex].name, "stmt")) {
                                options->stmt = sdb_strdup(optarg);
//                                printf("Statement: %s\n", options->stmt);
                        }
                        break;                                        
                case 'd':
                        options->dbconn->dbname = sdb_strdup(optarg);
                        break;
                case 'h':
                        options->dbconn->host = sdb_strdup(optarg);
                        break;
                case 'p':
                        options->dbconn->port = sdb_strdup(optarg);
                        break;
                case 's':
                        options->dbconn->socket = sdb_strdup(optarg);
                        break;
                case 'c':
                        options->dbconn->conninfo = sdb_strdup(optarg);
                        break;
                case 'u':
                        options->dbconn->user = sdb_strdup(optarg);
                        break;
                case 'P':
                        options->dbconn->password = sdb_strdup(optarg);
                        break;
                case 'T':
                        options->dbconn->table = sdb_strdup(optarg);
                        break;
                case 'o':
                        options->seperator = optarg[0];                        
                        break;
                case 't':
                        if (!strcmp("mysql", optarg)) {
                                options->dbconn->dbtype = SDB_MYSQL;
                        } else if (!strcmp("postgresql", optarg)) {
                                options->dbconn->dbtype = SDB_POSTGRESQL;
                        }
                        break;                        
                case 'v':
                        options->verbose = true;
                        break;
                case 'V':
                        options->version = true;
                        show_version();
                        sdb_options_destory(options);
                        exit(EXIT_SUCCESS);
                case '?':
                {
                        
                        usage(argv[0]);
                        sdb_options_destory(options);
                        exit(EXIT_SUCCESS);
                }
                default:
                        break;
		}	
	}
        while (argc - optind >=1) {
                optind++;
        }
}

void show_version(void) {
	fprintf(stderr, "dbdb version: %s\n", SDB_VERSION);
}


void usage(const char *progname) {
	fprintf(stderr,"Usage:\n");
	fprintf(stderr, " %s [OPTION]... [DATADIR]\n", basename(progname));
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, " -d, --dbname=DBNAME       database names\n");
	fprintf(stderr, " -T, --table=TABLENAME     table names\n");
	fprintf(stderr, " -h, --host=HOST           database hostnames or IPs\n");
	fprintf(stderr, " -p, --port=PORT           database ports\n");
	fprintf(stderr, " -s, --sock=SOCKET         database sockets\n");
        fprintf(stderr, " -c, --conninfo=CONNINFO   database conninfo\n");
	fprintf(stderr, " -u, --username=NAME       database superuser name\n");
	fprintf(stderr, " -P, --password=PASSWORD   prompt for a password\n");
	fprintf(stderr, " -o, --opt-separator=SPT   separator for multiple arguments\n");
	fprintf(stderr, " -t, --dbtype=TYPE         database type\n");
	fprintf(stderr, " --stmt=STATEMENT          SQL statement\n");
        fprintf(stderr, " -v, --verbose             show verbose information\n");
	fprintf(stderr, " -V, --version             output version information, then exit.\n");
	fprintf(stderr, " -?, --help                show this help, then exit\n");

        return;
}

