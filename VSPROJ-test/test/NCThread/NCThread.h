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
#define NCTHREAD_H

#ifndef NCTYPESDEFINE_H
#   include "NCTypesDefine.h"
#endif


namespace nutshell
{
#define NCTHREADSYSTEMIF_H
    struct NCThreadSystemIF{
    };

#define NCTHREADSYSTEM_H
    class NCThreadSystem : public NCThreadSystemIF
    {
    };

    class NCThreadBase
    {
    };

#define NC_THREAD_NAME_LEN         (64)

    /**
     * @brief
     *
     * @class NCThread
     */
    class NCThread
    {
    public:

        /**
         * Constructs a new thread. The thread does not begin executing
            until startThread() is called.
         */
        NCThread();

        /**
         * Constructor with NCThreadSystemIF.
         *
         * @param NCThreadSystemIF point
         */
        explicit NCThread(NCThreadSystemIF*);

        /**
         * Destructor
         */
        virtual ~NCThread();

        /**
         * Begins execution of the thread by calling run(), which should be
         *  reimplemented in a NCThread subclass to contain your code.
         *
         * @param  name: Thread name. Default name is "Unknown Thread".
         * \note  If the thread is already running, this function does nothing.
         */
        virtual VOID startThread(const NCCHAR *name = NCTEXT("Unknown Thread"));

        /**
         * Stop the thread.
         *
         * @param  msec: Default msec is INFINITE.
         * @return  NC_TRUE indicate success, vis versa.
         */
        virtual NC_BOOL stopThread(DWORD msec = INFINITE);

        /**
         * Terminates the execution of the thread. The thread may or may not
         *  be terminated immediately, depending on the operating systems
         *  scheduling policies.
         *
         * @return  NC_TRUE indicate success, vis versa.
         */
        virtual NC_BOOL terminate();

        /**
         * Joins a thread.
         *
         * @param  msec: Default msec is INFINITE.
         * @return  NC_TRUE indicate success, vis versa.
         */
        virtual NC_BOOL join(DWORD msec = INFINITE);

        /**
         * Blocks the thread if this NCThread object has finished.
         * @param  msec: time milliseconds has elapsed. If time is INFINITE (the
             default), then the wait will never timeout.
         * @return  NC_TRUE indicate success, vis versa.
         */
        virtual NC_BOOL wait(DWORD msec = INFINITE);

        /**
         * Wakes one thread waiting on the wait condition.
         */
        virtual VOID notify();

        /**
         * Reset the notify status.
         */
        virtual VOID reset();

        /**
         * Check whether the thread is alive or not.
         *
         * @return NC_TRUE indicates that the thread is alive,
            otherwise the thread is not alive.
         */
        virtual NC_BOOL isAlive();

        /**
         * Check whether the thread is quit.
         *
         * @return NC_TRUE indicates that the thread is quit,
            otherwise the thread is not quit.
         */
        virtual NC_BOOL checkQuit();

        /**
         * Get the thread name.
         *
         * @return thread name.
         */
        virtual const NCCHAR* getName();

        /**
         * Reset thread name for a thread which already have name before.
         *
         * @param  name: new thread name.
         * \note It is not a reentrant function. So, do not call it in other thread.
         *       If the name argument is NULL, the name would not be changed.
         */
        virtual VOID resetName(const NCCHAR* name);

        /**
         * Get the thread ID.
         *
         * @return thread ID.
         * \note If thread has not started yet, return 0.
         */
        virtual DWORD getThreadID();

        /**
         * Get the current thread priority.
         *
         * @return thread priority.
         */
        virtual INT getPriorityExt();

        /**
         * Set the priority for a running thread.
         *
         * @param  pri: new thread priority.
         * @return  NC_TRUE indicate success, vis versa.
         * \note If the thread is not running, this function does nothing and returns NC_FALSE immediately.
            Use startThread() to start a thread first.
         */
        virtual NC_BOOL setPriorityExt(INT pri);
        /**
         * Set thread priority to Normal Priority.
         *
         * @return   NC_TRUE indicate success, vis versa.
         * \note Normal Priority is the default priority of the operating
                system.
         */
        virtual NC_BOOL setPriorityNormal();

        /**
         * Set thread priority to Low Priority.
         *
         * @return  NC_TRUE indicate success, vis versa.
         * \note Low Priority scheduled less often than Normal Priority.
         */
        virtual NC_BOOL setPriorityLow();

        /**
         * Set thread priority to High Priority.
         *
         * @return   NC_TRUE indicate success, vis versa.
         * \note High Priority scheduled more often than NormalPriority.
         */
        virtual NC_BOOL setPriorityHigh();

        /**
         * Wakes one thread waiting on the wait condition. Besides, pAddr
         *  restored by message queue.
         *
         * @param pAddr: message.
         * @return  NC_TRUE indicate success, vis versa.
         * \note It only used by timer, See \ref NCTimer.h.
         */
        virtual NC_BOOL notifyMsg(LPVOID pAddr);

        /**
         * This is the main part of the thread. Running the action that define
         *  in this function. The starting point for the thread. After calling
         *  startThread(), the newly created thread calls this function.
         */
        virtual VOID run();

        /**
         * Get the thread system information.
         *
         * @return  NCThreadSystemIF.
         */
        virtual NCThreadSystemIF* getThreadSystem();

    protected:
        static void ThreadProc(LPVOID p);
        void ExecRun();
        void DispatchWindowMessages();

        /**
         * Forces the current thread to sleep for secs seconds.
         *
         * \ref msleep(), usleep().
         */
        virtual VOID sleep(ULONG secs);

        /**
         * Forces the current thread to sleep for msecs milliseconds.
         *
         * \ref sleep(), usleep().
         */
        virtual VOID msleep(ULONG msecs);

        /**
         * Forces the current thread to sleep for usecs microseconds.
         *
         * \ref sleep(), msleep().
         */
        virtual VOID usleep(ULONG usecs);

    private:
        NCThreadBase* m_pThreadImpl;
        NCThreadSystemIF* m_pThreadSystem;

        NCCHAR			m_szThreadName[NC_THREAD_NAME_LEN];
        HANDLE			m_hevent;
        volatile BOOL	m_quit_flg;
        volatile HANDLE	m_handle;
        volatile DWORD	m_thread_id;

    private:
        NCThread(const NCThread&);
        NCThread& operator=(const NCThread&);
    };


}

#endif

/* EOF */
