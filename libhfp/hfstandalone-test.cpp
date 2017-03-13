/*
 * Software Hands-Free with Crappy UI
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <libhfp/hfp.h>
#include <libhfp/soundio.h>

#include <string>
#include <map>

using namespace std;
using namespace libhfp;


class MySocketNotifier;
std::map<int, MySocketNotifier*> g_socketNofity_ReadMap;
std::map<int, MySocketNotifier*> g_socketNofity_WriteMap;


class MyTimerNotifier : public TimerNotifier
{
	bool timer_set;
	int timer_id;
public:
	void timerEvent() {
		timer_set = false;
		(*this)(this);
	}
	virtual void Set(int msec) {
		assert(Registered());
		if (timer_set)
			Cancel();
		timer_id = 0;
		timer_set = true;
	}
	virtual void Cancel(void) {
		if (timer_set) {
			timer_id = 0;
			timer_set = false;
		}
	}
	MyTimerNotifier(void)
        : timer_set(false)
        , timer_id(0)
    {
    }
	virtual ~MyTimerNotifier()
    {
		Cancel();
	}
};
class MySocketNotifier : public SocketNotifier
{
public:
	MySocketNotifier(int fh, bool nwrite)
    {
        if (fh != 0) {
            if (nwrite) {
                g_socketNofity_WriteMap[fh] = this;
            }
            else {
                g_socketNofity_ReadMap[fh] = this;
            }
        }
	}
	virtual ~MySocketNotifier()
    {
	}

    void SocketNotify(int fh) { (*this)(this, fh); }

	virtual void SetEnabled(bool enable)
    {
	}
};


class MyEventDispatchInterface : public DispatchInterface {
public:

public:
	/*
	 * Socket event reporting
	 */
	virtual SocketNotifier *NewSocket(int fh, bool nwrite)
    {
        return new MySocketNotifier(fh, nwrite);
    }
	virtual TimerNotifier *NewTimer(void)
    {
        return new MyTimerNotifier;
    }
	virtual void LogVa(DispatchInterface::logtype_t lt, const char *fmt, va_list ap)
    {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
	}
	MyEventDispatchInterface() {}
};

static MyEventDispatchInterface g_qt_ei;



//------------------------------------------------------------------
// socket
int handleReadInput()
{
    if (g_socketNofity_ReadMap.size() == 0) {
        return -1;
    }

	struct timeval time;
	fd_set fdset;
	int highestfd = -1;

	time.tv_sec = 0;
	time.tv_usec = 0;

	/* Add the fd's to the set. */
	FD_ZERO(&fdset);
    std::map<int, MySocketNotifier*>::iterator it;
    for (it = g_socketNofity_ReadMap.begin(); it != g_socketNofity_ReadMap.end(); ++it) {
        int fd = it->first;

        FD_SET(fd, &fdset);

        if (highestfd < it->first) {
            highestfd = it->first;
        }
    }

	if (highestfd < 0) {
		printf("No valid socket is open\n");
		return -1;
	}

	int ret = select((int)highestfd+1, &fdset, NULL, NULL, &time);

	/* Check if this is a timeout (0) or error (-1) */
    if (ret < 1) {
		return ret;
    }

    for (it = g_socketNofity_ReadMap.begin(); it != g_socketNofity_ReadMap.end(); ++it) {
        int fd = it->first;

	    if (fd >= 0 && FD_ISSET(fd, &fdset)) {
		    printf("[%d] socket data received\n", fd);
            MySocketNotifier* p = it->second;
            if (p) {
                p->SocketNotify(fd);
            }
	    }
	    else {
	    }
    }

    return ret;
}

int handleWriteInput()
{
    if (g_socketNofity_WriteMap.size() == 0) {
        return -1;
    }

	struct timeval time;
	fd_set fdset;
	int highestfd = -1;

	time.tv_sec = 0;
	time.tv_usec = 0;

	/* Add the fd's to the set. */
	FD_ZERO(&fdset);
    std::map<int, MySocketNotifier*>::iterator it;
    for (it = g_socketNofity_WriteMap.begin(); it != g_socketNofity_WriteMap.end(); ++it) {
        int fd = it->first;

        FD_SET(fd, &fdset);

        if (highestfd < it->first) {
            highestfd = it->first;
        }
    }

	if (highestfd < 0) {
		printf("No valid socket is open\n");
		return -1;
	}

	int ret = select((int)highestfd+1, NULL, &fdset, NULL, &time);

	/* Check if this is a timeout (0) or error (-1) */
    if (ret < 1) {
		return ret;
    }

    for (it = g_socketNofity_WriteMap.begin(); it != g_socketNofity_WriteMap.end(); ++it) {
        int fd = it->first;

	    if (fd >= 0 && FD_ISSET(fd, &fdset)) {
		    printf("[%d] socket data received\n", fd);
            MySocketNotifier* p = it->second;
            if (p) {
                p->SocketNotify(fd);
            }
	    }
	    else {
	    }
    }

    return ret;
}


void* fd_process(void *x_void_ptr)
{
	while(1) {
        int ret1 = handleReadInput();
        int ret2 = handleWriteInput();
        usleep(10*1000);
	}
}


