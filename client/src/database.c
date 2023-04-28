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
	snprintf(sql,sizeof(sql),"create table if not exists data(""time char(50),"
													   		   "temp char(50),"
															   "sn char(20));");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		zlog_error(zc,"sql create table failure:%s",error);
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
	snprintf(sql,sizeof(sql),"insert into data values('time:%s','temp:%s','sn:%s');",data->time,data->s_temp,data->sn);
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		zlog_error(zc,"insert error:%s",error);
		sqlite3_free(error);
		return -1;
	}
	return 0;
}

int sql_delete(void)
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	snprintf(sql,sizeof(sql),"delete from data order by time desc limit 1;");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("failure failure:%s\n",error);
		sqlite3_free(error);
		return -1;
	}
	else
	{
		return 0;
	}
}

int sql_data_count(void)
{
	int ret;
	int row;
	int column;
	int maxid = 0;
	char *error = 0;
	char **result;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	snprintf(sql,sizeof(sql),"select count(*) from data;");
	ret = sqlite3_get_table(db,sql,&result,&row,&column,&error);
	if(ret != SQLITE_OK)
	{
		zlog_error(zc,"get data count from table failure:%s",error);
		sqlite3_free(error);
		return -1;
	}

	if(row <= 0)
	{
		zlog_error(zc,"query records count from database failure");
		sqlite3_free(error);
		return -2;
	}

	maxid = atoi(result[1*column]);
	return maxid;
}

int sql_get_data(char *buf_send,int len,int maxid)
{
	int ret;
	int row;
	int column;
	char *error;
	char **result;
	char sql[maxn] = {0};
	
	memset(sql,0,sizeof(sql));
	snprintf(sql,sizeof(sql),"select * from data limit 1 offset %d;",maxid-1);
	ret = sqlite3_get_table(db,sql,&result,&row,&column,&error);
	if(ret != SQLITE_OK)
	{   
		printf("get table failure:%s\n",error);
		sqlite3_free(error);
		return -1;
	}
	snprintf(buf_send,128,"%s|%s|%s",result[1*column],result[1*column+1],result[1*column+2]);
	return 0;
}

int sql_close(void)
{
	char *error = 0;

	if(SQLITE_OK != sqlite3_close(db))
	{
		zlog_error(zc,"close database failure:%s",error);
		sqlite3_free(error);
		return -1;
	}
	zlog_info(zc,"close database successfully");
	return 0;
}
