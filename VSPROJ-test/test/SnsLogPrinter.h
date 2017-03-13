/*
* @(#)$Id: SnsLogPrinter.h 36453 2013-08-14 02:50:26Z ag+++ $
* @(#)SnsLogPrinter class
*
* (c)  PIONEER CORPORATION  2010
*
* All Rights Reserved.
*/
/** @file
* Gsensor process
*
*
* @attention C++
*/

#ifndef CXX_SNSLOGPRINTER_H
#define CXX_SNSLOGPRINTER_H

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

/*===========================================================================*/
/*  Head file include                                                        */

#include <stdio.h>

/*===========================================================================*/
/*  Value define                                                             */


/*===========================================================================*/
/*  Class define                                                             */
/**
* SnsLogPrinter class

*
*/

const int MAX_BUFFER_SIZE = 1024 * 4; //4KB

class SnsLogPrinter
{
public:

	/**
	* Get instance
	*
	* @param   none
	* @return  GpsHandler pointer
	*/
	static SnsLogPrinter* Instance(void);

	/**
	* Destroy all
	*
	* @param   none
	* @return  none
	*/
	static void	Destroy(void);

	void Stop();

	FILE* GetLogFile();

	void SetFileName(const char* pszFileName, unsigned int dwSize = 0);


	void WriteLog(const char* pszBuff, unsigned int dwSize = 0);

private:

	// Instance
	static SnsLogPrinter*		s_pcInstance;

	 /**
	* Constructor
	*
	* @param  none
	*/
	SnsLogPrinter();

	/**
	* Destructor
	*/
	virtual ~SnsLogPrinter();

    FILE*							m_pfFile;                                   ///< log file
	bool							m_bFileOpened;								///< file open flag
	char							m_szFileName[256];
	char							m_szBuff[MAX_BUFFER_SIZE];
	int								m_nBufferLoc;
};

#endif //#define CXX_SNSLOGPRINTER_H
