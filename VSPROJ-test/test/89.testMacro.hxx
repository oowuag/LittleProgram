#include <stdio.h>
#include <iostream>

#define _FILENAME_FUNC \
    { \
        std::string fileName = __FILE__; \
        size_t x1 = fileName.find_last_of("\\"); \
        if (std::string::npos != x1 && x1 < fileName.length() - 1) { \
            fileName = fileName.substr(x1 + 1); \
        } \
        std::string funcName = __FUNCTION__; \
        size_t x2 = funcName.find_last_of(":"); \
        if (std::string::npos != x2 && x2 < funcName.length() - 1) { \
            funcName = funcName.substr(x2 + 1); \
        } \
        printf("%s:%d@%s", fileName.c_str(), __LINE__, funcName.c_str()); \
    }

#define NCCOMM_LOG_D(A, B, ...) {printf("(" A ")[" B "]"); _FILENAME_FUNC; printf(__VA_ARGS__);printf("\n");}
#define NCCOMM_LOG_I(A, B, ...) NCCOMM_LOG_D(A, B, __VA_ARGS__)
#define NCCOMM_LOG_W(A, B, ...) NCCOMM_LOG_D(A, B, __VA_ARGS__)
#define NCCOMM_LOG_E(A, B, ...) NCCOMM_LOG_D(A, B, __VA_ARGS__)


int main()
{
    NCCOMM_LOG_D("a", "b", "c + %s", "d");
 





    NCCOMM_LOG_I("a", "b", "\t---- group[%d] ------", 1);
    NCCOMM_LOG_I("a", "b", "Unknown encoding \" %s \"", "pencodingval.c_str()");


    return 0;
}