//------------------------------------------------------------------


class ScanResult;
class RealUI;

class AgDevice
{
public:
	void Selected(void);
	void DetachConnectionClicked(void);
	void HoldClicked(void);
	void HangupClicked(void);
	void AcceptCallClicked(void);
	void RejectCallClicked(void);
	void ReconnectDeviceClicked(void);

public:
	HfpSession		*m_sess;
	bool			m_known_device;

	/* State kept in UI */
	RealUI			*m_ui;
	string			m_active_cli;
	string			m_setup_cli;

	enum CallControlMode {
		CCM_DEAD,
		CCM_DISCONN,
		CCM_IDLE,
		CCM_RINGING,
		CCM_CONNECTED,
		CCM_CALLWAITING
	}		m_callmode;

	AgDevice(HfpSession *sessp)
		: m_sess(sessp)
        , m_known_device(false)
        , m_callmode(CCM_DEAD)
    {
		m_sess->SetPrivate(this);
		/*
		 * We operate without an extra reference on the session
		 * and depend on the destruction callback to delete
		 * ourselves.
		 */
	}

	virtual ~AgDevice() {}

public:
	bool IsKnown(void) const
    {
        return m_known_device;
    }
	void SetKnown(bool known)
    {
		if (known && !m_known_device) {
			m_sess->Get();
		} else if (!known && m_known_device) {
			m_sess->Put();
		}
		m_known_device = known;
	}
};

class RealUI
{
public:

	BtHub			*m_hub;
	HfpService		*m_hfpsvc;

	AgDevice		*m_all_devices;

	bool			m_known_devices_only;
	bool			m_autoreconnect;

	AgDevice		*m_active_dev;

	AgDevice *GetAgp(BtDevice *devp) {
		AgDevice *agp = 0;
		HfpSession *sessp = m_hfpsvc->GetSession(devp);
		if (sessp) {
			agp = (AgDevice*)sessp->GetPrivate();
			sessp->Put();
			if (agp) {
				assert(agp->m_sess == sessp);
				return agp;
			}
		}
		return 0;
	}

	void CreateDeviceBox(AgDevice *devp)
    {
		devp->m_callmode = AgDevice::CCM_DEAD;
	}

	void NotifyConnection(AgDevice *agp, HfpSession *sessp, ErrorInfo *reason)
    {
		assert(sessp == agp->m_sess);

		if (!sessp->IsConnecting() && !sessp->IsConnected()) {
			/*
			 * Device is disconnected and must have been
			 * connecting or connected before
			 */
			printf("%s device disconnect!\n",
			       sessp->IsPriorDisconnectVoluntary()
			       ? "Voluntary" : "Involuntary");
			if (sessp->IsPriorDisconnectVoluntary()) {
				VoluntaryDisconnect(agp);
			}
		}

		else if (sessp->IsConnecting()) {
			/* Connection is being established */
			AuditInboundConnection(agp);
		}

		else if (sessp->IsConnected()) {
			/* Connection is established */
			printf("Device connected!\n"
			       "ThreeWayCalling: %s\n"
			       "ECNR: %s\n"
			       "Voice Recognition: %s\n"
			       "InBandRingTone: %s\n"
			       "Voice Tags: %s\n"
			       "Reject Call: %s\n",
			       sessp->FeatureThreeWayCalling() ? "yes" : "no",
			       sessp->FeatureEcnr() ? "yes" : "no",
			       sessp->FeatureVoiceRecog() ? "yes" : "no",
			       sessp->FeatureInBandRingTone() ? "yes" : "no",
			       sessp->FeatureVoiceTag() ? "yes" : "no",
			       sessp->FeatureRejectCall() ? "yes" : "no");
		}

		/* Kick off a name resolution task */
		if ((sessp->IsConnecting() || sessp->IsConnected()) &&
		    !sessp->GetDevice()->IsNameResolved()) {
			(void) sessp->GetDevice()->ResolveName();
		}

		UpdateButtons(agp);
	}

	void NotifyAudioConnection(AgDevice *agp, HfpSession *sessp) {
		assert(agp->m_sess == sessp);
		if (!sessp->IsConnectingAudio() &&
		    !sessp->IsConnectedAudio())
			AudioDetached(agp);
		else if (sessp->IsConnectedAudio())
			AudioAttached(agp);
	}

	void NotifyCall(AgDevice *agp, HfpSession *sessp,
			bool ac_changed, bool wc_changed, bool ring) {
		if (ac_changed) {
			printf("In Call: %s\n",
			       sessp->HasEstablishedCall() ? "yes" : "no");
			if (sessp->HasEstablishedCall()) {
				agp->m_active_cli = agp->m_setup_cli;
				if (!sessp->IsConnectedAudio()) {
					(void) sessp->SndOpen(true, true);
				}
			}
		}

		if (wc_changed) {
			printf("Call Setup: %sOutgoing %sIncoming\n",
			       sessp->HasConnectingCall() ? "" : "!",
			       sessp->HasWaitingCall() ? "" : "!");
			if (sessp->HasWaitingCall()) {
				agp->m_setup_cli =
					sessp->WaitingCallIdentity()
					? string(sessp->
						  WaitingCallIdentity()->
						  number)
					: string("");
				printf("WCLI: %s\n",agp->m_setup_cli.c_str());
			}
		}

		if (ring) {
			const char *clip = (sessp->WaitingCallIdentity() ?
				    sessp->WaitingCallIdentity()->number : 0);
			if (!clip)
				clip = "Private Number";
			printf("Ring!! %s\n", clip);
			agp->m_setup_cli = clip;
			if (!sessp->HasEstablishedCall())
				DoRing();
		}

		UpdateButtons(agp);
	}

