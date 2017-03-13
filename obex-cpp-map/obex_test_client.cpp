/**
	\file apps/obex_test_client.c
 	Client OBEX Commands.
	OpenOBEX test applications and sample code.

	Copyright (c) 2000 Pontus Fuchs, All Rights Reserved.

	OpenOBEX is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with OpenOBEX. If not, see <http://www.gnu.org/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <openobex/obex.h>

#include "obex_test_client.h"
#include "obex_test.h"
#include "vcard_xml_parse.h"

#if defined(_WIN32)
#include <io.h>
#define read _read
#define write _write
#define open _open
#if ! defined(_MSC_VER)
#warning "No implementation for basename() in win32!"
#endif
#define basename(x) (x)
#else
#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

int Is_Android = FALSE;

#pragma pack(1)
typedef struct obex_setpath_hdr {
    uint8_t  flags;
    uint8_t constants;
} obex_setpath_hdr_t;
#pragma pack()


enum map_cmd_id_type {
	map_cmd_id_SendEvent = 0,
	map_cmd_id_SetNotificationRegistration,
	map_cmd_id_SetFolder,
	map_cmd_id_GetFolderListing,
	map_cmd_id_GetMessagesListing,
	map_cmd_id_GetMessage
};

//
// Wait for an obex command to finish.
void syncwait(obex_t *handle)
{
	CDEBUG();
	struct context *gt;
	int ret;

	gt = (context *)OBEX_GetUserData(handle);

	while(!gt->clientdone) {
		//CDEBUG("syncwait()");
		ret = OBEX_HandleInput(handle, 30);
		if(ret < 0) {
			CDEBUG("Error while doing OBEX_HandleInput()");
			break;
		}
		if(ret == 0) {
			/* If running cable. We get no link-errors, so cancel on timeout */
			CDEBUG("Timeout waiting for data. Aborting");
			OBEX_CancelRequest(handle, FALSE);
			break;
		}
	}

	gt->clientdone = FALSE;
	CDEBUG("done");
}

//
//
void client_done(obex_t *handle, obex_object_t *object, int obex_cmd, int obex_rsp)
{
	CDEBUG();
	struct context *gt;
	gt = (context *)OBEX_GetUserData(handle);

	switch(obex_cmd)	{
	case OBEX_CMD_CONNECT:
		connect_client_done(handle, object, obex_rsp);
		break;
	case OBEX_CMD_DISCONNECT:
		disconnect_client_done(handle, object, obex_rsp);
		break;
	case OBEX_CMD_PUT:
		put_client_done(handle, object, obex_rsp);
		break;
	case OBEX_CMD_GET:
		get_client_done(handle, object, obex_rsp, gt->get_name);
		break;
	case OBEX_CMD_SETPATH:
		setpath_client_done(handle, object, obex_rsp);
		break;
	}
	gt->clientdone = TRUE;
}

class AppParamValue
{
public:
	int NewMessage;
    int FolderListingSize;
    int MessagesListingSize;
    int FractionDeliver;
    std::string MSETime;
	AppParamValue()
		: NewMessage(0)
		, FolderListingSize(0)
		, FractionDeliver(0)
	{
	}

    void dump()
	{
        printf("FolderListingSize=%d\n",FolderListingSize);
    }
};

bool parseApplicationParameter(char appParam[], int len)
{
    int i = 0;
	AppParamValue appParamValue;
	int tagId = -1;
	int tagLength = 0;
    while (i < len) {
        tagId = appParam[i++] & 0xff;     // Convert to unsigned to support values above 127
        tagLength = appParam[i++] & 0xff; // Convert to unsigned to support values above 127
        switch (tagId) {
            case 0x0d: //NewMessage
            {
                appParamValue.NewMessage = appParam[i] & 0x01;
                CDEBUG("appParamValue.NewMessage:%d", appParamValue.NewMessage);
            }
            break;
            case 0x11: //FolderListingSize
            {
                int highValue = appParam[i] & 0xff;
                int lowValue = appParam[i + 1] & 0xff;
                appParamValue.FolderListingSize = highValue * 256 + lowValue;
                CDEBUG("appParamValue.FolderListingSize:%d", appParamValue.FolderListingSize);
            }
            break;
            case 0x12: //MessagesListingSize
            {
                int highValue = appParam[i] & 0xff;
                int lowValue = appParam[i + 1] & 0xff;
                appParamValue.MessagesListingSize = highValue * 256 + lowValue;
                CDEBUG("appParamValue.MessagesListingSize:%d", appParamValue.MessagesListingSize);
            }
            break;
            case 0x16: //FractionDeliver
            {
                appParamValue.FractionDeliver = appParam[i] & 0x01;
                CDEBUG("appParamValue.FractionDeliver:%d", appParamValue.FractionDeliver);
            }
            break;
            case 0x19: //MSETime
            {
            	char *buff = new char[tagLength + 1];
            	memcpy(buff, &appParam[i], tagLength);
                appParamValue.MSETime = buff;
                delete[] buff;
                CDEBUG("appParamValue.MSETime:%s", appParamValue.MSETime.c_str());
            }
            break;
            default:
            {
                CDEBUG("Parse Application Parameter error");
                return false;
            }
            break;
        }
        i += tagLength; // Offset to next TagId
    }
    if (0) appParamValue.dump();
    return true;
}

