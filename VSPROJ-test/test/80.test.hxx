#include <stdio.h>


    enum NCBT_SERVICE
    {
        NCBT_SERVICE_NONE = 0,
        NCBT_SERVICE_HFP = (1 << 0),
        NCBT_SERVICE_MIN = NCBT_SERVICE_HFP,
        NCBT_SERVICE_A2DP = (1 << 1),
        NCBT_SERVICE_AVRCP = (1 << 2),
        NCBT_SERVICE_PBAP = (1 << 3),
        NCBT_SERVICE_MAP = (1 << 4),
        NCBT_SERVICE_DIP = (1 << 5),
        NCBT_SERVICE_DUN = (1 << 6),
        NCBT_SERVICE_HID = (1 << 7),    // no use
        NCBT_SERVICE_PAN = (1 << 8),
        NCBT_SERVICE_OPP = (1 << 9),
        NCBT_SERVICE_SDP = (1 << 10),
        NCBT_SERVICE_SPP = (1 << 11),
        NCBT_SERVICE_MAX = NCBT_SERVICE_SPP,
    };
#define NCBT_AVAILABLE_SERVICE_DEFAULT (NCBT_SERVICE_HFP \
                                        | NCBT_SERVICE_A2DP \
                                        | NCBT_SERVICE_AVRCP \
                                        | NCBT_SERVICE_PBAP \
                                        | NCBT_SERVICE_MAP \
                                        | NCBT_SERVICE_DIP \
                                        | NCBT_SERVICE_DUN \
                                        | NCBT_SERVICE_PAN \
                                        | NCBT_SERVICE_OPP)

#define NCBT_SUPPORTED_AVAILABLE_SERVICE_ALL (NCBT_SERVICE_HFP \
                                              | NCBT_SERVICE_A2DP \
                                              | NCBT_SERVICE_AVRCP \
                                              | NCBT_SERVICE_PBAP \
                                              | NCBT_SERVICE_MAP \
                                              | NCBT_SERVICE_DIP \
                                              | NCBT_SERVICE_DUN \
                                              | NCBT_SERVICE_PAN \
                                              | NCBT_SERVICE_OPP)

#define NCBT_SUPPORTED_AVAILABLE_SERVICE_EXECPT_DUN_PAN  (NCBT_SERVICE_HFP \
                                                          | NCBT_SERVICE_A2DP \
                                                          | NCBT_SERVICE_AVRCP \
                                                          | NCBT_SERVICE_PBAP \
                                                          | NCBT_SERVICE_MAP \
                                                          | NCBT_SERVICE_DIP \
                                                          | NCBT_SERVICE_OPP)

#define NCBT_SUPPORTED_AVAILABLE_SERVICE_EXECPT_MAP (NCBT_SERVICE_HFP \
                                                    | NCBT_SERVICE_A2DP \
                                                    | NCBT_SERVICE_AVRCP \
                                                    | NCBT_SERVICE_PBAP \
                                                    | NCBT_SERVICE_DIP \
                                                    | NCBT_SERVICE_DUN \
                                                    | NCBT_SERVICE_PAN \
                                                    | NCBT_SERVICE_OPP)

#define NCBT_SUPPORTED_AVAILABLE_SERVICE_EXECPT_DUN_PAN_MAP (NCBT_SERVICE_HFP \
                                                            | NCBT_SERVICE_A2DP \
                                                            | NCBT_SERVICE_AVRCP \
                                                            | NCBT_SERVICE_PBAP \
                                                            | NCBT_SERVICE_DIP \
                                                            | NCBT_SERVICE_OPP)


int main()
{

    int x1 = NCBT_AVAILABLE_SERVICE_DEFAULT;
    int x2 = NCBT_SUPPORTED_AVAILABLE_SERVICE_ALL;
    int x3 = NCBT_SUPPORTED_AVAILABLE_SERVICE_EXECPT_DUN_PAN;
    int x4 = NCBT_SUPPORTED_AVAILABLE_SERVICE_EXECPT_MAP;
    int x5 = NCBT_SUPPORTED_AVAILABLE_SERVICE_EXECPT_DUN_PAN_MAP;



    return 0;
}