#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "sdb_getopt.h"

int main(int argc, char *argv[])
{
        struct sdb_opts *options = sdb_options_init();
        
        sdb_get_options(argc, argv, options);
        parse_conninfo(options->dbconn->conninfo, options->dbconn);

        sdb_connect(options->dbconn);
        sdb_query(options->dbconn, options->stmt, sdb_row_print, NULL);
        sdb_disconn(options->dbconn);
        sdb_options_destory(options);

       	return 0;
}