	void NotifyIndicator(AgDevice *agp, HfpSession *sessp,
			     const char *indname, int value) {
		assert(agp->m_sess == sessp);
		printf("Indicator: \"%s\" = %d\n", indname, value);
		UpdateButtons(agp);
	}


	/*
	 * We trap the HFP session instantiation path and
	 * use it to create our AgDevice shadow objects.
	 */
	HfpSession *SessionFactory(BtDevice *devp) {
		HfpSession *sessp;
		AgDevice *agp;

		sessp = m_hfpsvc->DefaultSessionFactory(devp);
		if (!sessp)
			return 0;

		agp = new AgDevice(sessp);
		if (!agp) {
			sessp->Put();
			return 0;
		}

		agp->m_ui = this;
		CreateDeviceBox(agp);

		/* Register notification callbacks */
		sessp->cb_NotifyConnection.
			Bind(this, &RealUI::NotifyConnection,
			     agp, Arg1, Arg2);
		sessp->cb_NotifyAudioConnection.
			Bind(this, &RealUI::NotifyAudioConnection,
			     agp, Arg1);
		sessp->cb_NotifyCall.
			Bind(this, &RealUI::NotifyCall,
			     agp, Arg1, Arg2, Arg3, Arg4);
		sessp->cb_NotifyIndicator.
			Bind(this, &RealUI::NotifyIndicator,
			     agp, Arg1, Arg2, Arg3);

		return sessp;
	}

	void NotifyNameResolved(BtDevice *devp, const char *name,
				ErrorInfo *reason) {
		AgDevice *agp;

		if (devp->GetPrivate()) {
		}

		agp = GetAgp(devp);
		if (agp)
			UpdateButtons(agp);
	}

	BtDevice *DeviceFactory(bdaddr_t const &addr) {
		BtDevice *devp;
		devp = m_hub->DefaultDevFactory(addr);
		if (!devp)
			return 0;
		devp->cb_NotifyNameResolved.
			Register(this, &RealUI::NotifyNameResolved);
		return devp;
	}

	SoundIo *OpenCallRecord(AgDevice *agp) {
		SoundIo *siop;
		/* Open a SoundIo for a call record WAV file? */
		siop = SoundIoCreateFileHandler(&g_qt_ei, "wiretap.wav", true);
		return siop;
	}

	void MaybeAttachAudio(void) {
		AgDevice *agp = m_active_dev;
		HfpSession *sessp;
		assert(agp);
		sessp = agp->m_sess;
		assert(sessp);

		if (!sessp->IsConnectingAudio() &&
		    !sessp->IsConnectedAudio() &&
		    (sessp->HasEstablishedCall() ||
		     sessp->HasConnectingCall())) {
			/* Try to raise the audio connection */
			sessp->SndOpen(true, true);
		}

		else if (sessp->IsConnectedAudio()) {
			if (m_sound_user == SC_RINGTONE) {
				/*
				 * Stop playing a ring tone and
				 * answer the call!
				 */
				SoundCardRelease();
			}

			if ((m_sound_user == SC_NONE) && !SoundCardCall()) {
				printf("Could not start audio pump\n");
				SoundCardRelease();
				sessp->SndClose();
			}
		}
	}

	void SetDeviceFocus(AgDevice *agp) {
		if (agp == m_active_dev) { return; }
		if (agp && !agp->m_sess->IsConnected()) { return; }
		if (m_active_dev) {

			/* Stop streaming audio from the device losing focus */
			if (m_sound_user == SC_CALL) {
				SoundCardRelease();
				m_active_dev->m_sess->SndClose();
			}
		}
		m_active_dev = agp;
		if (agp != NULL) {
			MaybeAttachAudio();
		}
	}

	void SetDeviceBox(AgDevice *agp, AgDevice::CallControlMode ccm,
			  string &status_cap, string &ac_cap,
			  string &sc_cap) {

		if (ccm == agp->m_callmode) {
			goto skip_buttons;
		}

		switch (ccm) {
		case AgDevice::CCM_DEAD:
			break;
		case AgDevice::CCM_DISCONN:
			break;
		case AgDevice::CCM_IDLE:
			break;
		case AgDevice::CCM_RINGING:
			break;
		case AgDevice::CCM_CONNECTED:
			break;
		case AgDevice::CCM_CALLWAITING:
			break;
		default:
			abort();
		}

	skip_buttons:
		if (ccm != AgDevice::CCM_DEAD) {
		}

		agp->m_callmode = ccm;
	}