//
void parseResponse(obex_t *handle, obex_object_t *object, int obex_rsp)
{
	CDEBUG();
	if(obex_rsp == OBEX_RSP_SUCCESS) {
		CDEBUG("OK!");
		uint8_t *nonhdrdata;
		if(OBEX_ObjectGetNonHdrData(object, &nonhdrdata) == 4) {
			CDEBUG("Version: 0x%02x. Flags: 0x%02x", nonhdrdata[0], nonhdrdata[1]);
		}

		obex_headerdata_t hv;
		uint8_t hi;
		uint32_t hlen;

		struct context *gt = NULL;
		gt = (context *)OBEX_GetUserData(handle);

		const uint8_t *body = NULL;
		int body_len = 0;

		while (OBEX_ObjectGetNextHeader(handle, object, &hi, &hv, &hlen))	{
			switch(hi)	{
			case OBEX_HDR_NAME:
			{
				char name[200];
				OBEX_UnicodeToChar((uint8_t *) name, hv.bs, hlen);
				CDEBUG("OBEX_HDR_NAME = %*.*s", hlen/2, hlen/2, name);
			}
			break;

			case OBEX_HDR_LENGTH:
				CDEBUG("HEADER_LENGTH = %d", hv.bq4);
			break;

			case OBEX_HDR_CONNECTION:
				CDEBUG("CONNECTIONID = %d", hv.bq4);
				gt->connectionid = hv.bq4;
			break;

			case OBEX_HDR_WHO:
			{
				char who[200];
				OBEX_UnicodeToChar((uint8_t *)who, hv.bs, hlen);
				CDEBUG("OBEX_HDR_WHO[%d] = %*.*s", hlen/2, hlen/2, hlen/2, who);
			}
			break;

			case OBEX_HDR_BODY:
				CDEBUG("Found body len[%d]", hlen);
				body = hv.bs;
				body_len = hlen;
				CDEBUG("%*.*s", body_len, body_len, body);
				if (gt->client_cmd_id == map_cmd_id_GetMessagesListing) {
					parseVcardXmlString((char*)body, body_len);
				}
			break;

			case OBEX_HDR_BODY_END:
				CDEBUG("Found body end");
				body = hv.bs;
				body_len = hlen;
				CDEBUG("%*.*s", body_len, body_len, body);
			break;

			case OBEX_HDR_APPARAM:
				body = hv.bs;
				body_len = hlen;
				CDEBUG("OBEX_HDR_APPARAM: %d", body_len);
				printf("0x [ ");
				for (int i = 0; i < body_len; ++i)
				{
					printf("%02x ", body[i]);
				}
				printf("]\n");
				parseApplicationParameter((char *)body, body_len);
			break;

			default:
				CDEBUG("%s() Skipped header %02x", __FUNCTION__, hi);
			}
		}
	}
	else {
		CDEBUG("obex_rsp failed 0x%02x! (%s)", obex_rsp, OBEX_ResponseToString(obex_rsp));
	}
}

//
//
void connect_client(obex_t *handle)
{
	CDEBUG("");
	obex_object_t *object;
	obex_headerdata_t hd;

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_CONNECT)))	{
		CDEBUG("Error");
		return;
	}

	//bb582b40-420c-11db-b0de-0800200c9a66
	hd.bs = (uint8_t*)"\xbb\x58\x2b\x40\x42\x0c\x11\xdb\xb0\xde\x08\x00\x20\x0c\x9a\x66";
	if(OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TARGET, hd, 16, OBEX_FL_FIT_ONE_PACKET) < 0)	{
		CDEBUG("Error adding header");
		OBEX_ObjectDelete(handle, object);
		return;
	}

	OBEX_Request(handle, object);
	syncwait(handle);
}

