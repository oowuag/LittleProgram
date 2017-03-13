#include <stdlib.h>
#include "vcard_xml_parse.h"
#include "libxml/parser.h"
#include "libxml/xmlmemory.h"
#include "libxml/tree.h"
#include <vector>


bool parseVcardXmlString(const char* xmlStr, int len)
{
    //printf("xmlStr[%s]\n", xmlStr);
    xmlDocPtr pdoc = xmlParseMemory(xmlStr, len);
    xmlNodePtr curr = xmlDocGetRootElement(pdoc);
    if (curr == NULL) {
        printf("has not found root element\n");
        xmlFreeDoc(pdoc);
        return false;
    }
    if (!xmlStrcmp(curr->name, reinterpret_cast<const xmlChar *>("MAP-msg-listing"))) {
        for (curr = curr->xmlChildrenNode; curr; curr = curr->next) {
            char* handle = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("handle")));
            char* subject = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("subject")));
            char* datetime = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("datetime")));
            char* sender_name = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("sender_name")));
            char* sender_addressing = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("sender_addressing")));
            char* replyto_addressing = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("replyto_addressing")));
            char* recipient_name = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("recipient_name")));
            char* recipient_addressing = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("recipient_addressing")));
            char* type = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("type")));
            char* size = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("size")));
            char* text = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("text")));
            char* reception_status = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("reception_status")));
            char* attachment_size = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("attachment_size")));
            char* priority = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("priority")));
            char* read = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("read")));
            char* sent = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("sent")));
            char* protectedc = reinterpret_cast<char *>(xmlGetProp(curr, reinterpret_cast<const xmlChar *>("protected")));
            printf("--MAP-msg-listing-start---------------\n");
            printf("handle:%s\n", handle);
            printf("subject:%s\n", subject);
            printf("datetime:%s\n", datetime);
            printf("sender_name:%s\n", sender_name);
            printf("sender_addressing:%s\n", sender_addressing);
            printf("replyto_addressing:%s\n", replyto_addressing);
            printf("recipient_name:%s\n", recipient_name);
            printf("recipient_addressing:%s\n", recipient_addressing);
            printf("type:%s\n", type);
            printf("size:%s\n", size);
            printf("text:%s\n", text);
            printf("reception_status:%s\n", reception_status);
            printf("attachment_size:%s\n", attachment_size);
            printf("priority:%s\n", priority);
            printf("read:%s\n", read);
            printf("sent:%s\n", sent);
            printf("protected:%s\n", protectedc);
            printf("--MAP-msg-listing-end-----------------\n");
            xmlFree(handle);
            xmlFree(subject);
            xmlFree(datetime);
            xmlFree(sender_name);
            xmlFree(sender_addressing);
            xmlFree(replyto_addressing);
            xmlFree(recipient_name);
            xmlFree(recipient_addressing);
            xmlFree(type);
            xmlFree(size);
            xmlFree(text);
            xmlFree(reception_status);
            xmlFree(attachment_size);
            xmlFree(priority);
            xmlFree(read);
            xmlFree(sent);
            xmlFree(protectedc);
        }
    }
    xmlFreeDoc(pdoc);
    return true;
}

