#include <Windows.h>
#include <iostream>
using namespace std;


/* this is the state of our connection */
typedef struct {
	// gps status
} GpsState;

typedef struct ptw32_handle_t{    
    void * p;                   /* Pointer to actual object */    
    unsigned int x;             /* Extra information - reuse count etc */ 
ptw32_handle_t()
{
	cout << "ptw32_handle_t" << endl;
}
} ;    
typedef ptw32_handle_t pthread_t;    

typedef pthread_t (* gps_create_thread)(const char* name, void (*start)(void *), void* arg);

/** GPS callback structure. */
typedef struct {
	/** set to sizeof(GpsCallbacks) */
	size_t      size;

	gps_create_thread create_thread_cb;
} GpsCallbacks;

void pgps_thread(void *pThreadData)
{
	cout << "pgps_thread" << endl;
}




int main()
{
	//GpsState *state = NULL;
	//GpsCallbacks callbacks = {0};
	//callbacks.create_thread_cb("gps thread", pgps_thread, state);

	char buff[] = {0x01,0x00,0x00,0x00};
	void *pbuff = buff;
	int x = *((int*)(buff));

	cout << x << endl;


	return 0;
}