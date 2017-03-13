#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//only this head should include for use bdb.
#include <db.h>   
#define DATABASE "yangjian.db"

int main()
{
    DB_ENV *myEnv;
    DB *dbp;
    DBT key, data;
    int ret,t_ret;
    u_int32_t env_flags;
    //........... Create an environment object and initialize it for error reporting
    ret = db_env_create(&myEnv, 0);
    if (ret != 0)
    {
          fprintf(stderr, "Error creating env handle: %s\n", db_strerror(ret));
          return -1;
    }
    //........If the environment does not exist create it. Initialize the in-memory cache.
    env_flags = DB_CREATE | DB_INIT_MPOOL;
    //........Open the environment.
    ret = myEnv->open(myEnv,"/home/ag/program/bt/blz/db/",env_flags,0);
    if (ret != 0)
    {
          fprintf(stderr, "Environment open failed: %s", db_strerror(ret));
          return -1;
    }

    if ((ret = db_create(&dbp, myEnv, 0)) != 0)
    {
          fprintf(stderr, "db_create: %s\n", db_strerror(ret));
          exit (1);
    }

    if ((ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0)
    {
          dbp->err(dbp, ret, "%s", DATABASE);
          exit (1);
    }
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
	key.data = (void*)"sport";
    key.size = sizeof("sport");
    data.data = (void*)"football";
    data.size = sizeof("football");
/*
    //......put data
    if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0)
    {
          printf("db: %s: key stored.\n", (char *)key.data);
    }
      else
    {
          dbp->err(dbp, ret, "DB->put");
    }
*/

    //........put data NOOVERWRITE
    if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0)
    printf("db: %s: key stored.\n", (char *)key.data);
    else dbp->err(dbp, ret, "DB->put");

    //.......get data
    if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0)
    printf("db: %s: key retrieved: data was %s.\n", (char *)key.data, (char *)data.data);
    else
    dbp->err(dbp, ret, "DB->get");

    //......del data
    if((ret = dbp->del(dbp, NULL, &key, 0)) == 0)
    printf("db: %s: key was deleted.\n", (char *)key.data);
    else
    dbp->err(dbp, ret, "DB->del");

    //.........close, only when the db successful closed,the data can real write to the disk.
    //if ((t_ret = dbp->close(dbp, 0)) != 0 && ret == 0)
    //ret = t_ret;
    //exit(ret);
    if (dbp != NULL)
    dbp->close(dbp, 0);
    //.........close evn
    //........When you are done with an environment, you must close it.
    //........Before you close an environment, make sure you close any opened databases
    if (myEnv != NULL)
    myEnv->close(myEnv, 0);

    return 0;
}
//g++ -g -I./ test_db_1.cpp -o ./bin/test_db_1 -L./ -lpthread -pthread -lrt -ldb