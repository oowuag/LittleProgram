/**
	\file apps/obex_test.h
	Test IrOBEX, TCPOBEX and OBEX over cable to R320s.
	OpenOBEX test applications and sample code.

 */

#ifndef OBEX_TEST_H
#define OBEX_TEST_H


#define CABLE_DEBUG

#if defined(CABLE_DEBUG)
	#define CDEBUG(format, ...) printf("%s(%d): " format "\n", __FUNCTION__ , __LINE__, ## __VA_ARGS__)
#else
	#define CDEBUG(format, ...)
#endif

struct context
{
	int serverdone;
	int clientdone;
	char *get_name;	/* Name of last get-request */
	int connectionid;
	int client_cmd_id;
};

#endif
