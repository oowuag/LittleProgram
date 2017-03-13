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
 * @file NCBtPbObject.h
 * @brief Declaration file of class NCBtPbObject.
 */

#ifndef NCBTPBOBJECT_H
#define NCBTPBOBJECT_H
#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

//#include <nceventsys/NCRunnable.h>
#include "NCRefBase.h"
#include "NCString.h"
//#include "NCBtPbDef.h"


#define DECLARE_SINGLETON_REF(CLASS_NAME)    \
static CLASS_NAME* instance() \
{   \
    NCAutoSync cSync(s_syncObj);  \
    if (NULL == s_instance) { \
        s_instance = new CLASS_NAME();  \
        s_instance->forceIncStrong(s_instance); \
    }   \
    return s_instance;    \
}   \
\
static void destroy() \
{   \
    NCAutoSync cSync(s_syncObj);  \
    if (NULL != s_instance) { \
        s_instance->decStrong(s_instance); \
        s_instance = NULL;    \
    }   \
}   \
\
protected:   \
static CLASS_NAME* s_instance;    \
static NCSyncObj s_syncObj;

#define DEFINE_SINGLETON_REF(CLASS_NAME) \
    CLASS_NAME* CLASS_NAME::s_instance = NULL; \
    NCSyncObj CLASS_NAME::s_syncObj;

namespace nutshell
{
    class NCBtPbObject;
    class NCBtPbEvent;
    class NCBtPbNode;
    class NCBtPbEventManager;
    class NCBtPbQueuingThread;
    class NCBtSpThread;
    class NCBtTaskBase;


    enum NCBT_PB_QTHREAD_PRIORITY_LEVEL
    {
        PRIORITY_LEVEL_HIGH = 0,
        PRIORITY_LEVEL_LOW,
        PRIORITY_NUM
    };

    /**
     * class comment:TBD
     *
     * (TBD:description)
     *
     * @attention   TBD
     */
    class NCBtPbObject : public NCRefBase
    {
    public:
        NCBtPbObject();
        virtual ~NCBtPbObject();

        virtual void printInfo()
        {
        }

        virtual VOID onTrigger(const NCString& trigger, INT arg1, INT arg2)
        {
        }

        static VOID sendTrigger(const NCString& triggerName, INT arg1 = 0, INT arg2 = 0);

        static VOID sendProcEvent(const NCString& space,
                                  const NCString& identity,
                                  INT arg1 = 0,
                                  INT arg2 = 0,
                                  VOID* buff = NULL,
                                  size_t bufSize = 0);

        static VOID sendProcEvent(const NCString& space,
                                  const NCString& identity,
                                  INT arg1 = 0,
                                  INT arg2 = 0,
                                  const std::string& proto = "");
    private:
        NCBtPbObject(const NCBtPbObject& obj);
        NCBtPbObject& operator=(const NCBtPbObject& obj);
    };

    /**
     * class comment:TBD
     *
     * (TBD:description)
     *
     * @attention   TBD
     */
    class NCBtPbNode : public NCBtPbObject
    {
    public:
        friend class NCBtPbTimerRunnable;
        friend class NCBtPbTimerEvent;

        NCBtPbNode();
        virtual ~NCBtPbNode();

        virtual VOID onEvent(const NCBtPbEvent& ev);

        virtual VOID onTimer(INT id)
        {
        }

        VOID setTimer(INT id, INT ms);
        VOID killTimer(INT id);

        static VOID postEvent(NCBtPbEvent* ev, INT proi = PRIORITY_LEVEL_LOW);

        static VOID pushEvent(NCBtPbEvent* ev, INT proi = PRIORITY_LEVEL_LOW);

        static VOID postCmdTask(NCBtTaskBase* task);

        static VOID setDispatcher(NCBtPbQueuingThread* dispatcher);
        static VOID setEventManager(NCBtPbEventManager* eventManager);
        static VOID setCmdTaskThread(NCBtSpThread* taskThread);
    private:
        static NCBtPbQueuingThread* s_dispatcher;
        static NCBtPbEventManager* s_eventManager;
        static NCBtSpThread* s_taskThread;

        //std::map<INT, NCRunnableHolder> m_mapId2Timer;

        VOID actionTimer(INT id);
        NCBtPbNode(const NCBtPbNode& obj);
        NCBtPbNode& operator=(const NCBtPbNode& obj);
    };

    typedef ncsp<NCBtPbNode>::sp NCBtPbNodeSp;
    typedef ncsp<NCBtPbNode>::wp NCBtPbNodeWp;

    /**
     * class comment:TBD
     *
     * (TBD:description)
     *
     * @attention   TBD
     */
    class NCBtPbEvent : public NCBtPbObject
    {
    public:
        NCBtPbEvent(INT evId, NCBtPbNodeWp tgt = NULL, NC_BOOL autoRm = NC_FALSE);
        virtual ~NCBtPbEvent();

        INT getEventId() const
        {
            return m_evId;
        }

        NC_BOOL autoRemove() const
        {
            return m_autoRemove;
        }

        virtual VOID doAction(NCBtPbQueuingThread* thread);

        static VOID setDefaultTarget(NCBtPbNode* tgt);

    protected:
        INT m_evId;
        NCBtPbNodeWp m_target;
        NC_BOOL m_autoRemove;
        static NCBtPbNode* s_target_def;

        NCBtPbEvent(const NCBtPbEvent& obj);
        NCBtPbEvent& operator=(const NCBtPbEvent& obj);
    };
}  // end of namespace nutshell

#endif /* NCBTPBOBJECT_H */
/* EOF */
