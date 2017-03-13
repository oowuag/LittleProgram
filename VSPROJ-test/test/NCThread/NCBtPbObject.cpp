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

#include "NCBtPbObject.h"
//#include <nceventsys/NCEvSysContext.h>
//#include "NCBtLog.h"
//#include "NCBtPbEventManager.h"
#include "NCBtPbQueuingThread.h"
#include "NCBtSpThread.h"

//#include "NCBtPbModule.h"
//#include "NCBtSettingDef.h"

namespace nutshell
{
    // NCBtPbObject
    NCBtPbObject::NCBtPbObject()
    {
        // TODO Auto-generated constructor stub

    }

    NCBtPbObject::~NCBtPbObject()
    {
        // TODO Auto-generated destructor stub
    }

    VOID NCBtPbObject::sendTrigger(const NCString& triggerName, INT arg1, INT arg2)
    {

    }

    VOID NCBtPbObject::sendProcEvent(const NCString& space,
                                     const NCString& identity,
                                     INT arg1,
                                     INT arg2,
                                     VOID* buff,
                                     size_t bufSize)
    {

    }

    VOID NCBtPbObject::sendProcEvent(const NCString& space, const NCString& identity, INT arg1, INT arg2, const std::string& proto)
    {
        printf("send Proc Event:%s %s arg1:%d arg2:%d", space.getString(), identity.getString(), arg1, arg2);
    }

    // NCBtPbEvent
    NCBtPbNode* NCBtPbEvent::s_target_def = NULL;

    NCBtPbEvent::NCBtPbEvent(INT evId, NCBtPbNodeWp tgt, NC_BOOL autoRm)
            : m_evId(evId),
              m_target(tgt),
              m_autoRemove(autoRm)
    {

    }

    NCBtPbEvent::~NCBtPbEvent()
    {

    }

    VOID NCBtPbEvent::doAction(NCBtPbQueuingThread* thread)
    {
        NCBtPbNodeSp sp = m_target.promote();
        if (sp != NULL) {
            sp->onEvent(*this);
        }
        else if (s_target_def) {
            s_target_def->onEvent(*this);
        }
        else {

        }
    }

    VOID NCBtPbEvent::setDefaultTarget(NCBtPbNode* tgt)
    {
        s_target_def = tgt;
    }



    // NCBtPbNode
    NCBtPbQueuingThread* NCBtPbNode::s_dispatcher = NULL;
    NCBtPbEventManager* NCBtPbNode::s_eventManager = NULL;
    NCBtSpThread* NCBtPbNode::s_taskThread = NULL;

    VOID NCBtPbNode::setDispatcher(NCBtPbQueuingThread* dispatcher)
    {
        s_dispatcher = dispatcher;
    }

    VOID NCBtPbNode::setEventManager(NCBtPbEventManager* eventManager)
    {
        s_eventManager = eventManager;
    }

    VOID NCBtPbNode::setCmdTaskThread(NCBtSpThread* taskThread)
    {
        s_taskThread = taskThread;
    }

    NCBtPbNode::NCBtPbNode()
    {

    }

    NCBtPbNode::~NCBtPbNode()
    {
        //m_mapId2Timer.clear();
    }

    VOID NCBtPbNode::actionTimer(INT id)
    {
        //m_mapId2Timer.erase(id);
        onTimer(id);
    }

    VOID NCBtPbNode::setTimer(INT id, INT ms)
    {

    }

    VOID NCBtPbNode::killTimer(INT id)
    {

    }

    VOID NCBtPbNode::onEvent(const NCBtPbEvent& ev)
    {

    }

    VOID NCBtPbNode::postEvent(NCBtPbEvent* ev, INT proi)
    {
        if (!ev) {
            return;
        }

        if (s_dispatcher) {
            // NCBT_PB_LOG_D("postEvent: id: %08x", ev->getEventId());
            s_dispatcher->postRequest(ev, proi);
        }
        else {
            printf("post event failed");
            delete ev;
        }
    }

    VOID NCBtPbNode::pushEvent(NCBtPbEvent* ev, INT proi)
    {
        if (!ev) {
            return;
        }

        if (s_dispatcher) {
            // NCBT_PB_LOG_D("postEvent: id: %08x", ev->getEventId());
            s_dispatcher->pushRequest(ev, proi);
        }
        else {
            printf("post event failed");
            delete ev;
        }
    }

    VOID NCBtPbNode::postCmdTask(NCBtTaskBase* task)
    {
        if (!task) {
            return;
        }

        if (s_taskThread) {
            s_taskThread->postRequest(task);
        }
        else {
            printf("post cmd task failed");
            delete task;
        }
    }

}  // end of namespace nutshell
/* EOF */