	void AuditInboundConnection(AgDevice *targag) {
		if (targag->m_sess->IsConnectionRemoteInitiated() &&
		    m_known_devices_only &&
		    !targag->IsKnown()) {
			/* Rejected! */
			targag->m_sess->Disconnect();
			return;
		}

		if (targag->m_sess->IsConnected() && !m_known_devices_only) {
			targag->SetKnown(true);
			targag->m_sess->SetAutoReconnect(m_autoreconnect);
			SaveConfiguration();
		}
	}

	void VoluntaryDisconnect(AgDevice *targag) {
        //ag+++
        AgDevice *devAg = targag;
        if (devAg == NULL) {
            devAg = m_active_dev;
        }
        //
		if (devAg->IsKnown()) {
			devAg->SetKnown(false);
			SaveConfiguration();
		}
		devAg->m_sess->SetAutoReconnect(false);
	}

	void UpdateButtons(AgDevice *targag) {
		AgDevice::CallControlMode ccm = AgDevice::CCM_DEAD;

		if (targag) {
			HfpSession *sessp = targag->m_sess;
			string status, ac, sc;

			status = sessp->GetDevice()->GetName();
			if (!sessp->IsConnected() &&
			    !sessp->IsConnecting()) {

				if (sessp->IsAutoReconnect() ||
				    targag->IsKnown()) {
					status += "\nDetached";
					ccm = AgDevice::CCM_DISCONN;
				} else {
					ccm = AgDevice::CCM_DEAD;
				}
			}
			else if (sessp->IsConnecting()) {
				ccm = AgDevice::CCM_IDLE;
				status += "\nAttaching";
			}
			else if (!sessp->GetService()) {
				ccm = AgDevice::CCM_IDLE;
				status += "\nNo Service";
			}
			else {
				if (sessp->IsConnectedAudio()) {
					status += "\nAudio Open";
				} else {
					status += "\nReady";
				}

				if (!sessp->HasEstablishedCall() &&
				    !sessp->HasConnectingCall() &&
				    !sessp->HasWaitingCall()) {
					ccm = AgDevice::CCM_IDLE;
				}
				else if (!sessp->HasEstablishedCall() &&
					 !sessp->HasConnectingCall()) {
					ccm = AgDevice::CCM_RINGING;
					sc = "Incoming Call: " +
						targag->m_setup_cli;
				}
				else if (!sessp->HasWaitingCall() &&
					 !sessp->HasConnectingCall()) {
					ccm = AgDevice::CCM_CONNECTED;
					ac = "Connected: " +
						targag->m_active_cli;
				}
				else if (!sessp->HasEstablishedCall()) {
					ccm = AgDevice::CCM_CONNECTED;
					ac = "Dialing: " +
						targag->m_setup_cli;
				}
				else {
					ccm = AgDevice::CCM_CALLWAITING;
					ac = "Connected: " +
						targag->m_active_cli;
					sc = "Incoming Call: " +
						targag->m_setup_cli;
				}
			}
			SetDeviceBox(targag, ccm, status, ac, sc);
		}

		if ((m_active_dev == NULL) ||
		    !m_active_dev->m_sess->IsConnected()) {
			/* Maybe choose a new device? */
			if (targag && targag->m_sess->IsConnected()) {
				SetDeviceFocus(targag);
			} else {
				BtDevice *devp;
				AgDevice *agp = 0;
				for (devp = m_hub->GetFirstDevice();
				     (devp != NULL);
				     devp = m_hub->GetNextDevice(devp)) {
					agp = GetAgp(devp);
					if (agp->m_sess->IsConnected())
						break;
				}

				if (agp)
					SetDeviceFocus(agp);
			}
		}

		if (m_active_dev == NULL) {
			if (m_hub->IsStarted())
				printf("No Audio Gateways Available");
			else
				printf("Bluetooth Unavailable");
			return;
		}

		if ((targag != NULL) &&
		    (targag != m_active_dev)) {
			/* Uninteresting */
			return;
		}

		if (!m_active_dev->m_sess->IsConnected() ||
		    !m_active_dev->m_sess->GetService()) {
			/* Gray everything! */
		} else {
		}
	}

	/*
	 * Sound transfer + signal processing glue
	 */

	string			m_sound_driver;
	string			m_sound_driveropt;

	string			m_ringtone_filename;
	SoundIo			*m_ringtone_src;

	SoundIo			*m_membuf;

	SoundIoManager		*m_sound;
	int			m_packet_size_ms;
	int			m_outbuf_size_ms;

	SoundIoFltSpeex		*m_sigproc;
	SoundIoSpeexProps	m_sigproc_props;

	enum {
		SC_SHUTDOWN,
		SC_NONE,
		SC_CALL,
		SC_FEEDBACK,
		SC_MEMBUF,
		SC_RINGTONE
	}			m_sound_user;

