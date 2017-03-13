//
// serial.c / serial.cpp
// A simple serial port writing example
// Written by Ted Burke - last updated 13-2-2013
//
// To compile with MinGW:
//
//      gcc -o serial.exe serial.c
//
// To compile with cl, the Microsoft compiler:
//
//      cl serial.cpp
//
// To run:
//
//      serial.exe
//
 
#include <windows.h>
#include <process.h>
#include <stdio.h>
 

#define COM_CMD_COMMON_PRE         0x01,0x02,0x03,0x04,0x05,0x06,0x01

// B+ ACC PKB REV IG ILL+
const unsigned char COM_CMD_BUP_EN[]     = { 0x10, COM_CMD_COMMON_PRE, 0x04, 0x00 };
const unsigned char COM_CMD_BUP_DE[]     = { 0x10, COM_CMD_COMMON_PRE, 0x04, 0x01 };

const unsigned char COM_CMD_SPD_VA[]     = { 0x22, COM_CMD_COMMON_PRE, 0x01, 0x3C };

bool ATE_WriteComData(HANDLE hSerial, const unsigned char *buf, int len)
{
    if (NULL == buf || len < 0) {
        return false;
    }

	unsigned char * writeBuffer = NULL;
	DWORD 	bytesWritten = 0;
	bool	ret = true;

  	int bufferSize = (len + 5) * 2; // head2+tail2+cs1
	//m_status|= portvalue;
	writeBuffer = new unsigned char[bufferSize];
    memset(writeBuffer, 0, bufferSize);

    do {
        int cur = 0;
        // head+2
	    writeBuffer[cur++] = 0x9F;
	    writeBuffer[cur++] = 0x02;
        int cs = 0;
        for (int i = 0; i < len; ++i) {
            writeBuffer[cur++] = buf[i];
            cs ^= buf[i];
            if (buf[i] == 0x9F) {
                writeBuffer[cur++] = 0x9F;
            }
        }
        writeBuffer[cur++] = cs; // cs+1
        if (cs == 0x9F) {
            writeBuffer[cur++] = 0x9F;
        }
        // tail+2
        writeBuffer[cur++] = 0x9F;
        writeBuffer[cur++] = 0x03;

        if (cur > bufferSize) {
            ret = false;
            break;
        }

        if(!WriteFile(hSerial, writeBuffer, cur, &bytesWritten, NULL))
        {
            fprintf(stderr, "Error\n");
            ret = false;
        } 
    }while(0);

    delete []writeBuffer;

    Sleep(2); // wait at least 16ms??

    return ret;
}

UINT WINAPI ListenThread( void* pParam )  
{  
    HANDLE hSerial = reinterpret_cast<HANDLE>(pParam);  
 
    while (1) {
        unsigned char cRecved = 0;
        DWORD BytesRead = 0;
        BOOL bResult = ReadFile(hSerial, &cRecved, 1, &BytesRead, NULL);
        if (!bResult) {  
            DWORD dwError = GetLastError();
            fprintf(stderr, "ReadFile Error:%d\n", dwError);
            return 1;  
        }
        if (BytesRead > 0) {
            fprintf(stderr, "%02X ", cRecved);
        }
    }
 
    return 0;  
}  

int main()
{

    // Declare variables and structures
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
         
    // Open the highest available serial port number
    fprintf(stderr, "Opening serial port...");
    hSerial = CreateFile(
                "\\\\.\\COM3", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hSerial == INVALID_HANDLE_VALUE)
    {
            fprintf(stderr, "Error\n");
            return 1;
    }
    else fprintf(stderr, "OK\n");
     
    // Set device parameters (38400 baud, 1 start bit,
    // 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return 1;
    }
     
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = EVENPARITY;
    if(SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(hSerial);
        return 1;
    }
 
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(SetCommTimeouts(hSerial, &timeouts) == 0)
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }

    UINT threadId = 0;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, hSerial, 0, &threadId);

 
    // Send specified text (remaining command line arguments)
    fprintf(stderr, "Sending bytes...\n");
    if (!ATE_WriteComData(hSerial, COM_CMD_SPD_VA, sizeof(COM_CMD_SPD_VA))) {
        fprintf(stderr, "ATE_WriteComData error\n");
    }

    getchar();

    fprintf(stderr, "Sending bytes...\n");
    if (!ATE_WriteComData(hSerial, COM_CMD_SPD_VA, sizeof(COM_CMD_SPD_VA))) {
        fprintf(stderr, "ATE_WriteComData error\n");
    }

    getchar();

    CloseHandle(hThread);

    // Close serial port
    fprintf(stderr, "Closing serial port...");
    if (CloseHandle(hSerial) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");
 
    // exit normally
    return 0;
}