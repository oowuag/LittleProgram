/*
* @(#)$Id: SnsLogPrinter.cpp 36453 2013-08-14 02:50:26Z ag+++ $
* @(#)SnsLogPrinter class
*
* (c)  PIONEER CORPORATION  2010
*
* All Rights Reserved.
*/

/*===========================================================================*/
/* Precompiled header file for WIN */


/*===========================================================================*/
/* Header file include */


#ifndef CXX_SNSLOGPRINTER_H
#   include "SnsLogPrinter.h"
#endif

#include <string.h>

/*===========================================================================*/
/* Static data member */

SnsLogPrinter*	SnsLogPrinter::s_pcInstance = NULL;


#define SNSLOG(...) fprintf(SnsLogPrinter::Instance()->GetLogFile(), __VA_ARGS__)

/*---------------------------------------------------------------------------*/
// Get instance
SnsLogPrinter* SnsLogPrinter::Instance(void)
{
	if (s_pcInstance == NULL)
	{
		s_pcInstance = new SnsLogPrinter();
	}

	return s_pcInstance;
}

/*---------------------------------------------------------------------------*/
// Destroy all
void SnsLogPrinter::Destroy(void)
{
	if (s_pcInstance != NULL)
	{
		delete s_pcInstance;
		s_pcInstance = NULL;
	}
}

void SnsLogPrinter::Stop()
{
	if (true == m_bFileOpened) {
		fclose(m_pfFile);
		m_bFileOpened = false;
		m_pfFile = NULL;
	}
}

/*---------------------------------------------------------------------------*/
// Update the GPS last parameter
FILE* SnsLogPrinter::GetLogFile()
{
	if (false == m_bFileOpened) {
		m_pfFile = NULL;
		// open file
		m_pfFile = fopen(m_szFileName, "w");
		if (NULL != m_pfFile) {
			m_bFileOpened = true;
		}
	}
	return m_pfFile;
}

void SnsLogPrinter::SetFileName(const char* pszFileName, unsigned int dwSize)
{
	Stop();
	memset(m_szFileName, 0, sizeof(m_szFileName));
	if (dwSize == 0) {
		memcpy(m_szFileName, pszFileName, strlen(pszFileName));
	}
	else {
		memcpy(m_szFileName, pszFileName, dwSize);
	}
	//strcat(m_szFileName, "_Print.csv");
}

void SnsLogPrinter::WriteLog(const char* pszBuff, unsigned int dwSize)
{

	if(GetLogFile() == NULL)
	{
		return;
	}


	if(dwSize > 0)
	{
		if(m_nBufferLoc + (dwSize + 1) >= MAX_BUFFER_SIZE)
		{
			fwrite(m_szBuff, 1, m_nBufferLoc, GetLogFile());
			fprintf(GetLogFile(), "######\n");
			memset(m_szBuff, 0, sizeof(m_szBuff));
			m_nBufferLoc = 0;
		}


		for(int i = 0; i < dwSize; i++)
		{
			m_szBuff[m_nBufferLoc+i] = pszBuff[i];
		}
		m_nBufferLoc += dwSize;
		
	}
	else if(dwSize == 0)
	{
		dwSize = strlen(pszBuff);

		if(m_nBufferLoc + (dwSize + 1) >= MAX_BUFFER_SIZE)
		{
			fwrite(m_szBuff, 1, m_nBufferLoc, GetLogFile());
			fprintf(GetLogFile(), "######\n");
			memset(m_szBuff, 0, sizeof(m_szBuff));
			m_nBufferLoc = 0;
		}
		
		sprintf(&m_szBuff[m_nBufferLoc], "%s", pszBuff);
		m_nBufferLoc += dwSize;
	}
}



/*---------------------------------------------------------------------------*/
// Constructor
SnsLogPrinter::SnsLogPrinter(void)
: m_pfFile(NULL)
, m_bFileOpened(false)
, m_nBufferLoc(0)
{
	memset(m_szFileName, 0, sizeof(m_szFileName));
	memset(m_szBuff, 0, sizeof(m_szBuff));
}

/*---------------------------------------------------------------------------*/
// Destructor
SnsLogPrinter::~SnsLogPrinter(void)
{
	Stop();
}
