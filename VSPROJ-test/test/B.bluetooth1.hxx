#include <winsock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "irprops.lib")
using namespace std;

int main(){
    //Initialising winsock
    WSADATA data;
    int result;
    result = WSAStartup(MAKEWORD(2, 2), &data);//initializing winsock
    if (result!=0){
        cout << "An error occured while initialising winsock, closing....";
        exit(result);
    }

    //Initialising query for device
    WSAQUERYSET queryset;
    memset(&queryset, 0, sizeof(WSAQUERYSET));
    queryset.dwSize = sizeof(WSAQUERYSET);
    queryset.dwNameSpace = NS_BTH;

    HANDLE hLookup;
    result = WSALookupServiceBegin(&queryset, LUP_CONTAINERS, &hLookup);
    if (result!=0){
        cout << "An error occured while initialising look for devices, closing....";
        exit(result);
    }

    //Initialisation succedd, start looking for devices
    BYTE buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    DWORD bufferLength = sizeof(buffer);
    WSAQUERYSET *pResults = (WSAQUERYSET*)&buffer;
    while (result==0){
        result = WSALookupServiceNext(hLookup, LUP_RETURN_NAME | LUP_CONTAINERS | LUP_RETURN_ADDR | LUP_FLUSHCACHE |                 LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE,&bufferLength, pResults);
        if(result==0){// A device found
            //print the name of the device
            LPTSTR s = pResults->lpszServiceInstanceName;
            wcout << s << "found. quering for services\n";
            //Initialise quering the device services
            WSAQUERYSET queryset2;
            memset(&queryset2, 0, sizeof(queryset2));
            queryset2.dwSize = sizeof(queryset2);
            queryset2.dwNameSpace = NS_BTH;
            queryset2.dwNumberOfCsAddrs = 0;
            CSADDR_INFO * addr = (CSADDR_INFO *)pResults->lpcsaBuffer;
            WCHAR addressAsString[1000];
            DWORD addressSize = sizeof(addressAsString);
            WSAAddressToString(addr->RemoteAddr.lpSockaddr,addr->RemoteAddr.iSockaddrLength,NULL,addressAsString, &addressSize);
            queryset2.lpszContext = addressAsString;
            GUID protocol = L2CAP_PROTOCOL_UUID;
            queryset2.lpServiceClassId = &protocol;
            HANDLE hLookup2;
            int result2 = WSALookupServiceBegin(&queryset2, LUP_FLUSHCACHE |LUP_RETURN_NAME | |LUP_RETURN_TYPE| LUP_RETURN_BLOB | LUP_RETURN_COMMENT, &hLookup2);
            if (result2 !=0){
                cout << "An error occured while initializing query for services";
                exit(result);
            }

            //Start quering for device services
            while(result2 ==0){
                BYTE buffer2[4096];
                memset(buffer2, 0, sizeof(buffer2));
                DWORD bufferLength2 = sizeof(buffer2);
                WSAQUERYSET *pResults2 = (WSAQUERYSET*)&buffer2;
                result2 = WSALookupServiceNext(hLookup2,LUP_FLUSHCACHE |LUP_RETURN_NAME | LUP_RETURN_TYPE | LUP_RETURN_ADDR | LUP_RETURN_BLOB | LUP_RETURN_COMMENT,&bufferLength2,pResults2);
                if(result2 == 0)
                    wcout << "Service found: " << pResults2->lpszServiceInstanceName <<"\n";

            }
        }
    }
    return 0; 
}