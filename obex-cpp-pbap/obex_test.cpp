/**
	\file apps/obex_test.c
	Test IrOBEX, TCPOBEX and OBEX over cable to R320s.
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

#ifdef HAVE_BLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#endif

#include <openobex/obex.h>

#include "sdp_browse.h"
#include "obex_test.h"
#include "obex_test_client.h"


#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

#ifndef in_addr_t
#define in_addr_t unsigned long
#endif

#define TRUE  1
#define FALSE 0

#define IR_SERVICE "OBEX"
#define BT_CHANNEL 4


bdaddr_t BDADDR_ANY_const = {0, 0, 0, 0, 0, 0};

struct Bt_addr_name
{
	std::string addr;
	std::string name;
};

int hci_scan(std::vector<Bt_addr_name>& btVec)
{
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };

    Bt_addr_name item;

    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
        	strcpy(name, "[unknown]");
        //printf("[%d] %s  %s\n", i, addr, name);
        item.addr = addr;
        item.name = name;
        btVec.push_back(item);
    }

    free( ii );
    close( sock );
    return 0;
}

//
// Called by the obex-layer when some event occurs.
//
void obex_event(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp)
{
	switch (event)	{
	case OBEX_EV_PROGRESS:
		CDEBUG("Made some progress...");
		break;

	case OBEX_EV_ABORT:
		CDEBUG("Request aborted!");
		break;

	case OBEX_EV_REQDONE:
		//if(mode == OBEX_MODE_CLIENT) {
		CDEBUG("client_done");
		client_done(handle, object, obex_cmd, obex_rsp);
		break;
	case OBEX_EV_REQHINT:
		/* Accept any command. Not rellay good, but this is a test-program :) */
		CDEBUG("OBEX_ObjectSetRsp");
		OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
		break;

	case OBEX_EV_REQ:
		CDEBUG("server_request");
		break;

	case OBEX_EV_LINKERR:
		OBEX_TransportDisconnect(handle);
		CDEBUG("Link broken!");
		break;

	case OBEX_EV_STREAMEMPTY:
		CDEBUG("stream empty");
		fillstream(handle, object);
		break;

	default:
		CDEBUG("Unknown event %02x!", event);
		break;
	}
}

//
int main (int argc, char *argv[])
{
	char cmd[10] = { 0 };
	int num, end = 0;
	obex_t *handle;
	struct context global_context = {0};

	//AgiPhone
	char dest_addr[] = "CC:08:8D:AC:3A:6E";

	// HTC TSL M9
	//const char *dest_addr = "90:E7:C4:EE:D0:A7";

	// HTC COM M9
	//const char *dest_addr = "04:C2:3E:1D:84:20";

	// Galaxy-S3
	//const char dest_addr[] = "22:19:59:1C:FA:BE";

	// Galaxy-S5
	//const char dest_addr[] = "E4:12:1D:52:2E:EA";

	// Sony Xperia Z2
	//const char *dest_addr = "30:A8:DB:4F:1F:B3";

	/*
	std::vector<Bt_addr_name> btScanVec;
	hci_scan(btScanVec);
	int i = 0;
	for (std::vector<Bt_addr_name>::iterator it = btScanVec.begin(); it != btScanVec.end(); ++it, ++i)
	{
		printf("[%d] %s  %s\n", i, it->addr.c_str(), it->name.c_str());
	}
	char dest_addr[18] = {0};
	int select = 0;
	printf("> ");
	scanf("%d", &select);
	strcpy(dest_addr, btScanVec[select].addr.c_str());
	*/

	int channel = 13;
	if (argc >= 1) {
		if (argc == 2) {
			strcpy(dest_addr, argv[1]);
		}
	    if (argc >= 3) {
	    	channel = atoi(argv[2]);
	    }
	    else {
	    	char profileId[] = "1130";
			channel = sdp_browse(dest_addr, profileId);
	    }
	}

	bdaddr_t bdaddr;
	str2ba(dest_addr, &bdaddr);

	printf("Using Bluetooth RFCOMM transport addr : %s\n", dest_addr);
	if(! (handle = OBEX_Init(OBEX_TRANS_BLUETOOTH, obex_event, 0)))      {
		perror( "OBEX_Init failed");
		exit(0);
	}

	OBEX_SetUserData(handle, &global_context);

	printf( "OBEX Interactive test client/server.\n");

	memset(cmd, 0, sizeof(cmd));
	//strcpy(cmd, "c123q");
	//strcpy(cmd, "c123q");
	char ch[100];

	int autotest = 1;
	if (autotest && cmd[0] == '\0') {
		autotest = 0;
	}

	while (!end)
	{
		printf("> ");
		if (!autotest) {
			scanf("%s", cmd);
		}
		int i = 0;
		for (; i < strlen(cmd); ++i) {
			if (cmd[i] != '\0') {
				switch (cmd[i])	{
					case 'q':
						end=1;
					break;

					case 'c':
						if (bacmp(&bdaddr, &BDADDR_ANY_const) == 0) {
							printf("Device address error! (Bluetooth)\n");
							break;
						}
						if(BtOBEX_TransportConnect(handle, &BDADDR_ANY_const, &bdaddr, channel) <0) {
							printf("Transport connect error! (Bluetooth)\n");
							autotest = 0;
							memset(cmd, 0, sizeof(cmd));
							break;
						}
						// Now send OBEX-connect.
						connect_client(handle);
					break;

					case '1': // phonebook list
						setpath_client_SetPhoneBook(handle, "");
						setpath_client_SetPhoneBook(handle, "telecom");
						get_client_PullvCardListing(handle, "pb", NULL);
					break;

					case '2': // all phonebook
						get_client_PullPhoneBook(handle, "telecom/pb.vcf");
					break;

					case '3': // all callhistory
						get_client_PullPhoneBook(handle, "telecom/cch.vcf");
					break;

					case '4': // search vcard-list
						printf("input search num:");
						scanf("%s", ch);
						printf("get:[%s]\n", ch);
						setpath_client_SetPhoneBook(handle, "");
						setpath_client_SetPhoneBook(handle, "telecom");
						setpath_client_SetPhoneBook(handle, "pb");
						get_client_PullvCardListing(handle, NULL, ch);
					break;

					case '5': // vcard entry
						printf("input vcf:");
						scanf("%s", ch);
						printf("get:[%s]\n", ch);
						setpath_client_SetPhoneBook(handle, "");
						setpath_client_SetPhoneBook(handle, "telecom");
						setpath_client_SetPhoneBook(handle, "pb");
						get_client_PullvCardEntry(handle, ch);
					break;

					case 'd':
						disconnect_client(handle);
					break;

					default:
						printf("Unknown command %s\n", cmd);
				}
			}
		}
	}

	return 0;
}
