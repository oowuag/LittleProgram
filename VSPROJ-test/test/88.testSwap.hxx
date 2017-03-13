#include <vector>
#include <list>
#include <string>
#include <stdio.h>


#define UINT32 unsigned int
#define NCString std::string

#define getString() c_str()

class VcardItem
{
public:
    VcardItem()
        : m_handleId(-1),
        m_name()
    {

    }

    VcardItem(UINT32 handleId, NCString name)
    {
        m_handleId = handleId;
        m_name = name;
    }

    UINT32 getHandlerId() const
    {
        return m_handleId;
    }

    NCString getName() const
    {
        return m_name;
    }
private:
    UINT32 m_handleId;
    NCString m_name;
};



int main()
{
    std::list<VcardItem> m_vcardItems;

#if 1
    NCString m_firstName = "A";
    NCString m_lastName = "";

    VcardItem v1(13, "Abcdefghijklmnopqrstuvwxyz");
    m_vcardItems.push_back(v1);
    VcardItem v2(2, "A");
    m_vcardItems.push_back(v2);
#else
    NCString m_firstName = "EQ";
    NCString m_lastName = "BB";

    VcardItem v1(13, "TO PR");
    m_vcardItems.push_back(v1);
    VcardItem v2(2, "EQ BB");
    m_vcardItems.push_back(v2);
#endif

    // optimize sort oder
    for (std::list<VcardItem>::iterator iter = m_vcardItems.begin(); iter != m_vcardItems.end(); ++iter) {
        std::string xmlName = iter->getName().getString();
        int x1 = xmlName.find(m_firstName.getString());
        int x2 = xmlName.find(m_lastName.getString());
        int x3 = std::string::npos;
        if ((std::string::npos != xmlName.find(m_firstName.getString()))
            && (std::string::npos != xmlName.find(m_lastName.getString()))) {
                // if xmlName contain firstname and lastname, it meets high probability to match
                if (m_vcardItems.begin() != iter) {
                    std::swap(*iter, *(m_vcardItems.begin()));
                }
                break;
        }
    }

    return 0;
}