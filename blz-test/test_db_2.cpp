#include <sys/types.h>
#include <db.h> 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* DB的函数执行完成后，返回0代表成功，否则失败 */
void print_error(int ret)
{
    if(ret != 0)
        printf("ERROR: %s\n",db_strerror(ret));
}
/* 数据结构DBT在使用前，应首先初始化，否则编译可通过但运行时报参数错误  */
void init_DBT(DBT * key, DBT * data)
{
    memset(key, 0, sizeof(DBT));
    memset(data, 0, sizeof(DBT));
}

int rec_display(DB *dbp);

int main(void)
{
    DB *dbp;           
    DBT key, data;
    u_int32_t flags;  
    int ret;
    char *fruit = (char *)"apple";
    int number = 15;

    typedef struct customer
    {
        int  c_id;
        char name[10];
        char address[20];
        int  age;
    } CUSTOMER;
    CUSTOMER cust; 
    int key_cust_c_id = 1;
    cust.c_id = 1;
    strncpy(cust.name, "javer", 9);
    strncpy(cust.address, "chengdu", 19);
    cust.age = 32;

    // ------------------------------------- single.db
    /* 首先创建数据库句柄 */
    ret = db_create(&dbp, NULL, 0);
    print_error(ret);

    /* 创建数据库标志 */
    flags = DB_CREATE;    
    /* 创建一个名为single.db的数据库，使用B+树访问算法，本段代码演示对简单数据类型的处理 */
    ret = dbp->open(dbp, NULL, "single.db", NULL, DB_BTREE, flags, 0); 
    print_error(ret);

    init_DBT(&key, &data);
    /* 分别对关键字和数据赋值和规定长度 */
    key.data = fruit;
    key.size = strlen(fruit) + 1;
    data.data = &number;
    data.size = sizeof(int);
    /* 把记录写入数据库中，不允许覆盖关键字相同的记录 */
    ret = dbp->put(dbp, NULL, &key, &data,DB_NOOVERWRITE); 
    print_error(ret);
    /* 手动把缓存中的数据刷新到硬盘文件中，实际上在关闭数据库时，数据会被自动刷新 */
    dbp->sync(dbp, 0);

    init_DBT(&key, &data);
    key.data = fruit;
    key.size = strlen(fruit) + 1;
    /* 从数据库中查询关键字为apple的记录 */
    ret = dbp->get(dbp, NULL, &key, &data, 0);
    print_error(ret);
    /* 特别要注意数据结构DBT的字段data为void *型，所以在对data赋值和取值时，要做必要的类型转换。 */
    printf("The number = %d\n", *(int*)(data.data));
    if(dbp != NULL)
        dbp->close(dbp, 0);

    // ------------------------------------- complex.db
    ret = db_create(&dbp, NULL, 0);
    print_error(ret);
    flags = DB_CREATE;    

    /* 创建一个名为complex.db的数据库，使用HASH访问算法，本段代码演示对复杂数据结构的处理 */
    ret = dbp->open(dbp, NULL, "complex.db", NULL, DB_HASH, flags, 0); 
    print_error(ret);
    init_DBT(&key, &data);
    key.data = &(cust.c_id);
    key.size = sizeof(int);
    data.data = &cust;
    data.size = sizeof(CUSTOMER);
    ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE);
    print_error(ret);

    memset(&cust, 0, sizeof(CUSTOMER));

    key.size = sizeof(int);
    key.data = &key_cust_c_id;
    data.data = &cust;
    data.ulen = sizeof(CUSTOMER); 
    data.flags = DB_DBT_USERMEM;
    dbp->get(dbp, NULL, &key, &data, 0);
    print_error(ret);

    printf("c_id = %d name = %s address = %s age = %d\n", 
        cust.c_id, cust.name, cust.address, cust.age);

    rec_display(dbp);

    if(dbp != NULL)
        dbp->close(dbp, 0); 

    return 0;
}

//------------bulk
int rec_display(DB *dbp)
{
    DBC *dbcp;
    DBT key, data;
    size_t retklen, retdlen;
    void *retkey, *retdata;
    int ret, t_ret;
    void *p;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    /* Review the database in 5MB chunks. */
#define    BUFFER_LENGTH    (5 * 1024 * 1024)
    if ((data.data = malloc(BUFFER_LENGTH)) == NULL)
        return (-1);
    data.ulen = BUFFER_LENGTH;
    data.flags = DB_DBT_USERMEM;

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, DB_CURSOR_BULK)) 
        != 0) {
            dbp->err(dbp, ret, "DB->cursor");
            free(data.data);
            return (ret);
    }

    for (;;) {
        /*
         * Acquire the next set of key/data pairs.  This code 
         * does not handle single key/data pairs that won't fit 
         * in a BUFFER_LENGTH size buffer, instead returning 
         * DB_BUFFER_SMALL to our caller.
         */
        if ((ret = dbcp->get(dbcp,
            &key, &data, DB_MULTIPLE_KEY | DB_NEXT)) != 0) {
            if (ret != DB_NOTFOUND)
                dbp->err(dbp, ret, "DBcursor->get");
            break;
        }

        for (DB_MULTIPLE_INIT(p, &data);;) {
            DB_MULTIPLE_KEY_NEXT(p,
                &data, retkey, retklen, retdata, retdlen);
            if (p == NULL)
                break;
            printf("key: %.*s, data: %.*s\n",
                (int)retklen, (char *)retkey, (int)retdlen, 
                (char *)retdata);
        }
    }

    if ((t_ret = dbcp->close(dbcp)) != 0) {
        dbp->err(dbp, ret, "DBcursor->close");
        if (ret == 0)
            ret = t_ret;
    }

    free(data.data);

    return (ret);
}

//g++ -g -I./ test_db_2.cpp -o ./bin/test_db_2 -L./ -lpthread -pthread -lrt -ldb