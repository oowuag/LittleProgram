#include <Windows.h>

#include <MMSystem.h>

class CPcmWaveOut
{
public:
	CPcmWaveOut(void);
	~CPcmWaveOut(void);

	BOOL					Open(DWORD cbPlay = 0, DWORD cbParam = 0);
	BOOL					Close();
	BOOL					Play(LPSTR lpData, size_t size);
	BOOL					Stop();
	BOOL					Pause();
	BOOL					Resume();

	BOOL					GetVolume(DWORD * pdwVol);
	BOOL					SetVolume(DWORD dwVol);

	BOOL					IsPlaying();
	BOOL					IsOpened();

	DWORD					GetPlayPos();

	VOID					SetWaveFormat(WAVEFORMATEX stWaveFmt);

private:
	static VOID CALLBACK 	WaveOutCallback(HWAVEOUT hWaveOut
		, UINT uMsg
		, DWORD dwUser
		, DWORD dwParam1
		, DWORD dwParam2);

private:
	HWAVEOUT					m_hWaveOut;
	WAVEHDR						m_stWaveHdr;
	WAVEFORMATEX				m_stWaveFmt;

	BOOL						m_bIsOpened;
	BOOL						m_bIsPlaying;

	DWORD						m_dwAllPlayTime;

	DWORD						m_cbPlay;
	DWORD						m_cbParam;
};
