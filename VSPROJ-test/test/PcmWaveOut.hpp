#include "PcmWaveOut.h"

#pragma comment (lib, "Winmm.lib")

CPcmWaveOut::CPcmWaveOut(void)
{
	m_hWaveOut  = NULL;
	m_bIsOpened = FALSE;
	m_bIsPlaying = FALSE;
	m_dwAllPlayTime = 0;

	m_stWaveFmt.wFormatTag  = WAVE_FORMAT_PCM;
	m_stWaveFmt.nChannels = 1;
	m_stWaveFmt.nSamplesPerSec = 22050;
	m_stWaveFmt.wBitsPerSample = 16;
	m_stWaveFmt.nBlockAlign = m_stWaveFmt.nChannels * (m_stWaveFmt.wBitsPerSample >> 3);
	m_stWaveFmt.nAvgBytesPerSec = m_stWaveFmt.nSamplesPerSec * m_stWaveFmt.nBlockAlign;
	m_stWaveFmt.cbSize = 0;
}

CPcmWaveOut::~CPcmWaveOut(void)
{
	if (m_bIsOpened)
	{
		waveOutClose(m_hWaveOut);
	}
}

BOOL CPcmWaveOut::Open(DWORD cbPlay, DWORD cbParam)
{
	if (!m_bIsOpened) 
	{
		MMRESULT err;

		err = waveOutOpen(&m_hWaveOut, 0, &m_stWaveFmt, (DWORD)WaveOutCallback, 
			(DWORD)this, WAVE_MAPPED | CALLBACK_FUNCTION);
		if (err != MMSYSERR_NOERROR)
		{
			return FALSE;
		}

		ZeroMemory(&m_stWaveHdr, sizeof(m_stWaveHdr));
		err = waveOutPrepareHeader(m_hWaveOut, &m_stWaveHdr, sizeof(WAVEHDR));
		if (err != MMSYSERR_NOERROR)
		{
			waveOutClose(m_hWaveOut);
			m_hWaveOut = NULL;

			return FALSE;
		}

		m_cbPlay = cbPlay;
		m_cbParam = cbParam;

		m_bIsOpened = TRUE;
	}

	return TRUE;
}

BOOL CPcmWaveOut::Close()
{
	MMRESULT err = MMSYSERR_NOERROR;

	if (m_bIsPlaying)
	{
		Stop();
	}

	if (m_hWaveOut != NULL)
	{
		err = waveOutUnprepareHeader(m_hWaveOut, &m_stWaveHdr, sizeof(WAVEHDR));
		err = waveOutClose(m_hWaveOut);	
	}

	m_hWaveOut = NULL;
	m_bIsOpened = FALSE;

	return err == MMSYSERR_NOERROR;
}

BOOL CPcmWaveOut::Play( LPSTR lpData, size_t size )
{
	MMRESULT err = MMSYSERR_NOERROR;

	m_stWaveHdr.dwBufferLength = size;
	m_stWaveHdr.lpData = lpData;
	err = waveOutWrite(m_hWaveOut, &m_stWaveHdr, sizeof(m_stWaveHdr));

	if (err != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	m_bIsPlaying = TRUE;
	return err;
}

BOOL CPcmWaveOut::Stop()
{
	m_bIsPlaying = FALSE;
	return waveOutReset(m_hWaveOut) == MMSYSERR_NOERROR;
}

BOOL CPcmWaveOut::Pause()
{
	return waveOutPause(m_hWaveOut) == MMSYSERR_NOERROR;
}

BOOL CPcmWaveOut::Resume()
{
	return waveOutRestart(m_hWaveOut) == MMSYSERR_NOERROR;
}

BOOL CPcmWaveOut::GetVolume( DWORD * pdwVol )
{
	return waveOutGetVolume(m_hWaveOut, pdwVol) == MMSYSERR_NOERROR;
}

BOOL CPcmWaveOut::SetVolume( DWORD dwVol )
{
	return waveOutSetVolume(m_hWaveOut, dwVol) == MMSYSERR_NOERROR;
}

DWORD CPcmWaveOut::GetPlayPos()
{
	MMTIME stTime;
	stTime.wType = TIME_BYTES;
	waveOutGetPosition(m_hWaveOut, &stTime, sizeof(stTime));

	return stTime.u.cb;
}

BOOL CPcmWaveOut::IsPlaying()
{
	return m_bIsPlaying;
}

BOOL CPcmWaveOut::IsOpened()
{
	return m_bIsOpened;
}

VOID CPcmWaveOut::SetWaveFormat( WAVEFORMATEX stWaveFmt )
{
	m_stWaveFmt = stWaveFmt;
}

VOID CALLBACK CPcmWaveOut::WaveOutCallback( HWAVEOUT hWaveOut , UINT uMsg , DWORD dwUser , DWORD dwParam1 , DWORD dwParam2 )
{
	CPcmWaveOut	* pPcmWaveOut = (CPcmWaveOut *)dwUser;

	switch (uMsg)
	{
	case WOM_DONE:
		/*pPcmWaveOut->m_bIsPlaying = FALSE;*/
		if (pPcmWaveOut->m_bIsPlaying && pPcmWaveOut->m_cbPlay != 0)
		{
			(*(VOID (*)(DWORD, DWORD))pPcmWaveOut->m_cbPlay)(uMsg, pPcmWaveOut->m_cbParam);
		}
		pPcmWaveOut->m_bIsPlaying = FALSE;
		break;
	}
}
