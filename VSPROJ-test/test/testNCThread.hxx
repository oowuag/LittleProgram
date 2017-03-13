#include <iostream>
#include "NCThread/NCQueuingThread.h"
#include "NCThread/NCBtPbObject.h"
#include "NCThread/NCBtPbQueuingThread.h"

using namespace std;

namespace nutshell
{
    class BtPhonebookReq
    {
    public:
        BtPhonebookReq() {}
        virtual ~BtPhonebookReq() {}
        virtual VOID doAction(NCQueuingThread<BtPhonebookReq>* thread) 
        {
            printf("doAction\n");
        }

    private:
        BtPhonebookReq(const BtPhonebookReq&);
        BtPhonebookReq& operator=(const BtPhonebookReq&);
    };
}

int main()
{
    using namespace nutshell;

    nutshell::NCQueuingThread<nutshell::BtPhonebookReq> m_resolverThread("BTPB_REQ_THREAD_NAME");
    m_resolverThread.startThread();
    m_resolverThread.postRequest(new BtPhonebookReq());


    int x = 0;
    while(1)
    {
        Sleep(5);
        x++;
    }


    return 0;
}
