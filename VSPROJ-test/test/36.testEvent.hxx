#include <Windows.h>
#include <iostream>
using namespace std;

int main()
{
	HANDLE m_hGpsEvent_create = CreateEvent(NULL, FALSE, FALSE, TEXT("GPS_EVENT"));


	HANDLE m_hGpsEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("GPS_EVENT"));
	if (m_hGpsEvent == NULL) {
		return -1;
	}


	if(m_hGpsEvent != NULL){
		CloseHandle(m_hGpsEvent);
		m_hGpsEvent = NULL;
	}

	m_hGpsEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("GPS_EVENT"));
	if (m_hGpsEvent == NULL) {
		return -1;
	}


	if(m_hGpsEvent != NULL){
		CloseHandle(m_hGpsEvent);
		m_hGpsEvent = NULL;
	}


	if(m_hGpsEvent_create != NULL){
		CloseHandle(m_hGpsEvent_create);
		m_hGpsEvent_create = NULL;
	}

	m_hGpsEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("GPS_EVENT"));
	if (m_hGpsEvent == NULL) {
		return -1;
	}


}