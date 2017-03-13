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
 * @file NCSpThread.h
 * @brief
 *
 */
#ifndef NCSPTHREAD_H
#define NCSPTHREAD_H

#ifndef NCREFBASE_H
#   include "ncore/NCRefBase.h"
#endif
#ifndef NCTHREAD_H
#   include "ncore/NCThread.h"
#endif
#ifndef NCTIMER_H
#   include "ncore/NCTimer.h"
#endif
#ifndef NCSYNCOBJ_H
#   include "ncore/NCSyncObj.h"
#endif
#ifndef NCWAITOBJ_H
#   include "ncore/NCWaitObj.h"
#endif
#ifndef NCLIST_H
#   include "ncore/NCList.h"
#endif
#ifndef NCSTRING_H
#   include "ncore/NCString.h"
#endif
//#ifndef ANDROID_VECTOR_H
//#   include <utils/Vector.h>
//#endif
//#ifndef NCDEBUG_H
//#   include "ncore/NCDebug.h"
//#endif

//#ifndef NCATOMIC_H
//#   include "ncore/NCAtomic.h"
//#endif

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

namespace nutshell
{

#define int32_t INT32
#define uint32_t UINT32

    extern inline int android_atomic_cas(int32_t old_value, int32_t new_value,
        volatile int32_t *ptr)
    {
        return InterlockedCompareExchange((volatile LONG *)ptr, new_value, old_value);
    }

    extern inline int32_t android_atomic_or(int32_t value, volatile int32_t *ptr)
    {
        int32_t prev = *ptr;
        android_atomic_cas(prev, prev | value, ptr);
        return prev;
    }

    extern inline int32_t android_atomic_and(int32_t value, volatile int32_t *ptr)
    {
        int32_t prev = *ptr;
        android_atomic_cas(prev, prev & value, ptr);
        return prev;
    }

    extern inline INT32 nc_atomic_or(INT32 value, volatile INT32* addr)
    {
        return android_atomic_or(value, addr);
    }

    extern inline INT32 nc_atomic_and(INT32 value, volatile INT32* addr)
    {
        return android_atomic_and(value, addr);
    }

    /**
    * brief sample code
    *
    * @code
    * class MyReqBase : public NCRefBase { // must overried from NCRefBase!!!
    * public:
    *     MyReqBase() : mThread(NULL) {}
    *     virtual ~MyReqBase() {}

    *     // needed by template!!!
    *     virtual VOID doAction(NCSpThread<MyReqBase>* pThread) = 0;
    *     virtual VOID doTimeout() = 0;

    *     virtual VOID confirm() = 0;// add for sample.
    * protected:
    *     enum {
    *         REQ_TIMEOUT = 0,
    *         REQ_CONFIRMED,
    *     };
    *     NCSpThread<MyReqBase>* mThread;
    *     NCSyncObj mSyncState;
    *     volatile INT mState;
    * };

    * class MyReq : public MyReqBase {
    * public:
    *     VOID doAction(NCSpThread<MyReqBase>* pThread)
    *     {
    *         mThread = pThread;
    *         //do something, then wait
    *         pThread->waitRequest(5000); // wait 5s for notifyRequest
    *         if (REQ_TIMEOUT == mState) {
    *             // timeout.
    *         }
    *         else if (REQ_CONFIRMED == mState) {
    *             // confirmed.
    *         }
    *         else {
    *             // thread stopped.
    *         }
    *         mThread = NULL;
    *     }
    *     VOID doTimeout()
    *     {
    *         {// timeout
    *             NCAutoSync _sync(mSyncState);
    *             if (REQ_CONFIRMED == mState) return;
    *             mState = REQ_TIMEOUT;
    *         }
    *         // do something, then wakeup
    *         if (mThread) mThread->notifyRequest();
    *     }
    *     VOID confirm()
    *     {
    *         {// cofirmed
    *             NCAutoSync _sync(mSyncState);
    *             if (REQ_TIMEOUT == mState) return;
    *             mState = REQ_CONFIRMED;
    *         }
    *         // do something, then wakeup
    *         if (mThread) mThread->notifyRequest();
    *     }
    * };
    * class CSample {
    *     NCSpThread<MyReqBase> mThread;
    * public:
    *     CSample() : mThread(NCTEXT("SAMPLE_TRHEAD_NAME")) {}
    *     VOID addReq()
    *     {
    *         MyReq* req = new MyReq();
    *         mThread.postRequest(req);
    *     }
    *     VOID confirm()
    *     {
    *         NCSpThread<MyReqBase>::sp req;
    *         mThread.curRequest(req);
    *         if (req != NULL) req->confirm();
    *     }
    * };
    * @endcode
    */
    template <typename REQ>
    class NCSpThread : public NCThread
    {
    public:
        NCSpThread(const NCCHAR* thread_name);
        virtual ~NCSpThread();

        VOID postRequest(REQ* req);
        VOID pushRequest(REQ* req);

        typedef android::sp<REQ> sp;
        typedef android::wp<REQ> wp;
        typedef std::vector<sp> Vector;

        VOID curRequest(sp& out)
        {
            mSyncObj.syncStart();
            out = curReq;
            mSyncObj.syncEnd();
        }

        wp currRequest()
        {
            return curReq;
        }

        ///< wait/notify in Requeset::doAction()
        NC_BOOL isReqAction();
        VOID notifyRequest();
        VOID resetNotify();
        NC_BOOL waitRequest(DWORD msec = INFINITE);

