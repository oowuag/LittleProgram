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
#include "vcard_process.h"
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

#define OBEX_STREAM_CHUNK       4096

#pragma pack(1)
typedef struct obex_setpath_hdr {
    uint8_t  flags;
    uint8_t constants;
} obex_setpath_hdr_t;
#pragma pack()


uint8_t buffer[OBEX_STREAM_CHUNK];

int fileDesc;

static const uint FILTER_LO_NONE 	= 0x00000000;
static const uint FILTER_LO_VERSION = 0x00000001 << 0;  // vCard Version
static const uint FILTER_LO_FN 		= 0x00000001 << 1;  // Formatted Name
static const uint FILTER_LO_N 		= 0x00000001 << 2;  // Structured Presentation of Name
static const uint FILTER_LO_PHOTO 	= 0x00000001 << 3;  // Associated Image or Photo
static const uint FILTER_LO_BDAY 	= 0x00000001 << 4;  // Birthday
static const uint FILTER_LO_ADR 	= 0x00000001 << 5;  // Delivery Address
static const uint FILTER_LO_LABEL 	= 0x00000001 << 6;  // Delivery
static const uint FILTER_LO_TEL 	= 0x00000001 << 7;  // Tel Number
static const uint FILTER_LO_EMAIL 	= 0x00000001 << 8;  // Electronic Mail Address
static const uint FILTER_LO_MAILER 	= 0x00000001 << 9;  // Electronic Mail
static const uint FILTER_LO_TZ 		= 0x00000001 << 10;  // Time Zone
static const uint FILTER_LO_GEO 	= 0x00000001 << 11;  // Geographic Position
static const uint FILTER_LO_TITLE 	= 0x00000001 << 12;  // Job
static const uint FILTER_LO_ROLE 	= 0x00000001 << 13;  // Role within the Organization
static const uint FILTER_LO_LOGO 	= 0x00000001 << 14;  // Organization Logo
static const uint FILTER_LO_AGENT 	= 0x00000001 << 15;  // vCard of Person Representing
static const uint FILTER_LO_ORG 	= 0x00000001 << 16;  // Name of Organization
static const uint FILTER_LO_NOTE 	= 0x00000001 << 17;  // Comments
static const uint FILTER_LO_REV 	= 0x00000001 << 18;  // Revision
static const uint FILTER_LO_SOUND 	= 0x00000001 << 19;  // Pronunciation of Name
static const uint FILTER_LO_URL 	= 0x00000001 << 20;  // Uniform Resource Locato
static const uint FILTER_LO_UID 	= 0x00000001 << 21;  // Unique ID
static const uint FILTER_LO_KEY 	= 0x00000001 << 22;  // Public Encryption Key
static const uint FILTER_LO_NICKNAME = 0x00000001 << 23;  // Nickname
static const uint FILTER_LO_CATEGORIES = 0x00000001 << 24;  // Categories
static const uint FILTER_LO_PROID 	= 0x00000001 << 25;  // Product ID
static const uint FILTER_LO_CLASS 	= 0x00000001 << 26;  // Class information
static const uint FILTER_LO_SORTSTRING = 0x00000001 << 27;  // String used for sorting operations
static const uint FILTER_LO_XIRMCCALLDATETIME = 0x00000001 << 28;  // Time stamp


enum pbap_cmd_id_type {
	pbap_cmd_id_pullvcardlist = 0,
	pbap_cmd_id_pullphonebook,
	pbap_cmd_id_pullvcardentry,
	pbap_cmd_id_setphonebook
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
		ret = OBEX_HandleInput(handle, 3);
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

//
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
				struct context *gt;
				gt = (context *)OBEX_GetUserData(handle);
				gt->connectionid = hv.bq4;
			break;

			case OBEX_HDR_WHO:
			{
				char who[200];
				OBEX_UnicodeToChar((uint8_t *)who, hv.bs, hlen);
				CDEBUG("OBEX_HDR_WHO = %*.*s", hlen/2, hlen/2, who);
			}
			break;

