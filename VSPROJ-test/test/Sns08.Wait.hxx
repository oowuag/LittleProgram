#include <Windows.h>
#include <iostream>
using namespace std;

class NObject{
};

class NProcWaitObject: public NObject
{
protected:
	// -------------------------------------------------------------------------
	//! イベントハンドル
	HANDLE hEvent;

private:
	// -------------------------------------------------------------------------
	/*!
		コピーコンストラクタ
		@param src
			コピー元
		@return
			-
		@note
			使用不可
	*/
	NProcWaitObject(const NProcWaitObject& src){}
	// -------------------------------------------------------------------------
	/*!
		代入演算子
		@param src
			コピー元
		@return
			コピー後の自己参照
		@note
			使用不可
	*/
	NProcWaitObject& operator = (const NProcWaitObject& src){return *this;}
public:
	// -------------------------------------------------------------------------
	/*!
		コンストラクタ
		@return
			-
		@note
	*/
	NProcWaitObject()
	{};
	// -------------------------------------------------------------------------
	/*!
		コンストラクタ
		@param name
			ウェイトオブジェクト名称
		@param wait_once
			一度のみのWaitフラグ<br>
			TRUE: 一度#Notifyすると、#Resetするまでは#Waitで待たない<br>
			FALSE: #Waitから解放された後、再びNotifyされるまで#Waitで待機する。
		@return
			-
		@note
	*/
	explicit NProcWaitObject(LPCTSTR name, BOOL wait_once = FALSE)
	{};				// Mod: 2006.12.22 S.Kumagawa
	// -------------------------------------------------------------------------
	/*!
		デストラクタ
		@return
			-
		@note
	*/
	virtual ~NProcWaitObject()
	{};
	// -------------------------------------------------------------------------
	/*!
		ウェイトオブジェクトを生成する
		@param name
			ウェイトオブジェクト名称
		@param wait_once
			一度のみのWaitフラグ<br>
			TRUE: 一度#Notifyすると、#Resetするまでは#Waitで待たない<br>
			FALSE: #Waitから解放された後、再びNotifyされるまで#Waitで待機する。
		@return
			生成結果(TRUE: 生成成功)
		@note
			ウェイトオブジェクトを生成する。
		@sa
			Destroy()
	*/
	inline BOOL Create(LPCTSTR name, BOOL wait_once = FALSE);
	// -------------------------------------------------------------------------
	/*!
		ウェイトオブジェクトを破棄する
		@return
			-
		@note
			ウェイトオブジェクトを破棄する。
		@sa
			Create()
	*/
	inline void Destroy();
	// -------------------------------------------------------------------------
	/*!
		ウェイトする
		@param msec
			ウェイト時間(単位: msec  INFINITE: タイムアウトなし)
		@return
			ウェイト解除要因(TRUE: #Notifyによる解除  FALSE: その他タイムアウト等による解除)
		@note
		@sa
			Notify()
		@code
			// 例:
			// <プロセスA> ･･先に起動
			#include "NProcSyncObject.h"

			void FuncA(void)
			{
				NProcWaitObject nWait;

				nWait.Create(TEXT("FUNC_WAIT") );									// 同期オブジェクト生成
				OutputDebugString(TEXT("PROCESS A WAIT\r\n") );

				nWait.Wait();														// Wait開始

				OutputDebugString(TEXT("PROCESS A WAIT END\r\n") );
				nWait.Destroy();
			}

			// <プロセスB> ･･後から起動
			#include "NProcSyncObject.h"

			void FuncB(void)
			{
				NProcWaitObject nWait;

				nWait.Create(TEXT("FUNC_WAIT") );									// 同期オブジェクト生成
				OutputDebugString(TEXT("PROCESS B Notify\r\n") );

				nWait.Notify();														// Notify

				nWait.Destroy();
			}
		@endcode
	*/
	inline BOOL Wait(DWORD msec = INFINITE) const;
	// -------------------------------------------------------------------------
	/*!
		ウェイトを解除する
		@return
			-
		@note
			ウェイトを解除する。
		@sa
			Wait()
	*/
	inline void Notify() const;
	// -------------------------------------------------------------------------
	/*!
		ウェイト可能にする
		@return
			-
		@note
			シグナル状態をリセットする。<br>
			#Wait()を行う前に実行された#Notify()を無効とする場合には、
			#Wait()を実行する直前に本関数を用いてシグナル状態をリセットする。
		@sa
			Wait()<br>
			Notify()
	*/
	inline void Reset() const;
};