	bool SoundCardInit(void) {
		assert(!m_sound);
		assert(!m_sound);
		assert(m_sound_user == SC_SHUTDOWN);

		m_sound = new SoundIoManager(&g_qt_ei);
		if (!m_sound)
			return false;

		if (!m_sound->SetDriver(m_sound_driver.c_str(),
					m_sound_driveropt.c_str()))
			return false;

		m_sound->cb_NotifyAsyncState.Register(this,
					     &RealUI::NotifyPumpState);

		m_sound->SetPacketIntervalHint(m_packet_size_ms);
		m_sound->SetMinBufferFillHint(m_outbuf_size_ms);
		/* hard-code for now */
		m_sound->SetJitterWindowHint(10);

		m_sigproc = SoundIoFltCreateSpeex(&g_qt_ei);
		if (m_sigproc) {
			m_sigproc->Configure(m_sigproc_props);
			m_sound->SetDsp(m_sigproc);
		}

		m_sound_user = SC_NONE;
		return true;
	}

	void SoundCardShutdown(void) {
		assert(m_sound);
		assert(m_sound_user == SC_NONE);
		assert(!m_sound->IsStarted());
		delete m_sound;
		m_sound = 0;
		m_sound_user = SC_SHUTDOWN;
	}

	/*
	 * Previously the state of the sound card was poorly controlled.
	 * This routine is pedantic to make up for it.
	 */
	void SoundCardRelease(void) {
		switch (m_sound_user) {
		case SC_NONE:
			assert(!m_sound->IsStarted());
			break;
		case SC_CALL:
			m_sound->Stop();
			assert(m_sound->GetSecondary() ==
			       m_active_dev->m_sess);
			m_sound->SetSecondary(0);
			break;
		case SC_FEEDBACK:
			m_sound->Stop();
			assert(m_sound->GetSecondary() == 0);
			m_sound->SetSecondary(0);
			break;
		case SC_MEMBUF:
			assert(m_membuf);
			m_sound->Stop();
			assert(m_sound->GetSecondary() == m_membuf);
			m_sound->SetSecondary(0);
			m_membuf->SndClose();
			break;
		case SC_RINGTONE:
			assert(m_ringtone_src != 0);
			m_sound->Stop();
			assert(m_sound->GetSecondary() == m_ringtone_src);
			m_sound->SetSecondary(0);
			m_ringtone_src->SndClose();
			break;
		default:
			/* Unexpected state */
			abort();
		}
		m_sound_user = SC_NONE;
	}

	bool SoundCardCall(void) {
		assert(m_active_dev);
		assert(m_sound_user == SC_NONE);
		assert(!m_sound->IsStarted());
		assert(m_sound->GetSecondary() == 0);
		m_sound->SetSecondary(m_active_dev->m_sess);
		if (!m_sound->Start()) {
			m_sound->SetSecondary(0);
			return false;
		}
		m_sound_user = SC_CALL;
		return true;
	}

	bool SoundCardLoop(void) {
		assert(m_sound_user == SC_NONE);
		assert(!m_sound->IsStarted());
		assert(m_sound->GetSecondary() == 0);
		m_sound->Loopback();
		if (!m_sound->Start()) {
			m_sound->SetSecondary(0);
			return false;
		}
		m_sound_user = SC_FEEDBACK;
		return true;
	}

	bool SoundCardMembuf(bool in, bool out) {
		assert(m_membuf);
		assert(m_sound_user == SC_NONE);
		assert(!m_sound->IsStarted());
		assert(m_sound->GetSecondary() == 0);
		m_sound->SetSecondary(m_membuf);
		if (!m_membuf->SndOpen(in, out))
			abort();
		if (!m_sound->Start()) {
			m_sound->SetSecondary(0);
			m_membuf->SndClose();
			return false;
		}
		m_sound_user = SC_MEMBUF;
		return true;
	}

	bool SoundCardRingTone(void) {
		if (m_ringtone_src == 0)
			return false;

		assert(m_sound_user == SC_NONE);
		assert(!m_sound->IsStarted());
		assert(m_sound->GetSecondary() == 0);

		if (!m_ringtone_src->SndOpen(false, true))
			return false;

		m_sound->SetSecondary(m_ringtone_src);
		if (!m_sound->Start()) {
			m_sound->SetSecondary(0);
			m_ringtone_src->SndClose();
			return false;
		}

		m_sound_user = SC_RINGTONE;
		return true;
	}

	void OpenRingTone(void) {
		if (m_sound_user == SC_RINGTONE)
			/* Stop an existing ring tone playback */
			SoundCardRelease();

		if (m_ringtone_src) {
			delete m_ringtone_src;
			m_ringtone_src = 0;
		}

		m_ringtone_src = SoundIoCreateFileHandler(&g_qt_ei,
			m_ringtone_filename.c_str(), false);

		if (!m_ringtone_src) {
			printf("Could not create ring tone handler\n");
		}
	}

	void AudioAttached(AgDevice *agp) {
		/*
		 * If the focused device doesn't have an active
		 * audio connection, shift focus to this device.
		 */

		if ((m_active_dev != agp) &&
		    ((m_active_dev == NULL) ||
		     !m_active_dev->m_sess->IsConnectedAudio())) {
			SetDeviceFocus(agp);
		}

		/*
		 * If this was already the focused device, attempt
		 * to open the sound card and start the audio pump.
		 */

		else if (m_active_dev == agp) {
			MaybeAttachAudio();
		}

		printf("Audio attached!\n");
		UpdateButtons(agp);
	}