			case OBEX_HDR_BODY:
				CDEBUG("Found body");
				body = hv.bs;
				body_len = hlen;
				gt = (context *)OBEX_GetUserData(handle);
				if (gt->client_cmd_id == pbap_cmd_id_pullphonebook ||
					gt->client_cmd_id == pbap_cmd_id_pullvcardentry) {
					
					CDEBUG("%*.*s", body_len, body_len, body);
					CDEBUG("########################");
					vcardProcess((char*)body, body_len);
				}
				else if (gt->client_cmd_id == pbap_cmd_id_pullvcardlist) {
					parseVcardXmlString((char*)body, body_len);
				}
				else {
					CDEBUG("%*.*s", body_len, body_len, body);
				}
			break;

			case OBEX_HDR_APPARAM:
				body = hv.bs;
				body_len = hlen;
				CDEBUG("OBEX_HDR_APPARAM: %*.*s", body_len, body_len, body);
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

	hd.bs = (uint8_t*)"\x79\x61\x35\xf0\xf0\xc5\x11\xd8\x09\x66\x08\x00\x20\x0c\x9a\x66";
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

//5.1 PullPhoneBook Function
void get_client_PullPhoneBook(obex_t *handle, const char* name)
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
	CDEBUG("connection_id:%d", gt->connectionid);
	gt->client_cmd_id = pbap_cmd_id_pullphonebook;

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/phonebook";
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

	para[i++] = 0x06; // Filter
	para[i++] = 8;
	uint lowfilter = FILTER_LO_VERSION | FILTER_LO_FN | FILTER_LO_N
					| FILTER_LO_PHOTO | FILTER_LO_TEL | FILTER_LO_EMAIL;
	if (strcmp(&name[strlen(name) - 7], "cch.vcf") == 0) {
		CDEBUG("call history filter");
		lowfilter = FILTER_LO_VERSION | FILTER_LO_TEL | FILTER_LO_XIRMCCALLDATETIME | FILTER_LO_TZ;
	}
	uint highfilter = 0;
	para[i]     = highfilter & 0xff000000 >> 24;
	para[i + 1] = highfilter & 0x00ff0000 >> 16;
	para[i + 2] = highfilter & 0x0000ff00 >> 8;
	para[i + 3] = highfilter & 0x000000ff;
	i += 4;
	para[i]     = lowfilter & 0xff000000 >> 24;
	para[i + 1] = lowfilter & 0x00ff0000 >> 16;
	para[i + 2] = lowfilter & 0x0000ff00 >> 8;
	para[i + 3] = lowfilter & 0x000000ff;
	i += 4;

	para[i++] = 0x07; // Format vcard version
	para[i++] = 1;
	para[i++] = 0; // 0x00 = 2.1, 0x01 = 3.0

	para[i++] = 0x04; // MaxListCount
	para[i++] = 2;
	int maxCnt = 0xffff;
	para[i] = maxCnt >> 8;
	para[i + 1] = maxCnt & 0x00ff;
	i += 2;

	para[i++] = 0x05; // ListStartOffset
	para[i++] = 2;
	para[i] = 0x00; 	// high
	para[i + 1] = 0x00; // low
	i += 2;

	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);


	OBEX_Request(handle, object);
	syncwait(handle);
}

//5.4 PullvCardEntry Function
void get_client_PullvCardEntry(obex_t *handle, const char* name)
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
	gt->client_cmd_id = pbap_cmd_id_pullvcardentry;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/vcard";
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

	para[i++] = 0x06; // Filter
	para[i++] = 8;
	uint lowfilter = FILTER_LO_VERSION | FILTER_LO_FN | FILTER_LO_N
					| FILTER_LO_PHOTO | FILTER_LO_TEL | FILTER_LO_EMAIL;
	uint highfilter = 0;
	para[i]     = highfilter & 0xff000000 >> 24;
	para[i + 1] = highfilter & 0x00ff0000 >> 16;
	para[i + 2] = highfilter & 0x0000ff00 >> 8;
	para[i + 3] = highfilter & 0x000000ff;
	i += 4;
	para[i]     = lowfilter & 0xff000000 >> 24;
	para[i + 1] = lowfilter & 0x00ff0000 >> 16;
	para[i + 2] = lowfilter & 0x0000ff00 >> 8;
	para[i + 3] = lowfilter & 0x000000ff;
	i += 4;

	para[i++] = 0x07; // Format vcard version
	para[i++] = 1;
	para[i++] = 0; // 0x00 = 2.1, 0x01 = 3.0

	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);

	OBEX_Request(handle, object);
	syncwait(handle);
}