// *****************************************************************************
/*
Overview	:ウェイトオブジェクトを生成する
Arguments	:name						:ウェイトオブジェクト名称
			:wait_once					:一度のみのWaitフラグ
			:							:TRUE: 一度#Notifyすると、#Resetするまでは#Waitで待たない
			:							:FALSE: #Waitから解放された後、再びNotifyされるまで#Waitで待機する。
Return		:生成結果(TRUE: 生成成功)
Note		:ウェイトオブジェクトを生成する。
*/
inline BOOL NProcWaitObject::Create(LPCTSTR name, BOOL wait_once)
{
	if(hEvent != NULL) ::CloseHandle(hEvent);
	if( (hEvent = ::CreateEvent(NULL, wait_once, FALSE, name) ) == NULL) return FALSE;
	return TRUE;
}

// *****************************************************************************
/*
Overview	:ウェイトオブジェクトを破棄する
Arguments	:-
Return		:-
Note		:ウェイトオブジェクトを破棄する。
*/
inline void NProcWaitObject::Destroy()
{
	if(hEvent != NULL){
		::CloseHandle(hEvent);
		hEvent = NULL;
	}
}

// *****************************************************************************
/*
Overview	:ウェイトする
Arguments	:msec						:ウェイト時間(単位: msec  INFINITE: タイムアウトなし)
Return		:ウェイト解除要因(TRUE: #Notifyによる解除  FALSE: その他タイムアウト等による解除)
Note		:-
*/
inline BOOL NProcWaitObject::Wait(DWORD msec) const
{
	if(hEvent == NULL) return FALSE;
	if(::WaitForSingleObject(hEvent, msec) == WAIT_OBJECT_0) return TRUE;
	return FALSE;
}

// *****************************************************************************
/*
Overview	:ウェイトを解除する
Arguments	:-
Return		:-
Note		:ウェイトを解除する。
*/
inline void NProcWaitObject::Notify() const
{
	if(hEvent != NULL) ::SetEvent(hEvent);
}

// *****************************************************************************
/*
Overview	:ウェイト可能にする
Arguments	:-
Return		:-
Note		:シグナル状態をリセットする。
			:#Wait()を行う前に実行された#Notify()を無効とする場合には、
			:#Wait()を実行する直前に本関数を用いてシグナル状態をリセットする。
*/
inline void NProcWaitObject::Reset() const
{
	if(hEvent != NULL) ::ResetEvent(hEvent);
}

LPCTSTR NP_SNS_WAIT = "NP_SNS_WAIT";

VOID Run()
{
    NProcWaitObject			cWait;		// ウェイトオブジェクト

    // 同期オブジェクト作成	
    cWait.Create( NP_SNS_WAIT );

	int i = 0;
    while ( 1 ) {

		cout << i++ << endl;

        if( TRUE == cWait.Wait(200) ){

            /*** GPSハンドラからのNotify待ちの処理 ***/
            /* GPSハンドラからのGPS測位データの受信、デコード処理を行う */
            cout << "NPSnsHandlerBase::GetSnsHandlerBase()->GetGpsDataProc()" << endl ;
        }
    }
}

int main()
{
	NProcWaitObject m_cSNS_Wait;
	BOOL bRet = m_cSNS_Wait.Create( NP_SNS_WAIT );
	m_cSNS_Wait.Notify();
	m_cSNS_Wait.Notify();

	Run();

	return 0;
}