	void AudioDetached(AgDevice *agp) {
		if ((m_sound_user == SC_CALL) && (agp == m_active_dev)) {
			SoundCardRelease();
		}
		printf("Audio detached!\n");
		UpdateButtons(agp);
	}


public:
	void ToggleFeedbackTest(bool enable) {

		if (m_sound_user == SC_SHUTDOWN)
			return;

		if (enable && (m_sound_user == SC_NONE)) {
			string label;

			if (!SoundCardLoop()) {
				fprintf(stderr, "Could not start "
					"feedback loop\n");
				SoundCardRelease();
				return;
			}

            char buff[100];
			sprintf(buff, "%dms", m_sound->GetPacketInterval());
			return;
		}

		if (enable) {
			/* Sound card is busy */
			return;
		}

		if (m_sound_user == SC_FEEDBACK) {
			SoundCardRelease();
			if (m_active_dev)
				MaybeAttachAudio();
		}
	}

	void ToggleProcTest(bool enable) {

		if (m_sound_user == SC_SHUTDOWN)
			return;

		if (enable && (m_sound_user == SC_NONE)) {

            bool in = false;
            bool out = false;
			if (!SoundCardMembuf(in, out)) {
				fprintf(stderr, "Could not start "
					"memory buffer mode\n");
				return;
			}
		}

		if (enable) {
			/* Sound card is busy */
			return;
		}

		if (m_sound_user == SC_MEMBUF) {
			SoundCardRelease();
			if (m_active_dev)
				MaybeAttachAudio();
		}
	}

	void SoundCardReconfig(void) {
		int oldstate = m_sound_user;

		if (m_sound_user != SC_NONE)
			SoundCardRelease();

		if (m_sigproc &&
		    !m_sigproc->Configure(m_sigproc_props)) {
			fprintf(stderr, "%s: sigproc failed\n", __FUNCTION__);
			return;
		}
		m_sound->SetPacketIntervalHint(m_packet_size_ms);
		m_sound->SetMinBufferFillHint(m_outbuf_size_ms);

		switch (oldstate) {
		case SC_CALL:
			MaybeAttachAudio();
			break;
		case SC_FEEDBACK:
			if (SoundCardLoop()) {
				string label;
                char buff[100];
				sprintf(buff, "%dms", m_sound->GetPacketInterval());
                label = buff;
			}
			break;
		case SC_RINGTONE:
			SoundCardRingTone();
			break;
		}
	}

	void UpdatePacketSize(int packet_ms) {
		m_packet_size_ms = packet_ms;
		SoundCardReconfig();
	}

	void UpdateOutbufSize(int outbuf_ms) {
		m_outbuf_size_ms = outbuf_ms;
		SoundCardReconfig();
	}

	void UpdateNoiseReduction(bool enable) {
		m_sigproc_props.noisereduce = enable;
		SoundCardReconfig();
	}

	void UpdateEchoCancel(int echocancel_ms) {
		m_sigproc_props.echocancel_ms = echocancel_ms;
		SoundCardReconfig();
	}

	void UpdateAutoGain(int agc_level) {
		m_sigproc_props.agc_level = agc_level * 1000;
		SoundCardReconfig();
	}

	void UpdateDereverb(float drlvl, float drdecay) {
		m_sigproc_props.dereverb_level = drlvl / 100;
		m_sigproc_props.dereverb_decay = drdecay / 100;
		SoundCardReconfig();
	}

public:

	void NotifyInquiryResult(BtDevice *devp, ErrorInfo *error) {

		if (!devp) {
			if (error) {
                printf("NoHands. Inquiry Failed to Start: %s", error->Desc());
				return;
			}

			/* Scan is complete */
			printf("HCI scan complete\n");
			return;
		}

		devp->ResolveName();
		printf("Scan: %s\n", devp->GetName());
	}

	void NotifyBTState(ErrorInfo *reason) {
		if (!m_hub->IsStarted()) {
			printf("Bluetooth Unavailable");
			printf("Bluetooth Failure, hub shut down\n");
		} else {
			uint32_t devclass;
			if (m_hub->GetHci() &&
			    m_hub->GetHci()->GetDeviceClassLocal(devclass) &&
			    !m_hfpsvc->IsDeviceClassHf(devclass)) {
				m_hfpsvc->SetDeviceClassHf(devclass);
				printf("*** class 0x%06x;\n", devclass);
			}
		}

		UpdateButtons(0);
	}

	void NotifyPumpState(SoundIoManager *soundp, ErrorInfo &error) {
		int old_user = m_sound_user;
		fprintf(stderr, "Audio pump aborted\n");
		SoundCardRelease();
		switch (old_user) {
		case SC_CALL:
			m_active_dev->m_sess->SndClose();
			UpdateButtons(m_active_dev);
			break;
		case SC_FEEDBACK:
			break;
		case SC_MEMBUF:
			break;
		}
	}

