#include <string>
#include <stdio.h>

#define NC_BOOL bool
#define NCCHAR char

#include <string>
#include <stdarg.h>
#include <algorithm>
class NCString : public std::string
{
public:
    NCString()
        : std::string()
    {
    }
    NCString(const NCString& str)
        : std::string(str)
    {
    }
    NCString::NCString(const NCCHAR* p)
    {
        set(p);
    }

    NCString::NCString(const NCCHAR* p, int l)
    {
        set(p, l);
    }

    virtual ~NCString()
    {
    }

    int NCString::getLength() const
    {
        return static_cast<int>(length());
    }

    const NCCHAR*
    NCString::getString() const
    {
        return c_str();
    }

    bool NCString::set(const NCString& str)
    {
        return set(str.c_str());
    }

    bool NCString::set(const NCCHAR* p, int l = 0)
    {
        assign(p, l);
        return true;
    }

    bool NCString::format(const NCCHAR* format, ...)
    {
        if (format == NULL) {
            return false;
        }
        if (strlen(format) == 0) {
            return set("", 0);
        }
        int nMaxLen = static_cast<int>(strlen(format));
        while (1) {
            NCCHAR* szTemp = new NCCHAR[nMaxLen + 1];
            if (szTemp == NULL) {
                return false;
            }
            va_list list;
            va_start(list, format);
            int nRes = vsnprintf(szTemp, nMaxLen + 1, format, list);
            va_end(list);
            if (nRes < 0) {
                return false;
            }
            else if (nRes > nMaxLen) {
                delete[] szTemp;;
                nMaxLen = nRes;
            }
            else {
                set(szTemp, nRes);
                delete[] szTemp;;
                return true;
            }
        }
        return false;
    }

    int NCString::replace(const NCCHAR* src, const NCCHAR* des)
    {
        int nFindCnt = 0;
        std::size_t found = find(src);
        while (std::string::npos != found) {
            std::string::replace(found, strlen(src), des);
            found = find(src, found + 1);
            nFindCnt++;
        }
        return nFindCnt;
    }

    bool NCString::lowerCase()
    {
        std::transform(begin(), end(), begin(), ::tolower);
        return true;
    }

    NCString& operator=(const NCString& rhs)
    {
        if (this != &rhs)
        {
            std::string::operator=(rhs);
        }
        return *this;
    }

    NCString& operator +=(const NCString& rhs)
    {
        std::string::operator+=(rhs);
        return *this;
    }
};
const NCString operator +(const NCString& lhs, const NCString& rhs)
{
    NCString res(lhs);
    res += rhs;
    return res;
}

bool operator== (const NCString& lhs, const NCString& rhs) { return (std::string(lhs) == std::string(rhs)); }
bool operator== (const char*   lhs, const NCString& rhs)   { return (std::string(lhs) == std::string(rhs)); }
bool operator== (const NCString& lhs, const char*   rhs)   { return (std::string(lhs) == std::string(rhs)); }

bool operator!= (const NCString& lhs, const NCString& rhs) { return (std::string(lhs) != std::string(rhs)); }
bool operator!= (const char*   lhs, const NCString& rhs)   { return (std::string(lhs) != std::string(rhs)); }
bool operator!= (const NCString& lhs, const char*   rhs)   { return (std::string(lhs) != std::string(rhs)); }

bool operator<  (const NCString& lhs, const NCString& rhs) { return (std::string(lhs) <  std::string(rhs)); }
bool operator<  (const char*   lhs, const NCString& rhs)   { return (std::string(lhs) <  std::string(rhs)); }
bool operator<  (const NCString& lhs, const char*   rhs)   { return (std::string(lhs) <  std::string(rhs)); }

bool operator<= (const NCString& lhs, const NCString& rhs) { return (std::string(lhs) <= std::string(rhs)); }
bool operator<= (const char*   lhs, const NCString& rhs)   { return (std::string(lhs) <= std::string(rhs)); }
bool operator<= (const NCString& lhs, const char*   rhs)   { return (std::string(lhs) <= std::string(rhs)); }

bool operator>  (const NCString& lhs, const NCString& rhs) { return (std::string(lhs) >  std::string(rhs)); }
bool operator>  (const char*   lhs, const NCString& rhs)   { return (std::string(lhs) >  std::string(rhs)); }
bool operator>  (const NCString& lhs, const char*   rhs)   { return (std::string(lhs) >  std::string(rhs)); }

bool operator>= (const NCString& lhs, const NCString& rhs) { return (std::string(lhs) >= std::string(rhs)); }
bool operator>= (const char*   lhs, const NCString& rhs)   { return (std::string(lhs) >= std::string(rhs)); }
bool operator>= (const NCString& lhs, const char*   rhs)   { return (std::string(lhs) >= std::string(rhs)); }

int main()
{
    NCString a = "a";
    NCString b = "b";
    printf("%s\n", a.c_str());
    printf("%s\n", b.c_str());
    a = b;
    a = a;
    printf("%s\n", a.c_str());
    NCString c(a);
    printf("%s\n", c.c_str());


    NCString x = "x";
    printf("%s\n", x.c_str());


    NCString y = a + b;

    std::string xx;
    std::string yy;
    if (xx == yy) {
        printf("OK\n");
    }

    NCString x1 = "abc def ok";
    int x1ret = x1.replace(" ", "");
    printf("%s\n", x1.c_str());

    x1.format("%d", 11);
    printf("%s\n", x1.c_str());




    return 0;
}