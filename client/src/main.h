/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  main.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/20/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/20/23 00:01:50"
 *                 
 ********************************************************************************/
#ifndef __MAIN_H_
#define __MAIN_H_
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <sqlite3.h>
#include "get_temp.h"
#include "zlog.h"
#include "socket.h"
#include "database.h"
#include "data_pack.h"

zlog_category_t *zc;
void sig_handle(int signum);
void printf_usage(char *progname);
int main(int argc,char **argv);
#endif /* -----#ifndef __MAIN_H_ ----- */


