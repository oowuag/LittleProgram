#include <sys/types.h>
#include <db.h> 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* DB�ĺ���ִ����ɺ󣬷���0����ɹ�������ʧ�� */
void print_error(int ret)
{
    if(ret != 0)
        printf("ERROR: %s\n",db_strerror(ret));
}
/* ���ݽṹDBT��ʹ��ǰ��Ӧ���ȳ�ʼ������������ͨ��������ʱ����������  */
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
    /* ���ȴ������ݿ��� */
    ret = db_create(&dbp, NULL, 0);
    print_error(ret);

    /* �������ݿ��־ */
    flags = DB_CREATE;    
    /* ����һ����Ϊsingle.db�����ݿ⣬ʹ��B+�������㷨�����δ�����ʾ�Լ��������͵Ĵ��� */
    ret = dbp->open(dbp, NULL, "single.db", NULL, DB_BTREE, flags, 0); 
    print_error(ret);

    init_DBT(&key, &data);
    /* �ֱ�Թؼ��ֺ����ݸ�ֵ�͹涨���� */
    key.data = fruit;
    key.size = strlen(fruit) + 1;
    data.data = &number;
    data.size = sizeof(int);
    /* �Ѽ�¼д�����ݿ��У��������ǹؼ�����ͬ�ļ�¼ */
    ret = dbp->put(dbp, NULL, &key, &data,DB_NOOVERWRITE); 
    print_error(ret);
    /* �ֶ��ѻ����е�����ˢ�µ�Ӳ���ļ��У�ʵ�����ڹر����ݿ�ʱ�����ݻᱻ�Զ�ˢ�� */
    dbp->sync(dbp, 0);

    init_DBT(&key, &data);
    key.data = fruit;
    key.size = strlen(fruit) + 1;
    /* �����ݿ��в�ѯ�ؼ���Ϊapple�ļ�¼ */
    ret = dbp->get(dbp, NULL, &key, &data, 0);
    print_error(ret);
    /* �ر�Ҫע�����ݽṹDBT���ֶ�dataΪvoid *�ͣ������ڶ�data��ֵ��ȡֵʱ��Ҫ����Ҫ������ת���� */
    printf("The number = %d\n", *(int*)(data.data));
    if(dbp != NULL)
        dbp->close(dbp, 0);

    // ------------------------------------- complex.db
    ret = db_create(&dbp, NULL, 0);
    print_error(ret);
    flags = DB_CREATE;    

    /* ����һ����Ϊcomplex.db�����ݿ⣬ʹ��HASH�����㷨�����δ�����ʾ�Ը������ݽṹ�Ĵ��� */
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