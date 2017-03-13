
#ifndef VCARD_XML_PARSE_H
#define VCARD_XML_PARSE_H

#include <string>

struct VcardItem
{
    int         m_handleId;
    std::string m_name;

    VcardItem(int handId, std::string name)
    	: m_handleId(handId)
		, m_name(name)
    {
    }
};

bool parseVcardXmlString(const char* xmlStr, int len);

#endif
