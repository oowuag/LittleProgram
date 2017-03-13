/**
	\file apps/obex_test_client.h
 	Client OBEX Commands.
	OpenOBEX test applications and sample code.

 */

#ifndef OBEX_TEST_CLIENT_H
#define OBEX_TEST_CLIENT_H

#include "obex_test.h"

void client_done(obex_t *handle, obex_object_t *object, int obex_cmd, int obex_rsp);

void parseResponse(obex_t *handle, obex_object_t *object, int obex_rsp);

void connect_client(obex_t *handle);
void connect_client_done(obex_t *handle, obex_object_t *object, int obex_rsp);

void disconnect_client(obex_t *handle);
void disconnect_client_done(obex_t *handle, obex_object_t *object, int obex_rsp);

//5.1 SendEvent function
void put_client_SendEvent(obex_t *handle);
//5.2 SetNotificationRegistration function
void put_client_SetNotificationRegistration(obex_t *handle);
//5.3 SetFolder function
void setpath_client_SetFolder(obex_t *handle, const char* path);
//5.4 GetFolderListing function
void get_client_GetFolderListing(obex_t *handle);
//5.5 GetMessagesListing function
void get_client_GetMessagesListing(obex_t *handle, const char* name);
//5.6 GetMessage function
void get_client_GetMessage(obex_t *handle, const char* name);

void put_client_done(obex_t *handle, obex_object_t *object, int obex_rsp);
void setpath_client_done(obex_t *handle, obex_object_t *object, int obex_rsp);
void get_client_done(obex_t *handle, obex_object_t *object, int obex_rsp, char *name);

#endif
