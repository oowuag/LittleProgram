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

#include "NCString.h"

namespace nutshell
{
    const NCString operator +(const NCString& lhs, const NCString& rhs)
    {
        NCString res(lhs);
        res += rhs;
        return res;
    }

    NC_BOOL operator ==(const NCString& lhs, const NCCHAR* rhs)
    {
        return lhs.operator ==(NCString(rhs));
    }

    NC_BOOL operator !=(const NCString& lhs, const NCCHAR* rhs)
    {
        return !(lhs == rhs);
    }

    NC_BOOL operator ==(const NCCHAR* lhs, const NCString& rhs)
    {
        return rhs.operator ==(NCString(lhs));
    }

    NC_BOOL operator !=(const NCCHAR* lhs, const NCString& rhs)
    {
        return !(lhs == rhs);
    }
}

/* EOF */
