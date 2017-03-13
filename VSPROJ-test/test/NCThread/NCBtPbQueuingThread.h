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
 * @file NCBtPbQueuingThread.h
 * @brief
 *
 */

#ifndef NCBTPBQUEUINGTHREAD_H
#define NCBTPBQUEUINGTHREAD_H
#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <list>
//#include "ncore/NCGlobalAPI.h"
#include "NCTypesDefine.h"
#include "NCThread.h"
#include "NCSyncObj.h"
#include "NCString.h"
//#include "NCBtPbDef.h"
#include "NCBtPbObject.h"

namespace nutshell
{
    class NCBtPbEvent;


    /**
     * @class NCBtPbQueuingThread
     *
     * @brief Interface to operate the NCBtPbQueuingThread
     *
     */
    class NCBtPbQueuingThread : public NCThread
    {
    public:
        explicit NCBtPbQueuingThread(const NCCHAR* thread_name);
        virtual ~NCBtPbQueuingThread();

        virtual VOID onAwake();
        virtual VOID onStop();
        virtual VOID postRequest(NCBtPbEvent* rep, INT prio = PRIORITY_LEVEL_LOW);
        virtual VOID pushRequest(NCBtPbEvent* rep, INT prio = PRIORITY_LEVEL_LOW);

    protected:
        virtual VOID run();
        virtual NCBtPbEvent* popRequest();

        std::list<NCBtPbEvent*> m_reqList[PRIORITY_NUM];
        mutable NCSyncObj m_syncObj;
        NCString m_threadName;
        volatile NC_BOOL m_awaked;
    };
}

#endif /* NCBTPBQUEUINGTHREAD_H */
/* EOF */