        ///< override for wakeup mReqWaitObj
        virtual VOID startThread(const NCCHAR *name = NCTEXT("Unknown Thread"));
        virtual NC_BOOL stopThread(DWORD msec = INFINITE);
        virtual NC_BOOL terminate();

        ///< stop thread after the request::doAction
        virtual NC_BOOL stopThread(REQ* req, NC_BOOL clear, DWORD msec = INFINITE);

    protected:
        NC_BOOL popRequest();
        VOID clearRequest();
        NC_BOOL doStopReq();

        VOID toWait()
        {
            mReqWaitObj.wait();
        }

        VOID toNotify()
        {
            mReqWaitObj.notify();
        }

    protected:
        sp curReq;  // for current action.
        sp stopReq; // for quit action.
        Vector mReqList;
        NCSyncObj mSyncObj;
        NCString mThreadName;

    private:
        ///< override for protect NCThread::waitobj
        virtual VOID notify()
        {
            NCThread::notify();
        }

        virtual NC_BOOL wait(DWORD msec = INFINITE)
        {
            return NCThread::wait(msec);
        }

        virtual VOID run();

        enum
        {
            REQ_ACTION = 0x00000010, // REQ.doAction
        };

        /**
         * @brief
         *
         * @class ReqTimer
         */
        class ReqTimer : public NCTimer
        {
        public:
            ReqTimer(LONG msec, NC_BOOL iter, sp req)
            : NCTimer(msec, iter), curReq(req)
            {
            }

        private:
            sp curReq;
            virtual VOID OnTimer()
            {
                if (curReq != NULL) {
                    curReq->doTimeout();
                }
            }
        };

    private:
        NCWaitObj mReqWaitObj;
        volatile INT mReqState;
        NC_BOOL mQuit;
    };

    template <typename REQ>
    NCSpThread<REQ>::NCSpThread(const NCCHAR* thread_name)
    : mThreadName(thread_name)
    , mReqState(0)
    , mQuit(NC_FALSE)
    {
        //NC_PRINTF(("NCSpThread[%s] construct.", thread_name));
    }

    template <typename REQ>
    NCSpThread<REQ>::~NCSpThread()
    {
        clearRequest();
        stopThread();
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::postRequest(REQ* req)
    {
        mSyncObj.syncStart();
        startThread(mThreadName);
        mReqList.add(req);
        mSyncObj.syncEnd();
        toNotify();
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::pushRequest(REQ* req)
    {
        mSyncObj.syncStart();
        startThread(mThreadName);
        mReqList.push(req);
        mSyncObj.syncEnd();
        toNotify();
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::popRequest()
    {
        mSyncObj.syncStart();
        if (!mReqList.empty()) {
            curReq = mReqList[0];
            mReqList.erase(mReqList.begin());
            mSyncObj.syncEnd();
            return NC_TRUE;
        }
        curReq = NULL; // release the current request.
        mSyncObj.syncEnd();
        return NC_FALSE;
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::clearRequest()
    {
        mSyncObj.syncStart();
        mReqList.clear();
        mSyncObj.syncEnd();
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::isReqAction()
    {
        return (mReqState & REQ_ACTION) ? NC_TRUE : NC_FALSE;
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::notifyRequest()
    {
        notify();
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::resetNotify()
    {
        reset();
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::waitRequest(DWORD msec)
    {
        if (!isReqAction()) {
            return NC_FALSE;
        }

        if (INFINITE != msec) { // start timer
            ReqTimer timer(msec, false, curReq);
            NC_BOOL res = timer.start();
            if (!res) {
                printf("NCSpThread waitRequest(%u) to start timer failed.", msec);
            }
            wait();
        }
        else {
            wait();
        }
        return NC_TRUE;
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::startThread(const NCCHAR *name)
    {
        mQuit = NC_FALSE;
        return NCThread::startThread(name);
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::stopThread(DWORD msec)
    {
        mQuit = NC_TRUE;
        toNotify(); // notify mReqWaitObj
        return NCThread::stopThread(msec);
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::stopThread(REQ* req, NC_BOOL clear, DWORD msec)
    {
        mQuit = NC_TRUE;
        stopReq = req;
        if (clear) { // clear request but current.
            clearRequest();
        }
        toNotify();
        return join(msec);
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::terminate()
    {
        mQuit = NC_TRUE;
        toNotify(); // notify mReqWaitObj
        return NCThread::terminate();
    }

    template <typename REQ>
    NC_BOOL NCSpThread<REQ>::doStopReq()
    {
        if (stopReq != NULL) {
            curReq = stopReq;
            nc_atomic_or(REQ_ACTION, &mReqState);
            resetNotify(); // reset wait obj, before action...
            curReq->doAction(this);
            nc_atomic_and(~REQ_ACTION, &mReqState);
            curReq = NULL;
            stopReq = NULL;

            // stop thread.
            toNotify();
            return stopThread();
        }
        return NC_FALSE;
    }

    template <typename REQ>
    VOID NCSpThread<REQ>::run()
    {
        while (!checkQuit() && !mQuit) {
            while (!doStopReq() && popRequest()) {
                nc_atomic_or(REQ_ACTION, &mReqState);
                resetNotify(); // reset wait obj, before action...
                curReq->doAction(this);
                nc_atomic_and(~REQ_ACTION, &mReqState);
            }
            toWait();
            doStopReq();
        }
    }

}
#endif /* NCSPTHREAD_H */
/* EOF */
