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
 * @file NCSyncObj.h
 * @brief
 *
 */
#ifndef NCSYNCOBJ_H
#define NCSYNCOBJ_H

#ifndef NCTYPESDEFINE_H
#   include "NCTypesDefine.h"
#endif

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

namespace nutshell
{
    // Class declaration
    class NCSyncObj;

    /**
     * @brief
     *
     * @class NCSyncObj
     */
    class NCSyncObj
    {
    public:
        /**
         * Construction.
         */
        NCSyncObj();

        /**
         * Destruction.
         */
        virtual ~NCSyncObj();

        /**
         * Synchronize start.
         */
        VOID syncStart();

        /**
         * Try synchronize start
         *
         * @return NP_BOOL : NP_TRUE means synchronize succeed, and NP_FALSE failed.
         */
        NC_BOOL trySyncStart();

        /**
         * Synchronize end.
         */
        VOID syncEnd();

    protected:
        CRITICAL_SECTION cs;
        long lock_count;

    private:
        NCSyncObj(const NCSyncObj& src);
        NCSyncObj& operator = (const NCSyncObj& src);
    };

}
#endif /* NCSYNCOBJ_H */
/* EOF */