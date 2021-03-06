#include <stdio.h>
#include <Windows.h>

struct Node
{
   Node* next;
   void* data;
};

static Node* tail = NULL;
static Node* head = NULL;


bool cas(Node **dest, Node *old, Node *newval)
{
    InterlockedCompareExchange((volatile LONG *)dest, (LONG)newval, (LONG)old);
    return true;
}

//bool cas(Node **accum, Node *dest, Node *newval)
//{
//  if ( *accum == dest ) {
//      dest = newval;
//      return true;
//  }
//  return false;
//}

void Push(void* data)
 {
    Node* old_tail;
    Node* node = new Node();
    node->data = data;
    
    do 
    {
       old_tail = tail;
 
       if (!cas(&tail, old_tail, node))
           continue;
 
       if (old_tail)
          old_tail->next = node;
       else
          cas(&head, NULL, node);
 
       break;
 
    }while (1);
    
 }


struct aa
{
       char b;
       int a;
       double c;
};

class NCTimer
{
public:
    void restart()
    {
        printf("NCTimer::restart()\n");
    }
};

class NewNCTimer : public NCTimer
{
public:
    virtual void restart()
    {
        printf("NewNCTimer::restart()\n");
    }
};

        //DWORD firtTime = 0;
        //firtTime += first.m_byTickTime[2] << 16;
        //firtTime += first.m_byTickTime[1] << 8;
        //firtTime += first.m_byTickTime[0];

        //DWORD secondTime = 0;
        //secondTime += second.m_byTickTime[2] << 16;
        //secondTime += second.m_byTickTime[1] << 8;
        //secondTime += second.m_byTickTime[0];

        //if (firtTime < secondTime) {
        //    return true;
        //}
        //else {
        //    return false;
        //}

int main()
{
    //Push(NULL);
    //Push(NULL);
    //Push(NULL);

    LONG x = 0;
    x = 8 << 8;
    printf("%d\n", x);

    //char xx[10] = "012345678";
    //printf("%p\n",xx);
    //printf("%p\n",&xx);
    //memset(&xx, 0, sizeof(xx));

    aa a;
    memset(&a, 0, sizeof(a));
    printf("%p\n",a);
    printf("%p\n",&a);


    char szTemp[] = "0123456789ABCDEF0123456789ABCDEF";
    printf("size:%d\n", sizeof(szTemp));


    NewNCTimer ntimer;
    NCTimer *pTimer = &ntimer;
    pTimer->restart();


    return 0;
}