//5.3 PullvCardListing Function
void get_client_PullvCardListing(obex_t *handle, const char* name, const char* searchName)
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
	gt->client_cmd_id = pbap_cmd_id_pullvcardlist;
	CDEBUG("connection_id:%d", gt->connectionid);

	hd.bq4 = gt->connectionid;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION, hd, 4, OBEX_FL_FIT_ONE_PACKET);

	static const char* tp = "x-bt/vcard-listing";
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

	para[i++] = 0x01; // Order
	para[i++] = 1;
	para[i++] = 0x00; // 0x00 = indexed, 0x01 = alphanumeric, 0x02 = phonetic

	if (NULL != searchName) {
		CDEBUG("searchName:%s", searchName);
		para[i] = 0x02; // SearchValue
		i += 1;
		para[i] = strlen(searchName);
		int k = 1;
		for (; k <= para[i]; k++) {
			para[i + k] = searchName[k - 1];
		}
		i += para[i];
		i += 1;

		para[i] = 0x03; // SearchAttribute
		i += 2;
		para[i] = 0x01; // 0x00= Name, 0x01= Number, 0x02= Sound
		i += 1;
	}

	para[i++] = 0x04; // MaxListCount
	para[i++] = 2;
	int maxCnt = 0xffff;
	para[i] = maxCnt >> 8;
	para[i + 1] = maxCnt & 0x00ff;
	i += 2;

	para[i++] = 0x05; // ListStartOffset
	para[i++] = 2;
	para[i] = 0x00; 	// high
	para[i + 1] = 0x00; // low
	i += 2;


	hd.bs = (uint8_t *) para;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_APPARAM, hd, i, OBEX_FL_FIT_ONE_PACKET);

	OBEX_Request(handle, object);
	syncwait(handle);
}

void get_client_done(obex_t *handle, obex_object_t *object, int obex_rsp, char *name)
{
	CDEBUG("");
	parseResponse(handle, object, obex_rsp);
}

//5.2 SetPhoneBook Function
void setpath_client_SetPhoneBook(obex_t *handle, const char* path)
{
	obex_object_t *object;
	obex_headerdata_t hd;

	CDEBUG("SETPATH> ");

	if(! (object = OBEX_ObjectNew(handle, OBEX_CMD_SETPATH)))	{
		CDEBUG("Error");
		return;
	}
	context *gt = (context *)OBEX_GetUserData(handle);
	gt->client_cmd_id = pbap_cmd_id_setphonebook;
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


//
//
int fillstream(obex_t *handle, obex_object_t *object)
{
	int                    actual;
	obex_headerdata_t       hv;

	CDEBUG("Filling stream!");

	actual = read(fileDesc, buffer, OBEX_STREAM_CHUNK);
	if(actual > 0) {
		/* Read was ok! */
		hv.bs = buffer;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY,
				hv, actual, OBEX_FL_STREAM_DATA);
	}
	else if(actual == 0) {
		/* EOF */
		hv.bs = buffer;
		close(fileDesc); fileDesc = -1;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY,
				hv, 0, OBEX_FL_STREAM_DATAEND);
	}
	else {
		/* Error */
		hv.bs = NULL;
		close(fileDesc); fileDesc = -1;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY,
				hv, 0, OBEX_FL_STREAM_DATA);
	}

	return actual;
}


void put_client(obex_t *handle)
{
	obex_object_t *object;

	char lname[200];
	char rname[200];
	unsigned int rname_size;
	obex_headerdata_t hd;
	int num;

	uint8_t *buf;
	int file_size;

	CDEBUG("PUT file (local, remote)> ");
	num = scanf("%s %s", lname, rname);

	CDEBUG("Going to send %d bytes", file_size);

	/* Build object */
	object = OBEX_ObjectNew(handle, OBEX_CMD_PUT);

	rname_size = OBEX_CharToUnicode((uint8_t *) rname, (uint8_t *) rname, sizeof(rname));
	hd.bq4 = file_size;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_LENGTH, hd, 4, 0);

	hd.bs = (uint8_t *) rname;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_NAME, hd, rname_size, 0);

	hd.bs = buf;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY, hd, file_size, 0);

	free(buf);

 	OBEX_Request(handle, object);
	syncwait(handle);
}

void put_client_done(obex_t *handle, obex_object_t *object, int obex_rsp)
{
	CDEBUG();
	parseResponse(handle, object, obex_rsp);
}