void connect_client_done(obex_t *handle, obex_object_t *object, int obex_rsp)
{
	CDEBUG("");
	parseResponse(handle, object, obex_rsp);
}

//
//
void disconnect_client(obex_t *handle)
{
	obex_object_t *object;

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_DISCONNECT))) {
		CDEBUG("Error");
		return;
	}

	OBEX_Request(handle, object);
	syncwait(handle);
}

void disconnect_client_done(obex_t *handle, obex_object_t *object, int obex_rsp)
{
	CDEBUG("Disconnect done!");
	OBEX_TransportDisconnect(handle);
}

//5.1 SendEvent function
void put_client_SendEvent(obex_t *handle)
{
	CDEBUG("");
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("PUT > ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_PUT)))	{
		CDEBUG("Error");
		return;
	}

	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = map_cmd_id_SendEvent;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/MAP-event-report";
	hd.bs = (uint8_t *) tp;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TYPE, hd, strlen(tp) + 1, OBEX_FL_FIT_ONE_PACKET);

	uint8_t para[100] = { 0 };
	int i = 0;
	para[i++] = 0x0F; // MASInstanceID
	para[i++] = 1;
	para[i++] = 0x00; 	// 0-255
	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);

	//MAP-Event-Report object
	uint8_t buff[512] = { 0 };
	char* type_str = "NewMessage";
	char* handle_str = "12345678";
	char* folder_str = "TELECOM/MSG/INBOX";
	char* msg_type_str = "SMS_CDMA";
	sprintf ((char*)buff, "<MAP-event-report version = \"1.0\">\n \
		<event type = \"%s\" handle = \"%s\" folder = \"%s\" msg_type = \"%s\" />\n \
		</MAP-event-report>",
		type_str, handle_str, folder_str, msg_type_str);
	hd.bs = buff;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY, hd, strlen((char*)buff) + 1, OBEX_FL_FIT_ONE_PACKET);

	OBEX_Request(handle, object);
	syncwait(handle);
}

//5.2 SetNotificationRegistration function
void put_client_SetNotificationRegistration(obex_t *handle)
{
	CDEBUG("");
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("PUT > ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_PUT)))	{
		CDEBUG("Error");
		return;
	}

	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = map_cmd_id_SetNotificationRegistration;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/MAP-NotificationRegistration";
	hd.bs = (uint8_t *) tp;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TYPE, hd, strlen(tp) + 1, OBEX_FL_FIT_ONE_PACKET);

	uint8_t para[100] = { 0 };
	int i = 0;
	para[i++] = 0x0E; // NotificationStatus
	para[i++] = 1;
	para[i++] = 0x01; 	// 0:off 1:on
	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);

	//Filler-byte 0x30
	uint8_t buff[10] = { 0 };
	buff[0] = 0x30;
	hd.bs = buff;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY, hd, 1, OBEX_FL_FIT_ONE_PACKET);

	OBEX_Request(handle, object);
	syncwait(handle);
}

//
void put_client_done(obex_t *handle, obex_object_t *object, int obex_rsp)
{
	CDEBUG();
	parseResponse(handle, object, obex_rsp);
}

//5.3 SetFolder function
void setpath_client_SetFolder(obex_t *handle, const char* path)
{
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("SETPATH> ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_SETPATH)))	{
		CDEBUG("Error");
		return;
	}
	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = map_cmd_id_SetFolder;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	if (path != NULL && strlen(path) > 0) {
		int upath_len = (strlen(path) + 1) * 2;
		uint8_t *upath = (uint8_t*)malloc(upath_len);
		int upath_size = OBEX_CharToUnicode((uint8_t *) upath, (uint8_t *) path, upath_len);
		hd.bs = (uint8_t *) upath;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_NAME, hd, upath_size, OBEX_FL_FIT_ONE_PACKET);
		free(upath);
	}

	obex_setpath_hdr_t sphdr;
	memset(&sphdr, 0, sizeof(obex_setpath_hdr_t));
	sphdr.flags = 0x02;
	OBEX_ObjectSetNonHdrData(object, (uint8_t *) &sphdr, 2);

	OBEX_Request(handle, object);
	syncwait(handle);
}

void setpath_client_done(obex_t *handle, obex_object_t *object, int obex_rsp)
{
	CDEBUG();
	parseResponse(handle, object, obex_rsp);
}


