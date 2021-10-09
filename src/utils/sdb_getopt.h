/*
 * 
 * Copyright (c) 1980-2021, Bing Liu(spadcn@gmail.com)
 *
 * src/lib/sdb_getopt.c
 */

#ifndef SDB_GETOPT_H
#define SDB_GETOPT_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include "sdb.h"
#include "sdb_type.h"


struct sdb_opts {
        sdb_conn *dbconn;
        char *stmt;
        char seperator;
        bool verbose;
        bool version;
        int action;
};

void sdb_get_options(int argc, char *argv[],
							   struct sdb_opts *sdbopts);
void show_version();
void usage(const char *progname);
struct sdb_opts *sdb_options_init();
void sdb_options_destory(struct sdb_opts *options);
void parse_conninfo(const char *s, sdb_conn *dbconn);


#endif /* END SDB_GETOPT_H */
