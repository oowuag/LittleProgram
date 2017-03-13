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
 * @file NCRefBase.h
 * @brief
 *
 */
#ifndef NCREFBASE_H
#define NCREFBASE_H

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <list>
#include <map>
#include <vector>
#include "NCTypesDefine.h"

//#ifndef NCGLOBALAPI_H
//#   include "ncore/NCGlobalAPI.h"
//#endif
//#ifndef NCTYPESDEFINE_H
//#   include "ncore/NCTypesDefine.h"
//#endif
//#ifndef ANDROID_REF_BASE_H
//#   include <utils/RefBase.h>
//#endif
//#ifndef ANDROID_VECTOR_H
//#   include <utils/Vector.h>
//#endif

//#define DEF_CUSTOM_SP

#ifndef DEF_CUSTOM_SP

#include "../RefBase/RefBase.h"

#else
namespace nutshell
{
    class RefBase
    {
    public:
        RefBase() {};
        ~RefBase() {};
        void            incStrong(const void* id) const {}
        void            decStrong(const void* id) const {}
        void            forceIncStrong(const void* id) const {}
    };
}

namespace android
{
    struct _counter
    {
        _counter(int u):use(u){}
        ~_counter(){}
        int use;
    };  

    template<class T>
    class sp
    {
    public:
        //inline sp() : m_ptr(0) { }
        sp(T *t = NULL) : m_pc(new _counter(1))
        {
            m_ptr = t;
        }

        sp<T>(const sp<T> &rhs)
        {
            m_pc = rhs.m_pc;
            m_ptr = rhs.m_ptr;
            m_pc->use++;
        }

        ~sp()
        {
            if(--m_pc->use == 0)
            {
                delete m_ptr;
                delete m_pc;
            }
        }

        sp<T>& operator = (const sp<T>& rhs)
        {
            rhs.m_pc->use++;
            if(--m_pc->use == 0)
            {
                delete m_ptr;
                delete m_pc;
            }
            m_pc = rhs.m_pc;
            m_ptr = rhs.m_ptr;
            return *this;
        }

        template<typename U> sp<T>& operator = (const sp<U>& other)
        {
            T* otherPtr(other.m_ptr);
            if (otherPtr) {
                other.m_pc->use++;
            }
            if (m_ptr) {
                if(--m_pc->use == 0)
                {
                    delete m_ptr;
                    delete m_pc;
                }
            }
            m_ptr = otherPtr;
            m_pc = other.m_pc;

            return *this;
        }

        sp<T>& operator = (T* other)
        {
            if (m_ptr != other)
            {
                if(--m_pc->use == 0)
                {
                    delete m_ptr;
                }
                m_ptr = other;
                if (other)
                {
                    m_pc->use = 1;
                }
            }
            return *this;
        }

        template<typename U> sp<T>& operator = (U* other)
        {
            if (m_ptr != other)
            {
                if(--m_pc->use == 0)
                {
                    delete m_ptr;
                }
                m_ptr = other;
                m_pc->use = 1;
            }
            return *this;
        }

        T* get() { return m_ptr; }
        T& operator *(){ return *m_ptr; }  
        T* operator ->() { return m_ptr; }

        inline bool operator != (const T* o) const { return m_ptr != o; }
        inline bool operator == (const T* o) const { return m_ptr == o; }

        inline bool operator != (const sp<T>& other) const { return m_ptr != other.m_ptr; }
        inline bool operator == (const sp<T>& other) const { return m_ptr == other.m_ptr; }

        template<typename Y> friend class sp;
        template<typename Y> friend class wp;
    private:  
        T*        m_ptr;
        _counter* m_pc;
    };


    template<class T>
    class wp
    {
    public:
        wp(T* other) : m_ptr(other) { m_spRef = NULL; }
        wp(const wp<T>& other) : m_ptr(other.m_ptr) { m_spRef = other.m_spRef; }
        wp(const sp<T>& other) : m_ptr(other.m_ptr) { m_spRef = (sp<T> *)&other; }
        template<typename U> wp(U* other) : m_ptr(other) { m_spRef = NULL; }
        template<typename U> wp(const sp<U>& other) : m_ptr(other.m_ptr) { m_spRef = (sp<T> *)&other; }
        template<typename U> wp(const wp<U>& other) : m_ptr(other.m_ptr) { m_spRef = other.m_spRef; }

        T& operator *(){ return *m_ptr; }
        T* operator ->() { return m_ptr; }
        inline bool operator != (const T* o) const { return m_ptr != o; }

        sp<T> promote() const
        {
            if (m_spRef) { // assigned sp before
                sp<T>& ssp(*m_spRef);
                return ssp;
            }
            else { // sp null
                sp<T> ssp(m_ptr);
                ssp.m_pc->use++; // add again
                return ssp;
            }
        }

        wp<T>& operator = (T* other)
        {
            m_ptr = other;
            return *this;
        }
        wp<T>& operator = (const wp<T>& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        wp<T>& operator = (const sp<T>& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }

        template<typename U> wp& operator = (U* other)
        {
            m_ptr = other;
            return *this;
        }
        template<typename U> wp& operator = (const wp<U>& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        template<typename U> wp& operator = (const sp<U>& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }

        template<typename Y> friend class sp;
        template<typename Y> friend class wp;
    private:
        T *m_ptr;
        sp<T>* m_spRef;
    };

}
#endif

namespace nutshell
{
    /**
    * sample code:
    * @code
    * //must overried from NCRefBase!!!
    * class A : public NCRefBase
    * {
    * };
    *
    * //load a object with a strong pointer.
    * ncsp<A>::sp spa = new A();
    *
    * //reload with a weak pointer.
    * ncsp<A>::wp wpa = spa;
    *
    * //promote the weak pointer to a strong pointer.
    * //it fails if the obj already destoryed.
    * ncsp<A>::sp spa2 = wpa.promote();
    *
    * @endcode
    * @see NCSpThread.h
    */
#ifndef DEF_CUSTOM_SP
    typedef android::RefBase NCRefBase;
#else
    typedef nutshell::RefBase NCRefBase;
#endif

    template <typename T>
    struct ncsp
    {
        typedef android::sp<T> sp;
        typedef android::wp<T> wp;
        typedef std::vector<sp> Vector;
        typedef std::vector<wp> Vector_Wp;
    };
}

#endif /* NCREFBASE_H */
/* EOF */