//5.4 GetFolderListing function
void get_client_GetFolderListing(obex_t *handle)
{
	CDEBUG("");
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("GET > ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_GET)))	{
		CDEBUG("Error");
		return;
	}
	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = map_cmd_id_GetFolderListing;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-obex/folder-listing";
	hd.bs = (uint8_t *) tp;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TYPE, hd, strlen(tp) + 1, OBEX_FL_FIT_ONE_PACKET);

	uint8_t para[100] = { 0 };
	int i = 0;

	para[i++] = 0x01; // maxlistcount
	para[i++] = 2;
	int maxCnt = 0xffff;
	para[i] = maxCnt >> 8;
	para[i + 1] = maxCnt & 0x00ff;
	i += 2;

	para[i++] = 0x02; // start offset
	para[i++] = 2;
	para[i] = 0x00; 	// high
	para[i + 1] = 0x00; // low
	i += 2;

	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);

	OBEX_Request(handle, object);
	syncwait(handle);
}

//5.5 GetMessagesListing function
void get_client_GetMessagesListing(obex_t *handle, const char* name)
{
	CDEBUG("");
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("GET > ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_GET)))	{
		CDEBUG("Error");
		return;
	}
	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = map_cmd_id_GetMessagesListing;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/MAP-msg-listing";
	hd.bs = (uint8_t *) tp;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TYPE, hd, strlen(tp) + 1, OBEX_FL_FIT_ONE_PACKET);

	if (name != NULL && strlen(name) > 0) {
		int uname_len = (strlen(name) + 1) * 2;
		uint8_t *uname = (uint8_t*)malloc(uname_len);
		int uname_size = OBEX_CharToUnicode((uint8_t *) uname, (uint8_t *) name, uname_len);
		hd.bs = (uint8_t *) uname;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_NAME, hd, uname_size, OBEX_FL_FIT_ONE_PACKET);
		free(uname);
	}

	uint8_t para[100] = { 0 };
	int i = 0;

	para[i++] = 0x01; // maxlistcount
	para[i++] = 2;
	int maxCnt = 0xffff;
	para[i] = maxCnt >> 8;
	para[i + 1] = maxCnt & 0x00ff;
	i += 2;

	para[i++] = 0x02; // start offset
	para[i++] = 2;
	para[i] = 0x00; 	// high
	para[i + 1] = 0x00; // low
	i += 2;

	para[i++] = 0x03; // FilterMessageType
	para[i++] = 1;
	para[i++] = 0x0f; 	// 0000xxx1 = "SMS_GSM" 0000xx1x = "SMS_CDMA" 0000x1xx = "EMAIL" 00001xxx = "MMS"

	//MaxListCount
	//ListStartOffset
	//SubjectLength
	//ParameterMask
	//FilterMessageType
	//FilterPeriodBegin
	//FilterPeriodEnd
	//FilterReadStatus
	//FilterRecipient
	//FilterOriginator
	//FilterPriority

	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);

	OBEX_Request(handle, object);
	syncwait(handle);
}

//5.6 GetMessage function
void get_client_GetMessage(obex_t *handle, const char* name)
{
	CDEBUG("");
	//int maxCnt, int order, const char* serchName, int searchAttr;
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("GET > ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_GET)))	{
		CDEBUG("Error");
		return;
	}
	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = map_cmd_id_GetMessage;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/message";
	hd.bs = (uint8_t *) tp;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TYPE, hd, strlen(tp) + 1, OBEX_FL_FIT_ONE_PACKET);

	int uname_len = (strlen(name) + 1) * 2;
	uint8_t *uname = (uint8_t*)malloc(uname_len);
	int uname_size = OBEX_CharToUnicode((uint8_t *) uname, (uint8_t *) name, uname_len);
	hd.bs = (uint8_t *) uname;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_NAME, hd, uname_size, OBEX_FL_FIT_ONE_PACKET);
	free(uname);

	uint8_t para[100] = { 0 };
	int i = 0;

	para[i++] = 0x0A; // Attachment
	para[i++] = 1;
	para[i++] = 0x00; 	// OFF

	para[i++] = 0x14; // charset
	para[i++] = 1;
	para[i++] = 0x01; 	// 0:native 1:UTF-8

/*
	para[i++] = 0x15; // FractionRequest
	para[i++] = 1;
	para[i++] = 0x00; 	// first
*/
	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);


	//application


	OBEX_Request(handle, object);
	syncwait(handle);
}

//
void get_client_done(obex_t *handle, obex_object_t *object, int obex_rsp, char *name)
{
	CDEBUG("");
	parseResponse(handle, object, obex_rsp);
}

//5.7 SetMessageStatus function
//5.8 PushMessage function
//5.9 UpdateInbox function