	void DoRing(void) {
		/* Only start a ring tone if the device is unoccupied */
		if (m_sound_user == SC_NONE)
			SoundCardRingTone();
	}

	bool DelPend(HfpPendingCommand *pendp) {
		if (!pendp)
			return false;

		delete pendp;
		return true;
	}

	/*
	 * Bluetooth Command Handling for the user interface
	 */
	virtual void KeypadPress(char c) {
		if ((m_active_dev != NULL) &&
		    m_active_dev->m_sess->HasEstablishedCall()) {
			DelPend(m_active_dev->m_sess->CmdSendDtmf(c));
		}
	}
	virtual void KeypadClick(char c) {
		if ((m_active_dev == NULL) ||
		    (!m_active_dev->m_sess->HasEstablishedCall() &&
		     !m_active_dev->m_sess->HasConnectingCall())) {
		}
	}
	virtual void Dial(const string &phoneNum) {
		if (!phoneNum.empty() &&
		    (m_active_dev != NULL) &&
		    m_active_dev->m_sess->GetService()) {
			printf("Dial: %s\n", phoneNum.c_str());
			DelPend(m_active_dev->m_sess->
				CmdDial(phoneNum.c_str()));
		}
	}
	virtual void RedialClicked(void) {
		if ((m_active_dev != NULL) &&
		    m_active_dev->m_sess->GetService()) {
			DelPend(m_active_dev->m_sess->CmdRedial());
		}
	}
	virtual void HandsetClicked(void) {
		if (m_sound_user == SC_CALL) {
			SoundCardRelease();
			m_active_dev->m_sess->SndClose();
			UpdateButtons(m_active_dev);
		} else
			MaybeAttachAudio();
	}
	virtual void HoldClicked(AgDevice *devp) {
		DelPend(devp->m_sess->CmdCallSwapHoldActive());
	}
	virtual void HangupClicked(AgDevice *devp) {
        //ag+++
        AgDevice *devAg = devp;
        if (devAg == NULL) {
            devAg = m_active_dev;
        }
        //
        DelPend(devAg->m_sess->CmdHangUp());
	}
	virtual void AcceptCallClicked(AgDevice *devp) {
        //ag+++
        AgDevice *devAg = devp;
        if (devAg == NULL) {
            devAg = m_active_dev;
        }
        //
		if (devAg->m_sess->HasEstablishedCall()) {
			DelPend(devAg->m_sess->CmdCallSwapDropActive());
		} else {
			DelPend(devAg->m_sess->CmdAnswer());
		}
	}
	virtual void RejectCallClicked(AgDevice *devp) {
        //ag+++
        AgDevice *devAg = devp;
        if (devAg == NULL) {
            devAg = m_active_dev;
        }
        //
		DelPend(devAg->m_sess->CmdCallDropHeldUdub());
	}

    virtual void QueryPhoneNumber(AgDevice *devp) {
        AgDevice *devAg = devp;
        if (devAg == NULL) {
            devAg = m_active_dev;
        }
        DelPend(devAg->m_sess->CmdQueryNumber());
    }


	/*
	 * Configuration Data
	 */

	void LoadConfiguration(void) {
		OpenRingTone();
	}

	void SaveConfiguration(void) {
	}

	void ReconnectNow(void) {
		BtDevice *devp;

		for (devp = m_hub->GetFirstDevice();
		     devp != NULL;
		     devp = m_hub->GetNextDevice(devp)) {
			AgDevice *agp = 0;
			agp = GetAgp(devp);
			if (agp->IsKnown()) {
				agp->ReconnectDeviceClicked();
			}
		}
	}

	bool Init(void) {
		LoadConfiguration();

		if (!SoundCardInit())
			return false;

		if (!m_sound->TestOpen()) {
			printf("Audio Device Could Not Be Opened");
		}

		m_hub->SetAutoRestart(true);
		m_hub->Start();
		NotifyBTState(0);

		if (m_autoreconnect) {
			ReconnectNow();
		}
		return true;
	}

	AgDevice *Connect(const char *rname) {
		HfpSession *sessp;
		AgDevice *agp = 0;
		sessp = m_hfpsvc->Connect(rname);
		if (sessp != NULL) {
			agp = (AgDevice *) sessp->GetPrivate();
			assert(agp && (agp->m_sess == sessp));

			if (!agp->IsKnown()) {
				agp->SetKnown(true);
				SaveConfiguration();
			}
			sessp->SetAutoReconnect(m_autoreconnect);
			UpdateButtons(agp);
		}
		return agp;
	}

	RealUI(const char *name = 0, bool modal = false)
		: m_known_devices_only(true)
        , m_autoreconnect(false)
        , m_active_dev(NULL)
        , m_ringtone_src(0)
        , m_membuf(0)
        , m_sound(0)
        , m_sound_user(SC_SHUTDOWN)
    {
		m_hub = new BtHub(&g_qt_ei);

		m_hub->cb_InquiryResult.Register(this,
						 &RealUI::NotifyInquiryResult);
		m_hub->cb_NotifySystemState.Register(this,
						&RealUI::NotifyBTState);
		m_hub->cb_BtDeviceFactory.Register(this,
						&RealUI::DeviceFactory);

		m_hfpsvc = new HfpService();
		m_hfpsvc->cb_HfpSessionFactory.Register(this,
						&RealUI::SessionFactory);
		m_hub->AddService(m_hfpsvc);

		UpdateButtons(NULL);
	}

