/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  database.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/17/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/17/23 15:23:16"
 *                 
 ********************************************************************************/
#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "data_pack.h"
#define maxn 100

static sqlite3 *db;
sqlite3 *open_database(char *db_name);
int data_pack(struct send_data* data,char *devsn,int *tim_first);
int sql_insert(struct send_data *data);
int sql_select(void);
int sql_delect(void);
int callback(void *arg,int column,char **value,char **name);
#endif /* ----- #ifndef __DATABASE_H_ ----- */
