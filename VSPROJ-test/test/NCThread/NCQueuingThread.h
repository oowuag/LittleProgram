/**
 * Copyright @ 2013 - 2014 Suntec Software(Shanghai) Co., Ltd.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted except as agreed by
 * Suntec Software(Shanghai) Co., Ltd.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */


/**
 * @file NCQueuingThread.h
 * @brief
 *
 */

#ifndef NCQUEUINGTHREAD_H
#define NCQUEUINGTHREAD_H

#ifndef NCTYPESDEFINE_H
#   include "NCTypesDefine.h"
#endif
#ifndef NCTHREAD_H
#   include "NCThread.h"
#endif
#ifndef NCSYNCOBJ_H
#   include "NCSyncObj.h"
#endif
#ifndef NCLIST_H
#   include "NCList.h"
#endif
#ifndef NCSTRING_H
#   include "NCString.h"
#endif


namespace nutshell
{
    /**
    * @brief
    *
    * @class NCQueuingThread
    *
    * @code
    *
    * class MyReq
    * {
    * public:
    *     VOID doAction(NCQueuingThread<MyReq>* pThread)
    *     {
    *     //do something
    *     }
    * };
    *
    * class CSample {
    *     NCQueuingThread<MyReq> mThread;
    * public:
    *     CSample() : mThread(NCTEXT("SAMPLE_TRHEAD_NAME")) {}
    *     VOID addReq()
    *     {
    *         MyReq* req = new MyReq();
    *         mThread.postRequest(req);
    *     }
    * };
    *
    * class CSample
    * {
    *     class MyThread : public NCQueuingThread<MyReq>
    *     {
    *     public:
    *         MyThread() : NCQueuingThread<MyReq>(NCTEXT("SAMPLE_TRHEAD_NAME")) {}
    *     };
    *
    *     MyThread mThread;
    * public:
    *     VOID addReq()
    *     {
    *         MyReq* req = new MyReq();
    *         mThread.postRequest(req);
    *     }
    * };
    * @endcode
    */
    template <typename REQ>
    class NCQueuingThread : public NCThread
    {
    public:
        NCQueuingThread(const NCCHAR* thread_name) : mThreadName(thread_name)
        {
        }

        virtual ~NCQueuingThread()
        {
            mReqList.clearData();
        }

        virtual VOID postRequest(REQ* rep);
        virtual VOID pushRequest(REQ *rep);

    protected:
        virtual VOID run();
        virtual REQ* popRequest();

        NCList<REQ> mReqList;
        NCSyncObj mSyncObj;
        NCString mThreadName;
    };

    template <typename REQ>
    VOID NCQueuingThread<REQ>::postRequest(REQ* rep)
    {
        mSyncObj.syncStart();
        startThread(mThreadName);
        mReqList.append(rep);
        mSyncObj.syncEnd();
        notify();
    }

    template <typename REQ>
    VOID NCQueuingThread<REQ>::pushRequest(REQ* rep)
    {
        mSyncObj.syncStart();
        startThread(mThreadName);
        mReqList.push(rep);
        mSyncObj.syncEnd();
        notify();
    }

    template <typename REQ>
    REQ* NCQueuingThread<REQ>::popRequest()
    {
        mSyncObj.syncStart();
        REQ* req = mReqList.pop();
        mSyncObj.syncEnd();
        return req;
    }

    template <typename REQ>
    VOID NCQueuingThread<REQ>::run()
    {
        REQ* req = NULL;
        while (!checkQuit()) {
            req = popRequest();
            while (NULL != req) {
                req->doAction(this);
                delete req;
                req = popRequest();
            }
            wait();
        }
    }
}

#endif /* NCQUEUINGTHREAD_H */
/* EOF */
