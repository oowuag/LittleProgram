#include <iostream>
#include <string>
using namespace std;


struct FilterBit {
    //       bit  property                  onlyCheckV21  excludeForV21
    FN (       1, "FN",                       true,         false),
    PHOTO(     3, "PHOTO",                    false,        false),
    BDAY(      4, "BDAY",                     false,        false),
    ADR(       5, "ADR",                      false,        false),
    EMAIL(     8, "EMAIL",                    false,        false),
    TITLE(    12, "TITLE",                    false,        false),
    ORG(      16, "ORG",                      false,        false),
    NOTE(     17, "NOTE",                     false,        false),
    URL(      20, "URL",                      false,        false),
    NICKNAME( 23, "NICKNAME",                 false,        true),
    DATETIME( 28, "X-IRMC-CALL-DATETIME",     false,        false);

    int pos;
    string prop;
    bool onlyCheckV21;
    bool excludeForV21;

    FilterBit(int pos, string prop, bool onlyCheckV21, bool excludeForV21) {
        this.pos = pos;
        this.prop = prop;
        this.onlyCheckV21 = onlyCheckV21;
        this.excludeForV21 = excludeForV21;
    }
}


int main()
{
    return 0;
}