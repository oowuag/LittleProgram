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

#ifndef NCSTRING_H
#define NCSTRING_H

#ifndef NCTYPESDEFINE_H
#   include "NCTypesDefine.h"
#endif

#include <string>
#include <algorithm>
//#define NCString std::string

#define NCCORE_API 

namespace nutshell
{

    typedef NC_BOOL (*NC_Judger)(const NCCHAR* chVal, USHORT& chLen);

    class NCString
    {
    public:
        NCString() { }
        NCString(const NCString& rhs)
        {
            m_str = rhs.m_str;
        }
        NCString(const NCCHAR* p)
        {
            if (p) {
                m_str = p;
            }
        }
        NCString(const NCCHAR* p, INT l)
        {
            m_str = p;
            m_str.resize(l);
        }

#ifndef NSTRING_UNICODE
        NCString(const WCHAR* p) { }
        NCString(const WCHAR* p, INT l) { }
        NC_BOOL set(const WCHAR* p) { }
        NC_BOOL set(const WCHAR* p, INT l) { }
        INT getString(WCHAR* p, INT l) const { }
#endif

        NCString(DWORD dwCodepage, const CHAR* p) { }
        NCString(DWORD dwCodepage, const CHAR* p, INT l) { }

        virtual ~NCString() { }

        NC_BOOL set(const NCCHAR* p)
        {
            m_str = p;
            return NC_TRUE;
        }

        NC_BOOL set(const NCCHAR* p, INT l)
        {
            m_str = p;
            m_str.resize(l);
            return NC_TRUE;
        }

        //INT getString(NCCHAR* p, INT l) const { }

        const NCCHAR* getString() const
        {
            return m_str.c_str();
        }

        INT getLength() const
        {
            return m_str.length();
        }

        //NC_BOOL format(INT maxlen, const NCCHAR* format, ...) {}

        NC_BOOL format(const NCCHAR* fmt, ...)
        {
            if (fmt == NULL) {
                return NC_FALSE;
            }

            va_list vl;

            va_start(vl, fmt);
            int size = vsnprintf(0, 0, fmt, vl) + sizeof('\0');
            va_end(vl);

            char buffer[256];

            va_start(vl, fmt);
            size = vsnprintf(buffer, size, fmt, vl);
            va_end(vl);

            m_str = std::string(buffer, size);

            return NC_TRUE;
        }

        INT replace(const NCCHAR* src, const NCCHAR* des)
        {
            INT nReplaceCount = 0;
            size_t start_pos = 0;
            while((start_pos = m_str.find(src, start_pos)) != std::string::npos) {
                m_str.replace(start_pos, strlen(src), des);
                start_pos += strlen(des); // ...
                nReplaceCount++;
            }
            return nReplaceCount;
        }


        NC_BOOL lowerCase()
        {
            std::transform(m_str.begin(), m_str.end(), m_str.begin(), ::tolower);
            return NC_TRUE;
        }

        NC_BOOL endWith(NCCHAR ch) const
        {
            if (m_str.length() <= 0) {
                return NC_FALSE;
            }

            return (*(m_str.rbegin()) == ch) ? NC_TRUE : NC_FALSE;
        }

        NC_BOOL NCString::add(const NCCHAR* p)
        {
            if (p == NULL) {
                return NC_FALSE;
            }
            m_str += p;
            return NC_TRUE;
        }

        NC_BOOL NCString::trim()
        {
            m_str.erase(0, m_str.find_first_not_of(" "));
            m_str.erase(m_str.find_last_not_of(" ") + 1);
            return NC_TRUE;
        }

        INT NCString::find(const NCCHAR* substr) const
        {
            return m_str.find(substr);
        }

        NCString left(INT len) const
        {
            NCString strDes;
            if (m_str.length() <= 0) {
                return strDes;
            }
            strDes.set(m_str.c_str(), len);
            return strDes;
        }

        //const char* c_str() const
        //{	// return pointer to null-terminated nonmutable array
        //    return m_str.c_str();
        //}

        //unsigned int length() const
        //{	// return length of sequence
        //    return m_str.length();
        //}

        operator const NCCHAR*() const
        {
            return getString();
        }

        NCString& operator=(const NCString& rhs)
        {
            m_str = rhs.m_str;
            return *this;
        }

        NCString& operator +=(const NCString& rhs)
        {
            m_str += rhs.m_str;
            return *this;
        }

        NC_BOOL NCString::operator ==(const NCString& lhr) const
        {
            return m_str == lhr.m_str;
        }

        NC_BOOL NCString::operator !=(const NCString& lhr) const
        {
            return m_str != lhr.m_str;
        }

        // std::string
    private:
        std::string m_str;
    };

    // This global operator is used to concatenate two NCString together.
    NCCORE_API const NCString operator +(const NCString& lhs, const NCString& rhs);
    // \name Interfaces for compare NCString and NCCHAR array
    // These interfaces is used to stop compiler converting NCString to NCCHAR* and compare the memory address.
    // @{
    // Compare NCString to NCCHAR array.
    NCCORE_API NC_BOOL operator ==(const NCString& lhs, const NCCHAR* rhs);
    // Compare NCCHAR array to NCString.
    NCCORE_API NC_BOOL operator ==(const NCCHAR* rhs, const NCString& lhs);
    // Compare NCString to NCCHAR array for not equal.
    NCCORE_API NC_BOOL operator !=(const NCString& lhs, const NCCHAR* rhs);
    // Compare NCCHAR array to NCString for not equal.
    NCCORE_API NC_BOOL operator !=(const NCCHAR* rhs, const NCString& lhs);
}

#endif
/* EOF */
