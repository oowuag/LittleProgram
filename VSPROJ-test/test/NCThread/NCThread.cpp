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

#ifndef NCTHREAD_H
#   include "NCThread.h"
#endif


namespace nutshell
{

    VOID NCThread::ThreadProc(LPVOID p)
    {
        if(p == NULL) return ;
        ((NCThread*)p)->ExecRun();
    }

    NCThread::NCThread()
    {
        m_handle		= NULL;
        m_thread_id	= 0; 
        m_quit_flg	= FALSE; 
        m_szThreadName[0] = TEXT('\0');
        m_hevent          = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    NCThread::NCThread(NCThreadSystemIF* sp)
    {
    }

    NCThread::~NCThread()
    {
        stopThread();
        if(m_hevent) {
            ::CloseHandle(m_hevent);
        }
    }

    VOID NCThread::startThread(const NCCHAR* p)
    {
        if (m_handle)	return;	

        if (p == NULL) {
            p = "Unknown Thread";
        }
        lstrcpyn(m_szThreadName, p, NC_THREAD_NAME_LEN);
        m_szThreadName[NC_THREAD_NAME_LEN - 1] = TEXT('\0');

        DWORD tid = 0;
        m_quit_flg  = FALSE;
        m_handle = ::CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)ThreadProc, (LPVOID)this, CREATE_SUSPENDED, &tid);

        ::ResumeThread(m_handle);
    }

    NC_BOOL
    NCThread::stopThread(DWORD msec)
    {
        if (!m_handle) {
            return TRUE;
        }

        m_quit_flg = TRUE;
        notify();
        NC_BOOL ret = join(msec);
        return ret;
    }

    NC_BOOL
    NCThread::terminate()
    {
        if (!m_handle) {
            return TRUE;
        }

        HANDLE tmp_handle = m_handle;
        m_handle		= NULL;
        m_thread_id	= 0;

        BOOL ret  = ::TerminateThread(tmp_handle, 0);
        ::CloseHandle(tmp_handle);

        return ret;
    }

    NC_BOOL
    NCThread::join(DWORD msec)
    {
        if (!m_handle) {
            return TRUE;
        }

        switch(::WaitForSingleObject(m_handle, msec))
        {
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
        default:
            return FALSE;
        }
    }

    void NCThread::ExecRun()
    {
        m_thread_id = ::GetCurrentThreadId();

        run();

        ::CloseHandle(m_handle);
        m_handle = NULL;
        m_thread_id = 0;
    }

    NC_BOOL
    NCThread::wait(DWORD msec)
    {
        DWORD start_time = ::GetTickCount();

        while(1)
        {
            DWORD wait_time = 0;

            if (msec == INFINITE) {
                wait_time = INFINITE;
            }
            else {
                DWORD loss = ::GetTickCount() - start_time;
                wait_time = (msec > loss) ? (msec - loss) : 0;
            }

            switch(::WaitForSingleObject(&m_hevent, wait_time))
            {
            case WAIT_OBJECT_0:
                return TRUE;
            case WAIT_TIMEOUT:
                return FALSE;
            default:
                break;
            }
        }
    }

    VOID NCThread::notify()
    {
        ::SetEvent(m_hevent);
    }

    VOID NCThread::reset()
    {
        ::ResetEvent(m_hevent);
    }

    NC_BOOL NCThread::isAlive()
    {
        return (m_handle != NULL);
    }

    NC_BOOL NCThread::checkQuit()
    {
        return (m_quit_flg);
    }

    const NCCHAR* NCThread::getName()
    {
        return m_szThreadName;
    }

    VOID NCThread::resetName(const NCCHAR* name)
    {
        if (name == NULL) {
            return;
        }
        strncpy(m_szThreadName, name, NC_THREAD_NAME_LEN);
        m_szThreadName[NC_THREAD_NAME_LEN - 1] = '\0';
    }

    DWORD NCThread::getThreadID()
    {
        return (m_thread_id);
    }

    INT NCThread::getPriorityExt()
    {
        return 0;
    }

    NC_BOOL NCThread::setPriorityExt(INT pri)
    {
        return 0;
    }

    NC_BOOL NCThread::setPriorityNormal()
    {
        return 0;
    }

    NC_BOOL NCThread::setPriorityLow()
    {
        return 0;
    }

    NC_BOOL NCThread::setPriorityHigh()
    {
        return 0;
    }

    NC_BOOL NCThread::notifyMsg(LPVOID pAddr)
    {
        return 0;
    }

    VOID NCThread::run()
    {
        printf("calling father\n");
    }

    NCThreadSystemIF* NCThread::getThreadSystem()
    {
        return NULL;
    }

    VOID NCThread::sleep(ULONG secs)
    {
    }

    VOID NCThread::msleep(ULONG msecs)
    {
    }

    VOID NCThread::usleep(ULONG usecs)
    {
    }
}
/* EOF */
