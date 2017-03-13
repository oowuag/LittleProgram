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

#ifndef NCSYNCOBJ_H
#   include "NCSyncObj.h"
#endif

namespace nutshell
{

    NCSyncObj::NCSyncObj()
    {
        lock_count = 0;
        ::InitializeCriticalSection(&cs);
    }

    NCSyncObj::~NCSyncObj()
    {
        ::DeleteCriticalSection(&cs);
    }

    VOID
    NCSyncObj::syncStart()
    {
        ::EnterCriticalSection(&cs);
    }

    NC_BOOL
    NCSyncObj::trySyncStart()
    {
        return 0;
    }

    VOID
    NCSyncObj::syncEnd()
    {
        ::LeaveCriticalSection(&cs);
    }

}
/* EOF */