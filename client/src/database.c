/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  database.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 10:07:32"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include "database.h"
#include "zlog.h"

sqlite3 *open_database(char *db_name)
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	if(!db_name)
	{
		zlog_error(zc,"invalid input arguments");
	}
	ret = sqlite3_open(db_name,&db);
	if(ret != SQLITE_OK)
	{
		zlog_error(zc,"open database error:%s",sqlite3_errmsg(db));
	}
	zlog_info(zc,"database open  successfully");
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"create table if not exists data(""time char(50),"
			"temp char(50),"
			"sn char(20));");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		zlog_error(zc,"sql create table failure:%s",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
	zlog_info(zc,"create table successfully");
	return db;
}

int sql_insert(s_data *data)
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	if(!data)
	{
		zlog_error(zc,"invalid input arguments");
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"insert into data values('time:%s\n','temp:%s\n','sn:%s');",data->time,data->s_temp,data->sn);
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
	//	zlog_error(zc,"insert error:%s",error);
		sqlite3_close(db);
		sqlite3_free(error);
		return -1;
	}
//	zlog_info(zc,"insert into table successfully");
	return 0;
}

int sql_select(void)
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from data order by time desc limit 1;");
	ret = sqlite3_exec(db,sql,callback,NULL,&error);
	if(ret != SQLITE_OK)
	{
		zlog_info(zc,"select failure:%s",error);
		sqlite3_close(db);
		sqlite3_free(error);
		return -1;
	}
	printf("\n------------------------------\n");
	return 0;
}

int sql_delect()
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"delete from data order by time desc limit 1;");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		zlog_error(zc,"delete table error:%s",error);
		sqlite3_close(db);
		sqlite3_free(error);
		return -1;
	}
	return 0;
}

int callback(void *arg,int column,char **value,char **name)
{
	int i;
	for(i=0; i<column; i++)
	{
		printf("%s",value[i]?value[i]:"NULL");
	}
	return 0;
}


