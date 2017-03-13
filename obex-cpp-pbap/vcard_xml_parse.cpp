#include <stdlib.h>
#include "vcard_xml_parse.h"
#include "libxml/parser.h"
#include "libxml/xmlmemory.h"
#include "libxml/tree.h"
#include <vector>


bool parseVcardXmlString(const char* xmlStr, int len)
{
    //printf("xmlStr[%s]\n", xmlStr);
    std::vector<VcardItem> vcardItems;

    xmlDocPtr pdoc = xmlParseMemory(xmlStr, len);
    xmlNodePtr curr = xmlDocGetRootElement(pdoc);
    if (curr == NULL) {
        printf("has not found root element\n");
        xmlFreeDoc(pdoc);
        return false;
    }
    if (!xmlStrcmp(curr->name, reinterpret_cast<const xmlChar *>("vCard-listing"))) {
        for (curr = curr->xmlChildrenNode; curr; curr = curr->next) {
            char* handle = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("handle")));
            if (NULL == handle) {
                continue;
            }
            char *stopstring = NULL;
            long handleId = strtol(handle, &stopstring, 16);
            xmlFree(handle);
            char* nodeName = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("name")));
            if (NULL == nodeName) {
                continue;
            }
            printf("handleId_0x[%lx], nodeName[%s]\n", handleId, nodeName);
            vcardItems.push_back(VcardItem(handleId, nodeName));
            xmlFree(nodeName);
        }
    }
    printf("vcardItems length [%lu]\n", vcardItems.size());
    xmlFreeDoc(pdoc);
    return true;
}

