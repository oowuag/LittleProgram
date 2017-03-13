#include <stdio.h>
#include <Windows.h>
 
#pragma comment(lib,"ws2_32.lib")
#define  PORT 4000
#define  IP_ADDRESS "172.26.11.99"
 
int main(int argc, char* argv[])
{
    WSADATA  Ws;
    SOCKET ClientSocket;
    struct sockaddr_in ClientAddr;
    int Ret = 0;
    char DataBuffer[MAX_PATH];
	int nDataSize = 0;

    /* Init Windows Socket */
    if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
    {
        printf("Init Windows Socket Failed::%d\n", GetLastError());
        return -1;
    }
 
    /* Create Socket */
    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( ClientSocket == INVALID_SOCKET )
    {
        printf("Create Socket Failed::%d\n", GetLastError());
        return -1;
    }

    ClientAddr.sin_family = AF_INET;
    ClientAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    ClientAddr.sin_port = htons(PORT);
    memset(ClientAddr.sin_zero, 0x00, 8);

	/* connect socket */
    Ret = connect(ClientSocket,(struct sockaddr*)&ClientAddr, sizeof(ClientAddr));
    if ( Ret == SOCKET_ERROR )
    {
        printf("Connect Error::%d\n", GetLastError());
        return -1;
    }
    else
    {
        printf("Connect succedded!\n");
    }

    while (1)
    {
		/* only send data - can do in thread
        scanf("%s", SendBuffer);
		// send data to server
        Ret = send(ClientSocket, SendBuffer, (int)strlen(SendBuffer), 0);
        if ( Ret == SOCKET_ERROR )
        {
            printf("Send Info Error::%d\n", GetLastError());
            break;
        }		
		if('q' == SendBuffer[0])
		{
			break;
		}
		*/

        nDataSize = recv(ClientSocket, DataBuffer, MAX_PATH, 0);
        if ( nDataSize < 0 )
        {
            printf("Receive Data Error::%d\n", GetLastError());
            break;
        }
        else if ( nDataSize == 0 )
        {
			printf("server shutdown ok. Error::%d\n", GetLastError());
            break;
        }

		// make sure end+1 is '\0'
		DataBuffer[nDataSize] = '\0';
		printf("Tick=%lu, Data=%s\n", GetTickCount(), DataBuffer);

    }
    
	/* close socket */
    closesocket(ClientSocket);
    WSACleanup();
 
    return 0;
 }
 