	virtual ~RealUI() {
		SoundCardRelease();
		SoundCardShutdown();
		delete m_hub;
	}
};


/*
 * I'm not clever enough to slot these user interface callbacks to
 * RealUI, so they are slotted to AgDevice and we call RealUI from here.
 */

void AgDevice::
Selected(void)
{
	m_ui->SetDeviceFocus(this);
	m_ui->UpdateButtons(NULL);
}

void AgDevice::
DetachConnectionClicked(void)
{
	if (m_sess->IsConnected() || m_sess->IsConnecting()) {
		m_sess->Disconnect();
		/*
		 * Because we called Disconnect directly, it will be
		 * recorded as a voluntary disconnection, and
		 * NotifyDeviceConnection() will behave as such.
		 */
		m_ui->NotifyAudioConnection(this, m_sess);
		m_ui->NotifyConnection(this, m_sess, 0);
	} else {
		m_ui->VoluntaryDisconnect(this);
		m_ui->UpdateButtons(this);
	}
}

void AgDevice::
HoldClicked(void)
{
	m_ui->HoldClicked(this);
}

void AgDevice::
HangupClicked(void)
{
	m_ui->HangupClicked(this);
}

void AgDevice::
AcceptCallClicked(void)
{
	m_ui->AcceptCallClicked(this);
}

void AgDevice::
RejectCallClicked(void)
{
	m_ui->RejectCallClicked(this);
}

void AgDevice::
ReconnectDeviceClicked(void)
{
	if (!m_sess->IsConnecting() && !m_sess->IsConnected()) {
		m_sess->Connect();
		m_ui->UpdateButtons(this);
	}
}

void printHelp()
{
    printf("> Input num:\n");
    printf(">   1 Dial Number\n");
    printf(">   2 Hang up\n");
    printf(">   3 Redial\n");
    printf(">   4 AcceptCallClicked\n");
    printf(">   5 RejectCallClicked\n");
    printf(">   6 QueryPhoneNumber\n");
    printf(">   11 KeypadPress\n");
    printf("> 100 quit\n");
}


int main(int argc, char **argv)
{
	RealUI *rui;
	rui = new RealUI();

	if (!rui->Init()) {
		fprintf(stderr, "Init failed!\n");
		return 1;
	}


	char dest_addr[] = "CC:08:8D:AC:3A:6E";
	if (argc >= 2) {
		strcpy(dest_addr, argv[1]);
	}
	else {
	}

	if (!rui->Connect(dest_addr)) {
		fprintf(stderr, "Connection setup failed!\n");
		return 1;
	}


    pthread_t fd_notify_thread;
    if(pthread_create(&fd_notify_thread, NULL, fd_process, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    char ch[10] = { 0 };
    while(1) {
        scanf("%s", ch);
        int cmd = atoi(ch);

        char dat[100] = { 0 };
        switch (cmd) {
            case 0:
            {
                printHelp();
            }
            break;
            case 1:
            {
                printf("> Input Dial:");
                scanf("%s", dat);
                std::string phoneNum(dat);
                rui->Dial(phoneNum);
            }
            break;
            case 11:
            {
                printf("> 11\n");
                char cch[20] = {0};
                while(scanf("%s", cch)!=EOF && cch[0] != 'q') {
                    printf("#KeypadPress:%c\n", cch[0]);
                    rui->KeypadPress(cch[0]);
                }
                 printf("> 11-quit\n");
            }
            break;
            case 2:
            {
                printf("#hang up\n");
                rui->HangupClicked(NULL);
            }
            break;
            case 3:
            {
                printf("#ReDial\n");
                rui->RedialClicked();
            }
            break;
            case 4:
            {
                printf("#AcceptCallClicked\n");
                rui->AcceptCallClicked(NULL);
            }
            break;
            case 5:
            {
                printf("#RejectCallClicked\n");
                rui->RejectCallClicked(NULL);
            }
            break;
            case 6:
            {
                printf("#QueryPhoneNumber\n");
                rui->QueryPhoneNumber(NULL);
            }
            break;
            default:
                break;
        }
        if (cmd == 100) {
            printf("quit\n");
            break;
        }

        usleep(100*1000);
    }

    pthread_cancel(fd_notify_thread);

    std::map<int, MySocketNotifier*>::iterator it;
    for (it = g_socketNofity_ReadMap.begin(); it != g_socketNofity_ReadMap.end(); ++it) {
        int fd = it->first;
        close(fd);
    }
    for (it = g_socketNofity_WriteMap.begin(); it != g_socketNofity_WriteMap.end(); ++it) {
        int fd = it->first;
        close(fd);
    }

    delete rui;
    rui = NULL;

	return 0;
}
