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

#include "NCBtPbQueuingThread.h"
#include "NCBtPbObject.h"
//#include "NCBtLog.h"

namespace nutshell
{
    NCBtPbQueuingThread::NCBtPbQueuingThread(const NCCHAR* thread_name)
            : m_threadName(thread_name),
              m_awaked(NC_FALSE)
    {
    }

    NCBtPbQueuingThread::~NCBtPbQueuingThread()
    {
        m_syncObj.syncStart();
        for (int i = 0; i < PRIORITY_NUM; ++i) {
            for (std::list<NCBtPbEvent*>::iterator it = m_reqList[i].begin(); it != m_reqList[i].end();) {
                delete *it;
                it = m_reqList[i].erase(it);
            }
        }
        m_syncObj.syncEnd();
    }

    VOID NCBtPbQueuingThread::onAwake()
    {
        printf("");
        m_awaked = NC_TRUE;
        notify();
    }

    VOID NCBtPbQueuingThread::onStop()
    {
        m_awaked = NC_FALSE;

        m_syncObj.syncStart();
        for (int i = 0; i < PRIORITY_NUM; ++i) {
            for (std::list<NCBtPbEvent*>::iterator it = m_reqList[i].begin(); it != m_reqList[i].end();) {
                NCBtPbEvent* ev = *it;
                if (ev && ev->autoRemove()) {
                    delete ev;
                    it = m_reqList[i].erase(it);
                }
                else {
                    ++it;
                }
            }
        }
        m_syncObj.syncEnd();
    }

    VOID NCBtPbQueuingThread::postRequest(NCBtPbEvent* rep, INT prio)
    {
        if (prio < 0 || prio >= PRIORITY_NUM) {
            delete rep;
            return;
        }

        m_syncObj.syncStart();
        startThread (m_threadName);
        m_reqList[prio].push_back(rep);
        m_syncObj.syncEnd();
        notify();
    }

    VOID NCBtPbQueuingThread::pushRequest(NCBtPbEvent* rep, INT prio)
    {
        if (prio < 0 || prio >= PRIORITY_NUM) {
            delete rep;
            return;
        }

        m_syncObj.syncStart();
        startThread (m_threadName);
        m_reqList[prio].push_front(rep);
        m_syncObj.syncEnd();
        notify();
    }

    NCBtPbEvent* NCBtPbQueuingThread::popRequest()
    {
        NCBtPbEvent* req = NULL;
        m_syncObj.syncStart();
        for (INT i = 0; i < PRIORITY_NUM; ++i) {
            if (m_reqList[i].empty()) {
                continue;
            }

            req = m_reqList[i].front();
            m_reqList[i].pop_front();
            break;
        }
        m_syncObj.syncEnd();
        return req;
    }

    VOID NCBtPbQueuingThread::run()
    {
        while (!checkQuit()) {
            if (m_awaked) {
                NCBtPbEvent* req = popRequest();
                while (NULL != req) {
                    req->doAction(this);
                    delete req;

                    if (!m_awaked) {
                        break;
                    }
                    req = popRequest();
                }
            }
            wait();
        }
    }
}  // end of namespace nutshell
/* EOF */
