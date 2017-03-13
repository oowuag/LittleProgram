#include <iostream>
#include <Windows.h>
using namespace std;

class NPSnsHandlerBase
{
public:
	NPSnsHandlerBase( LPCTSTR HandlerName )
	{}
};

class NPSnsHandler : public NPSnsHandlerBase
{
public:
	NPSnsHandler( LPCTSTR HandlerName );

	static NPSnsHandler	*GetSnsHandler( void ){ return m_pSnsHandler; };

	~NPSnsHandler();

private:
	static NPSnsHandler	*m_pSnsHandler;
};

NPSnsHandler* NPSnsHandler::m_pSnsHandler = NULL;

NPSnsHandler::NPSnsHandler( LPCTSTR HandlerName ): NPSnsHandlerBase( HandlerName )
{
	m_pSnsHandler = this;
}

NPSnsHandler::~NPSnsHandler(VOID)
{
}


int main()
{
	NPSnsHandler* pHandler = new NPSnsHandler("NPGpsSnsHandler");

	NPSnsHandler* mypHandler = NPSnsHandler::GetSnsHandler();


	delete pHandler;


	return 0;
}