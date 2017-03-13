/*
 * Software Bluetooth Hands-Free Implementation
 *
 * Copyright (C) 2006-2008 Sam Revitch <samr7@cs.washington.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * This package is specific to Linux and the Qualcomm BlueZ Bluetooth
 * stack for Linux.  It is not specific to any GUI or application
 * framework, and can be adapted to most by creating an appropriate
 * DispatchInterface class.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/sco.h>

#include <libhfp/hfp.h>

namespace libhfp {


HfpService::
HfpService(int caps)
	: RfcommService(HANDSFREE_AGW_SVCLASS_ID),
	  m_sco_listen(-1), m_sco_listen_not(0),
	  m_brsf_my_caps(caps), m_svc_name(0), m_svc_desc(0),
	  m_sdp_rec(0), m_sco_enable(true),
	  m_complaint_sco_mtu(false), m_complaint_sco_vs(false),
	  m_complaint_sco_listen(false)
{
}

HfpService::
~HfpService()
{
	if (m_svc_name) {
		free(m_svc_name);
		m_svc_name = 0;
	}
	if (m_svc_desc) {
		free(m_svc_desc);
		m_svc_desc = 0;
	}
}

bool HfpService::
ScoListen(ErrorInfo *error)
{
	struct sockaddr_sco saddr;
	int sock = -1, res;
	uint16_t mtu, pkts, vs, nvs;
	BtHci *hcip;

	assert(m_sco_listen == -1);

	hcip = GetHub()->GetHci();
	if (!hcip)
		return false;

	/*
	 * I'm not sure what the whole story is with this, but some
	 * Broadcom dongles cause the kernel to set its SCO MTU
	 * values to 16:0.  This is unsuitable for us, we need bigger
	 * packets and more buffering, and we will refuse to listen
	 * if it is not available.
	 */
	if (!hcip->GetScoMtu(mtu, pkts, error))
		return false;

	if ((mtu < 48) || (pkts < 8)) {
		if (!hcip->SetScoMtu(64, 8)) {
			if (error)
				error->Set(LIBHFP_ERROR_SUBSYS_BT,
					   LIBHFP_ERROR_BT_BAD_SCO_CONFIG,
					   "Unsuitable SCO MTU values %u:%u "
					   "detected\n"
					   "To fix this, run, as superuser, "
					   "\"hciconfig hci0 scomtu 64:8\"",
					   mtu, pkts);
			if (!m_complaint_sco_mtu) {
				GetDi()->LogError(
					"Unsuitable SCO MTU values %u:%u "
					"detected\n"
					"To fix this, run, as superuser, "
					"\"hciconfig hci0 scomtu 64:8\"",
					mtu, pkts);
				m_complaint_sco_mtu = true;
			}
			return false;
		}

		mtu = 64;
		pkts = 8;
	}

	/*
	 * Verify that the voice setting is suitable for our ends.
	 * We expect 2's complement, 16-bit linear coding.
	 */
	if (!hcip->GetScoVoiceSetting(vs, error))
		return false;

	if (vs != 0x0060) {
		nvs = 0x0060;
		if (!hcip->SetScoVoiceSetting(nvs)) {
			if (error)
				error->Set(LIBHFP_ERROR_SUBSYS_BT,
					   LIBHFP_ERROR_BT_BAD_SCO_CONFIG,
					   "Unsuitable SCO voice setting "
					   "0x%04x detected\n"
					   "To fix this, run, as superuser, "
					   "\"hciconfig hci0 voice 0x0060\"",
					   vs);
			if (!m_complaint_sco_vs) {
				GetDi()->LogError(
					"Unsuitable SCO voice setting "
					"0x%04x detected\n"
					"To fix this, run, as superuser, "
					"\"hciconfig hci0 voice 0x0060\"",
					vs);
				m_complaint_sco_vs = true;
			}
			return false;
		}

		vs = nvs;
	}

	/*
	 * Somebody may have also forgotten to enable SCO support
	 * in the kernel, or the sco.ko module got lost.
	 */
	sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (sock < 0) {
		sock = errno;
		if (sock == EPROTONOSUPPORT) {
			GetDi()->LogError(error,
					  LIBHFP_ERROR_SUBSYS_BT,
					  LIBHFP_ERROR_BT_NO_SUPPORT,
					  "Your kernel is not configured with "
					  "support for SCO sockets.");
			GetHub()->SetAutoRestart(false);
			return false;
		}
		GetDi()->LogWarn(error, LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Create SCO socket: %s",
				 strerror(errno));
		return false;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sco_family = AF_BLUETOOTH;
	bacpy(&saddr.sco_bdaddr, &(hcip->GetAddr()));

	if (bind(sock, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
		res = -errno;
		if (res == -EADDRINUSE) {
			if (error)
				error->Set(LIBHFP_ERROR_SUBSYS_BT,
					   LIBHFP_ERROR_BT_SERVICE_CONFLICT,
					   "Another service has claimed your "
					   "system's Bluetooth audio socket.\n"
					   "Please disable it before "
					   "attempting to use HFP for "
					   "Linux");

			if (!m_complaint_sco_listen) {
				GetDi()->LogError(
					"Another service has claimed your "
					"system's Bluetooth audio socket.\n"
					"Please disable it before "
					"attempting to use HFP for Linux");
				m_complaint_sco_listen = true;
			}
		} else {
			GetDi()->LogWarn(error,
					 LIBHFP_ERROR_SUBSYS_BT,
					 LIBHFP_ERROR_BT_SYSCALL,
					 "Bind SCO socket: %s",
					 strerror(-res));
		}
		goto failed;
	}

	if (listen(sock, 1) < 0) {
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Set SCO socket to listen: %s",
				 strerror(errno));
		goto failed;
	}

	if (!SetNonBlock(sock, true)) {
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Set SCO listener nonblocking: %s",
				 strerror(errno));
		goto failed;
	}

	m_sco_listen_not = GetDi()->NewSocket(sock, false);
	if (!m_sco_listen_not) {
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Could not create SCO listening "
				 "socket notifier");
		goto failed;
	}

	m_sco_listen_not->Register(this, &HfpService::ScoListenNotify);
	m_sco_listen = sock;

	GetDi()->LogDebug("SCO MTU: %u:%u Voice: 0x%04x", mtu, pkts, vs);
	return true;

failed:
	if (sock >= 0)
		close(sock);
	return false;
}

void HfpService::
ScoListenNotify(SocketNotifier *notp, int fh)
{
	BtDevice *devp;
	HfpSession *sessp = 0;
	struct sockaddr_sco saddr;
	socklen_t al;
	int ssock;
	bool res;

	assert(fh == m_sco_listen);
	assert(notp == m_sco_listen_not);

	al = sizeof(saddr);
	ssock = accept(fh, (struct sockaddr*)&saddr, &al);
	if (ssock < 0) {
		GetDi()->LogDebug("SCO accept: %s", strerror(errno));
		return;
	}

	/* Determine the owner of this SCO connection */
	devp = GetHub()->GetDevice(saddr.sco_bdaddr, false);
	if (devp) {
		sessp = FindSession(devp);
		devp->Put();
	}

	if (!sessp) {
		char bdstr[32];
		close(ssock);
		ba2str(&saddr.sco_bdaddr, bdstr);
		GetDi()->LogDebug("Got SCO connect request from %s with "
				  "no existing session", bdstr);
		return;
	}

	res = sessp->ScoAccept(ssock);
	sessp->Put();

	if (!res)
		close(ssock);
}

void HfpService::
ScoCleanup(void)
{
	if (m_sco_listen_not) {
		assert(m_sco_listen >= 0);
		delete m_sco_listen_not;
		m_sco_listen_not = 0;
	}
	if (m_sco_listen >= 0) {
		close(m_sco_listen);
		m_sco_listen = -1;

		m_complaint_sco_mtu = false;
		m_complaint_sco_vs = false;
		m_complaint_sco_listen = false;
	}
}

/*
 * This function, along with the majority of sdp_lib, is ridiculous.
 * It doesn't have to be this hard, guys!
 */
bool HfpService::
SdpRegister(ErrorInfo *error)
{
	uuid_t root_uuid, hfsc_uuid, gasc_uuid;
	sdp_list_t *root_list = 0;
	sdp_profile_desc_t hfp_pdesc;
	sdp_record_t *svcrec;
	uint16_t caps;

	if (!(svcrec = sdp_record_alloc()))
		goto nomem;

	/* No failure reporting path here! */
	sdp_set_info_attr(svcrec,
			  GetServiceName(),
			  NULL,
			  GetServiceDesc());

	sdp_uuid16_create(&hfsc_uuid, HANDSFREE_SVCLASS_ID);
	if (!(root_list = sdp_list_append(0, &hfsc_uuid)))
		goto nomem;
	sdp_uuid16_create(&gasc_uuid, GENERIC_AUDIO_SVCLASS_ID);
	if (!(root_list = sdp_list_append(root_list, &gasc_uuid)))
		goto nomem;
	if (sdp_set_service_classes(svcrec, root_list) < 0)
		goto nomem;

	sdp_list_free(root_list, 0);
	root_list = 0;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	if (!(root_list = sdp_list_append(NULL, &root_uuid)))
		goto nomem;
	if (sdp_set_browse_groups(svcrec, root_list) < 0)
		goto nomem;

	sdp_list_free(root_list, 0);
	root_list = 0;

	if (!RfcommSdpSetAccessProto(svcrec, RfcommGetListenChannel()))
		goto nomem;

	// Profiles
	sdp_uuid16_create(&hfp_pdesc.uuid, HANDSFREE_PROFILE_ID);
	hfp_pdesc.version = 0x0105;
	if (!(root_list = sdp_list_append(NULL, &hfp_pdesc)))
		goto nomem;
	if (sdp_set_profile_descs(svcrec, root_list) < 0)
		goto nomem;

	sdp_list_free(root_list, 0);
	root_list = 0;

	/* Add one last required attribute */
	caps = m_brsf_my_caps & 0x1f;
	if (sdp_attr_add_new(svcrec, SDP_ATTR_SUPPORTED_FEATURES,
			     SDP_UINT16, &caps) < 0)
		goto nomem;

	if (!GetHub()->SdpRecordRegister(svcrec, error))
		goto failed;

	m_sdp_rec = svcrec;
	return true;

nomem:
	if (error)
		error->SetNoMem();

failed:
	if (root_list)
		sdp_list_free(root_list, 0);
	if (svcrec)
		sdp_record_free(svcrec);
	return false;
}

void HfpService::
SdpUnregister(void)
{
	if (m_sdp_rec) {
		/* FIXME: Can this leak the SDP record? */
		GetHub()->SdpRecordUnregister(m_sdp_rec);
		m_sdp_rec = 0;
	}
}

bool HfpService::
Start(ErrorInfo *error)
{
	assert(GetHub());

	if (!RfcommService::Start(error))
		goto failed;

	if (!RfcommListen(error))
		goto failed;

	if (m_sco_enable && !ScoListen(error))
		goto failed;

	if (!SdpRegister(error))
		goto failed;

	return true;

failed:
	Stop();
	return false;
}

void HfpService::
Stop(void)
{
	assert(GetHub());
	if (GetHub()->IsStarted())
		SdpUnregister();
	RfcommCleanup();
	ScoCleanup();
	RfcommService::Stop();
}

RfcommSession * HfpService::
SessionFactory(BtDevice *devp)
{
	HfpSession *sessp;

	assert(GetHub());
	assert(!FindSession(devp));

	if (cb_HfpSessionFactory.Registered())
		sessp = cb_HfpSessionFactory(devp);
	else
		sessp = DefaultSessionFactory(devp);

	return sessp;
}

HfpSession *HfpService::
DefaultSessionFactory(BtDevice *devp)
{
	return new HfpSession(this, devp);
}

HfpSession *HfpService::
Connect(BtDevice *devp, ErrorInfo *error)
{
	HfpSession *sessp = GetSession(devp);
	if (!sessp) {
		if (error)
			error->SetNoMem();
		return 0;
	}
	if (!sessp->Connect(error)) {
		sessp->Put();
		return 0;
	}
	return sessp;
}

HfpSession *HfpService::
Connect(bdaddr_t const &bdaddr, ErrorInfo *error)
{
	HfpSession *sessp = GetSession(bdaddr);
	if (!sessp) {
		if (error)
			error->SetNoMem();
		return 0;
	}
	if (!sessp->Connect(error)) {
		sessp->Put();
		return 0;
	}
	return sessp;
}

HfpSession *HfpService::
Connect(const char *addrstr, ErrorInfo *error)
{
	HfpSession *sessp = GetSession(addrstr);
	if (!sessp) {
		if (error)
			error->SetNoMem();
		return 0;
	}
	if (!sessp->Connect(error)) {
		sessp->Put();
		return 0;
	}
	return sessp;
}

bool HfpService::
SetScoEnabled(bool sco_enable, ErrorInfo *error)
{
	if (m_sco_enable == sco_enable)
		return true;

	if (GetHub()->IsStarted()) {
		if (!sco_enable) {
			ScoCleanup();

		}
		else if (!ScoListen(error)) {
			return false;
		}
	}

	m_sco_enable = sco_enable;
	return true;
}

bool HfpService::
SetCaps(int caps, ErrorInfo *error)
{
	int old_caps = m_brsf_my_caps;

	m_brsf_my_caps = caps;

	if ((m_sdp_rec != 0) && ((old_caps ^ caps) & 0x1f)) {
		SdpUnregister();
		if (!SdpRegister(error)) {
			/* Now we're just screwed! */
			m_brsf_my_caps = old_caps;
			(void) SdpRegister(0);
			return false;
		}
	}

	return true;
}

bool HfpService::
SetServiceName(const char *val, ErrorInfo *error)
{
	char *oldval, *newval = 0;

	oldval = m_svc_name;
	m_svc_name = 0;
	if (val && !val[0])
		val = 0;
	if (val) {
		m_svc_name = newval = strdup(val);
		if (!m_svc_name) {
			m_svc_name = oldval;
			if (error)
				error->SetNoMem();
			return false;
		}
	}

	if (m_sdp_rec != 0) {
		SdpUnregister();
		if (!SdpRegister(error)) {
			/* Now we're just screwed! */
			m_svc_name = oldval;
			if (newval)
				free(newval);
			(void) SdpRegister(0);
			return false;
		}
	}

	if (oldval)
		free(oldval);
	return true;
}

bool HfpService::
SetServiceDesc(const char *val, ErrorInfo *error)
{
	char *oldval, *newval = 0;

	oldval = m_svc_desc;
	m_svc_desc = 0;
	if (val && !val[0])
		val = 0;
	if (val) {
		m_svc_desc = newval = strdup(val);
		if (!m_svc_desc) {
			m_svc_desc = oldval;
			if (error)
				error->SetNoMem();
			return false;
		}
	}

	if (m_sdp_rec != 0) {
		SdpUnregister();
		if (!SdpRegister(error)) {
			m_svc_desc = oldval;
			if (newval)
				free(newval);
			(void) SdpRegister(0);
			return false;
		}
	}

	if (oldval)
		free(oldval);
	return true;
}

HfpSession::
HfpSession(HfpService *svcp, BtDevice *devp)
	: RfcommSession(svcp, devp), m_conn_state(BTS_Disconnected),
	  m_command_timer(0),
	  m_chld_0(false), m_chld_1(false), m_chld_1x(false),
	  m_chld_2(false), m_chld_2x(false),m_chld_3(false), m_chld_4(false),
	  m_clip_enabled(false), m_ccwa_enabled(false),
	  m_inum_service(0), m_inum_call(0), m_inum_callsetup(0),
	  m_inum_callheld(0), m_inum_signal(0), m_inum_roam(0),
	  m_inum_battchg(0),
	  m_inum_names(NULL), m_inum_names_len(0),
	  m_state_service(false), m_state_call(false), m_state_callsetup(0),
	  m_state_signal(-1), m_state_roam(-1), m_state_battchg(-1),
	  m_state_bvra(false), m_state_bsir(false), m_state_ecnr(false),
	  m_state_vgm(-1), m_state_vgs(-1),
	  m_sco_state(BVS_Invalid), m_sco_sock(-1), m_sco_nonblock(false),
	  m_sco_not(0), m_callsetup_presumed(false), m_timer(0),
	  m_clip_timer(0), m_clip_state(CLIP_UNKNOWN), m_clip_value(0),
	  m_timeout_ring(5000), m_timeout_ring_ccwa(20000),
	  m_timeout_dial(20000), m_timeout_clip(250), m_timeout_command(30000),
	  m_rsp_start(0), m_rsp_len(0)
{
}

HfpSession::
~HfpSession()
{
	if (m_timer) {
		delete m_timer;
		m_timer = 0;
	}
	if (m_clip_timer) {
		delete m_clip_timer;
		m_clip_timer = 0;
	}
	if (m_command_timer) {
		delete m_command_timer;
		m_command_timer = 0;
	}
	assert(m_conn_state == BTS_Disconnected);
	assert(m_commands.Empty());
	assert(!m_inum_names);
	assert(m_sco_state == BVS_Invalid);
	assert(m_sco_sock < 0);
	assert(!m_sco_not);
}

void HfpSession::
NotifyConnectionState(ErrorInfo *async_error)
{
	ErrorInfo local_error;

	if (IsRfcommConnecting()) {
		assert(m_conn_state == BTS_Disconnected);
		m_conn_state = BTS_RfcommConnecting;
		assert(!async_error);
		if (cb_NotifyConnection.Registered())
			cb_NotifyConnection(this, 0);
	}

	else if (IsRfcommConnected()) {
		/*
		 * BTS_Disconnected would indicate an inbound connection.
		 * BTS_RfcommConnecting would indicate outbound.
		 */
		assert(!async_error);
		assert((m_conn_state == BTS_RfcommConnecting) ||
		       (m_conn_state == BTS_Disconnected));
		m_conn_state = BTS_Handshaking;

		if (!m_timer) {
			m_timer = GetDi()->NewTimer();
			if (m_timer)
				m_timer->Register(this, &HfpSession::Timeout);
		}
		if (!m_clip_timer) {
			m_clip_timer = GetDi()->NewTimer();
			if (m_clip_timer)
				m_clip_timer->Register(this,
						&HfpSession::ClipTimeout);
		}
		if (!m_command_timer) {
			m_command_timer = GetDi()->NewTimer();
			if (m_command_timer)
				m_command_timer->Register(this,
						&HfpSession::CommandTimeout);
		}

		if (!m_timer || !m_clip_timer || !m_command_timer) {
			local_error.SetNoMem();
			__Disconnect(&local_error, false);
		}

		if (IsRfcommConnected() && !HfpHandshake(&local_error)) {
			__Disconnect(&local_error, false);
		}

		if (IsRfcommConnected() &&
		    IsConnectionRemoteInitiated() &&
		    cb_NotifyConnection.Registered())
			cb_NotifyConnection(this, 0);
	}

	else {
		/* This is all handled by __Disconnect */
		assert(m_conn_state == BTS_Disconnected);
		assert(!IsConnectingAudio());
		assert(!IsConnectedAudio());
		UpdateCallSetup(0);
	}

	if (async_error && cb_NotifyConnection.Registered())
		cb_NotifyConnection(this, async_error);
}

void HfpSession::
__Disconnect(ErrorInfo *reason, bool voluntary)
{
	ErrorInfo dfl_error;

	/* Trash the command list */
	while (!m_commands.Empty()) {
		DeleteFirstCommand(false);
	}

	dfl_error.Set(LIBHFP_ERROR_SUBSYS_BT,
		      LIBHFP_ERROR_BT_USER_DISCONNECT,
		      "User initiated disconnection");

	/*
	 * notify = true: Do both AudioState/Packet callbacks synchronously,
	 * notify = false: Do only the SoundIo callback asynchronously.
	 */
	__DisconnectSco(reason != 0, true, reason == 0,
			reason ? *reason : dfl_error);

	CleanupIndicators();

	if (m_conn_state != BTS_Disconnected) {
		m_conn_state = BTS_Disconnected;
	}
	m_rsp_start = m_rsp_len = 0;
	m_chld_0 = m_chld_1 = m_chld_1x = m_chld_2 = m_chld_2x = m_chld_3 =
		m_chld_4 = false;
	m_clip_enabled = false;
	m_ccwa_enabled = false;
	m_inum_service = m_inum_call = m_inum_callsetup = m_inum_callheld = 0;
	m_inum_signal = m_inum_roam = m_inum_battchg = 0;
	m_state_service = false;
	m_state_call = false;
	m_state_signal = m_state_roam = m_state_battchg = -1;
	m_state_bvra = m_state_bsir = m_state_ecnr = false;
	m_state_vgm = m_state_vgs = -1;

	RfcommSession::__Disconnect(reason, voluntary);
}

bool HfpSession::
Connect(ErrorInfo *error)
{
	if (m_conn_state != BTS_Disconnected)
		return true;

	m_conn_state = BTS_RfcommConnecting;
	if (RfcommConnect(error))
		return true;

	m_conn_state = BTS_Disconnected;
	return false;
}

void HfpSession::
AutoReconnect(void)
{
	assert(m_conn_state == BTS_Disconnected);

	if (Connect() && cb_NotifyConnection.Registered())
		cb_NotifyConnection(this, 0);

	/*
	 * We could invoke cb_NotifyConnectionState here, but
	 * we don't.  Most clients will use this notification to
	 * trigger a name query.  Name queries are most efficiently
	 * processed after the RFCOMM service level connection has
	 * completed, which we also submit a notification for.
	 */
}

void HfpSession::
SdpSupportedFeatures(uint16_t features)
{
	assert(!IsConnected());
	SetSupportedFeatures(features);
}


bool HfpSession::
ScoGetParams(int ssock, ErrorInfo *error)
{
	struct sco_conninfo sci;
	struct sco_options sopts;
	socklen_t size;

	size = sizeof(sci);
	if (getsockopt(ssock, SOL_SCO, SCO_CONNINFO, &sci, &size) < 0) {
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Query SCO_CONNINFO: %s",
				 strerror(errno));
		return false;
	}

	size = sizeof(sopts);
	if (getsockopt(ssock, SOL_SCO, SCO_OPTIONS, &sopts, &size) < 0) {
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Query SCO_OPTIONS: %s",
				 strerror(errno));
		return false;
	}

	m_sco_use_tiocoutq = false;
#if 0
	int outq;

	outq = 0;
	/* Test support for TIOCOUTQ */
	if (!ioctl(m_sco_sock, TIOCOUTQ, &outq)) {
		m_sco_use_tiocoutq = true;
	} else if (errno != EOPNOTSUPP) {
		GetDi()->LogWarn("SCO TIOCOUTQ: unexpected errno %d", errno);
	}
#endif

	m_sco_handle = sci.hci_handle;
	m_sco_mtu = sopts.mtu;
	m_sco_packet_samps = ((sopts.mtu > 48) ? 48 : sopts.mtu) / 2;
	return true;
}

bool HfpSession::
ScoConnect(ErrorInfo *error)
{
	struct sockaddr_sco src, dest;
	int ssock, err;
	BtHci *hcip;

	if (!GetService()->GetScoEnabled()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_BAD_SCO_CONFIG,
				   "SCO audio support disabled by "
				   "configuration");
		return false;
	}

	if (!IsConnected()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_NOT_CONNECTED,
				   "No service-level connection for device");
		return false;
	}
	if (IsConnectedAudio() || IsConnectingAudio()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_SOUNDIO,
				   LIBHFP_ERROR_SOUNDIO_ALREADY_OPEN,
				   "Audio connection already present "
				   "or in progress");
		return false;
	}

	hcip = GetHub()->GetHci();
	if (!hcip) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	memset(&src, 0, sizeof(src));
	src.sco_family = AF_BLUETOOTH;
	bacpy(&src.sco_bdaddr, BDADDR_ANY);

	memset(&dest, 0, sizeof(dest));
	dest.sco_family = AF_BLUETOOTH;
	bacpy(&dest.sco_bdaddr, &GetDevice()->GetAddr());

	/* Now create the socket and connect */

	ssock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (ssock < 0) {
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Create SCO socket: %s",
				 strerror(errno));
		return false;
	}

	if (bind(ssock, (struct sockaddr*)&src, sizeof(src)) < 0) {
		close(ssock);
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Bind SCO socket: %s",
				 strerror(errno));
		return false;
	}

	if (!SetNonBlock(ssock, true)) {
		close(ssock);
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Set SCO socket nonblocking: %s",
				 strerror(errno));
		return false;
	}

	if (connect(ssock, (struct sockaddr*)&dest, sizeof(dest)) < 0) {
		if ((errno != EINPROGRESS) && (errno != EAGAIN)) {
			err = errno;
			close(ssock);
			GetDi()->LogWarn(error,
					 LIBHFP_ERROR_SUBSYS_BT,
					 LIBHFP_ERROR_BT_SYSCALL,
					 "Connect SCO socket: %s",
					 strerror(err));
			return false;
		}
	}

	assert(m_sco_not == 0);
	m_sco_not = GetDi()->NewSocket(ssock, true);
	m_sco_not->Register(this, &HfpSession::ScoConnectNotify);

	m_sco_state = BVS_SocketConnecting;
	m_sco_sock = ssock;
	m_sco_nonblock = true;
	BufCancelAbort();
	return true;
}

bool HfpSession::
ScoAccept(int ssock)
{
	if (!IsConnected()) {
		return false;
	}
	if (IsConnectedAudio() || IsConnectingAudio()) {
		return false;
	}
	m_sco_state = BVS_SocketConnecting;
	m_sco_sock = ssock;
	m_sco_nonblock = false;
	ScoConnectNotify(NULL, ssock);
	return true;
}

void HfpSession::
ScoConnectNotify(SocketNotifier *notp, int /*fh*/)
{
	int sockerr;
	socklen_t sl;
	ErrorInfo error;

	assert(m_sco_state == BVS_SocketConnecting);

	if (notp) {
		assert(notp == m_sco_not);
		delete m_sco_not;
		m_sco_not = 0;
	}

	m_sco_state = BVS_Connected;
	assert(m_sco_not == 0);

	/* Check for a connect error */
	sl = sizeof(sockerr);
	if (getsockopt(m_sco_sock, SOL_SOCKET, SO_ERROR,
		       &sockerr, &sl) < 0) {
		GetDi()->LogWarn(&error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Retrieve status of SCO connect: %s",
				 strerror(errno));
		__DisconnectSco(true, true, false, error);
		return;
	}

	if (sockerr) {
		GetDi()->LogWarn(&error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "SCO connect: %s", strerror(sockerr));
		__DisconnectSco(true, true, false, error);
		return;
	}

	/* Retrieve the connection parameters */
	if (!ScoGetParams(m_sco_sock, &error)) {
		/* Both callbacks synchronous */
		__DisconnectSco(true, true, false, error);
		return;
	}

	BufOpen(m_sco_packet_samps, 2);

	if (cb_NotifyAudioConnection.Registered())
		cb_NotifyAudioConnection(this, 0);
}

void HfpSession::
ScoDataNotify(SocketNotifier *notp, int fh)
{
	int outq = 0;
	assert(fh == m_sco_sock);
	assert(notp == m_sco_not);
	assert(IsConnectedAudio());

	SndPushInput(true);

	if (!IsConnectedAudio())
		return;

	if (m_sco_use_tiocoutq) {
		if (ioctl(m_sco_sock, TIOCOUTQ, &outq)) {
			GetDi()->LogWarn("SCO TIOCOUTQ: %s",
					 strerror(errno));
			outq = m_hw_outq;
		} else {
			outq /= 2;
		}
	} else {
		outq = m_hw_outq;
	}

	BufProcess(outq, false, false);
}


/*
 * The SCO disconnect logic has two potential callback invocations to do
 * and is complex.
 *
 * When disconnect is called, and a state transition is possible,
 * we do the state transition immediately.  If we did the state transition,
 * and async = true, we schedule SndHandleAbort() to be called later via
 * BufAbort().  Otherwise we call SndHandleAbort() in-line.
 *
 * If we did not do a state transition, but noticed a pending call to
 * SndHandleAbort() (m_abort = true), and async = false, we call
 * SndHandleAbort() directly.  The call to BufClose() will cancel the
 * pending call to SndHandleAbort().
 *
 * Each time __DisconnectSco() is called, and either we did the state
 * transition, or noticed a pending call to SndHandleAbort(), we remove
 * the unrequested callback invocations from the pending set, e.g. if
 * two calls are made to __DisconnectSco(), one with notifyvs=false,
 * the other with notifyp=false, no callbacks will be invoked.
 */

void HfpSession::
SndHandleAbort(ErrorInfo reason)
{
	ErrorInfo copy, *usenvs;
	assert(m_sco_sock < 0);

	usenvs = &reason;
	if (m_sco_nas_pending && m_sco_nvs_pending) {
		copy = reason;
		usenvs = &copy;
	}

	BufClose();
	if (m_sco_nas_pending) {
		m_sco_nas_pending = false;
		if (cb_NotifyAsyncStop.Registered())
			cb_NotifyAsyncStop(this, reason);
	}
	if (m_sco_nvs_pending) {
		m_sco_nvs_pending = false;
		if (cb_NotifyAudioConnection.Registered())
			cb_NotifyAudioConnection(this, usenvs);
	}
}

void HfpSession::
__DisconnectSco(bool notifyvs, bool notifyp, bool async, ErrorInfo &error)
{
	bool new_abort = false;

	if (m_sco_sock >= 0) {
		assert(!m_abort.IsSet());
		assert(IsConnectingAudio() || IsConnectedAudio());
		new_abort = true;

		m_sco_nvs_pending = true;
		m_sco_nas_pending = true;

		/*
		 * Skip cb_NotifyAsyncStop() if async audio handling
		 * isn't enabled
		 */
		if (!SndIsAsyncStarted())
			m_sco_nas_pending = false;

		SndAsyncStop();
		assert(!m_sco_not);
		close(m_sco_sock);
		m_sco_sock = -1;
		m_sco_state = BVS_Invalid;
		m_sco_nonblock = false;
	}

	/*
	 * m_abort   -> Call to SndHandleAbort() is pending
	 * new_abort -> State transition effected in this call
	 */
	if (m_abort.IsSet() || new_abort) {
		if (!notifyvs)
			m_sco_nvs_pending = false;
		if (!notifyp)
			m_sco_nas_pending = false;

		if (async)
			/* This will set m_abort and schedule */
			BufAbort(GetDi(), error);
		else
			/* This will invoke methods directly */
			SndHandleAbort(error);
	}
}


/*
 * SoundIo interface implementation
 */

bool HfpSession::
SndOpen(bool play, bool capture, ErrorInfo *error)
{
	/* We only support full duplex operation */
	if (!play || !capture) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_SOUNDIO,
				   LIBHFP_ERROR_SOUNDIO_DUPLEX_MISMATCH,
				   "Device must stream in full duplex mode");
	}
	return ScoConnect(error);
}

void HfpSession::
SndClose(void)
{
	ErrorInfo error;

	/* No callbacks from this path */
	__DisconnectSco(false, false, false, error);
}

void HfpSession::
SndGetProps(SoundIoProps &props) const
{
	props.has_clock = true;
	props.does_source = IsConnectedAudio();
	props.does_sink = IsConnectedAudio();
	props.does_loop = false;
	props.remove_on_exhaust = false;
	props.outbuf_size = 0;
}

void HfpSession::
SndGetFormat(SoundIoFormat &format) const
{
	/* This is the Bluetooth audio format */
	format.samplerate = 8000;
	format.sampletype = SIO_PCM_S16_LE;
	format.nchannels = 1;
	format.bytes_per_record = 2;
	format.packet_samps = m_sco_packet_samps;
}

bool HfpSession::
SndSetFormat(SoundIoFormat &format, ErrorInfo *error)
{
	if (!IsConnectedAudio() ||
	    (format.samplerate != 8000) ||
	    (format.sampletype != SIO_PCM_S16_LE) ||
	    (format.nchannels != 1) ||
	    (format.packet_samps != m_sco_packet_samps)) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_SOUNDIO,
				   LIBHFP_ERROR_SOUNDIO_FORMAT_MISMATCH,
				   "Device requires 8KHz, S16_LE, 1ch");
		return false;
	}
	return true;
}

void HfpSession::
SndPushInput(bool nonblock)
{
	unsigned int nsamples;
	uint8_t *buf;
	ssize_t res;
	int err;

	if (!IsConnectedAudio()) { return; }

	if (m_sco_nonblock != nonblock) {
		if (!SetNonBlock(m_sco_sock, nonblock)) {
			GetDi()->LogWarn("SCO set nonblock failed");
		}
		m_sco_nonblock = nonblock;
	}

	while (1) {
		nsamples = 0;
		m_input.GetUnfilled(buf, nsamples);
		if (!nsamples) {
			GetDi()->LogWarn("SCO input buffer is zero-length?");
			return;
		}
		res = read(m_sco_sock, buf, nsamples * 2);
		if (res < 0) {
			err = errno;
			if ((err != EAGAIN) &&
			    (err != ECONNRESET)) {
				GetDi()->LogWarn("Read SCO data: %s",
						 strerror(err));
			}
			if (ReadErrorFatal(err)) {
				/* Connection is lost */
				res = 0;
			} else {
				/* Just ignore it */
				return;
			}
		}
		if (res == 0) {
			ErrorInfo error;

			error.Set(LIBHFP_ERROR_SUBSYS_BT,
				  LIBHFP_ERROR_BT_SYSCALL,
				  "SCO Connection reset");

			/* Connection lost: both callbacks asynchronous */
			__DisconnectSco(true, true, true, error);
			return;
		} else if (res != (ssize_t) (nsamples * 2)) {
			GetDi()->LogWarn("SCO short read: expected:%d "
					 "got:%zd", (nsamples*2), res);
		}

		m_input.PutUnfilled(res / 2);

		/*
		 * HACK: BlueZ support for SIOCOUTQ is not universally present.
		 *
		 * To improvise, we assume that the queue grows every time we
		 * submit a packet, and shrinks symmetrically each time we
		 * receive a packet.
		 *
		 * This might cause growing latency that does not self-correct.
		 */
		if (!m_sco_use_tiocoutq) {
			if ((ssize_t) m_hw_outq < (res / 2))
				m_hw_outq = 0;
			else
				m_hw_outq -= (res / 2);
		}
	}
}

void HfpSession::
SndPushOutput(bool nonblock)
{
	unsigned int nsamples;
	uint8_t *buf;
	ssize_t res;
	int err;

	if (!IsConnectedAudio()) { return; }

	if (m_sco_nonblock != nonblock) {
		if (!SetNonBlock(m_sco_sock, nonblock)) {
			GetDi()->LogWarn("SCO set nonblock failed");
		}
		m_sco_nonblock = nonblock;
	}

	while (1) {
		nsamples = 0;
		m_output.Peek(buf, nsamples);
		nsamples -= (nsamples % m_sco_packet_samps);
		if (!nsamples) { return; }

		res = send(m_sco_sock, buf, nsamples * 2, MSG_NOSIGNAL);

		if (res < 0) {
			err = errno;
			if (err != EAGAIN) {
				GetDi()->LogWarn("Write SCO data: %s",
						 strerror(err));
			}
			if (WriteErrorFatal(err)) {
				ErrorInfo error;

				error.Set(LIBHFP_ERROR_SUBSYS_BT,
					  LIBHFP_ERROR_BT_SYSCALL,
					  "Fatal SCO write error: %s",
					  strerror(err));

				/* Connection lost: both cbs asynchronous */
				__DisconnectSco(true, true, true, error);
			}
			return;
		}
		if (res != (ssize_t) (nsamples * 2)) {
			GetDi()->LogWarn("SCO short write: expected:%d "
					 "got:%zd", (nsamples*2), res);
		}
		m_output.Dequeue(res / 2);
		if (!m_sco_use_tiocoutq)
			m_hw_outq += (res / 2);
	}
}

bool HfpSession::
SndAsyncStart(bool play, bool capture, ErrorInfo *error)
{
	if (!play || !capture) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_SOUNDIO,
				   LIBHFP_ERROR_SOUNDIO_DUPLEX_MISMATCH,
				   "Device must stream in full duplex mode");
		return false;
	}
	if (!IsConnectedAudio()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_SOUNDIO,
				   LIBHFP_ERROR_SOUNDIO_DUPLEX_MISMATCH,
				   "Not connected to audio gateway");
		return false;
	}
	if (m_sco_not != NULL) { return true; }
	m_sco_not = GetDi()->NewSocket(m_sco_sock, false);
	if (!m_sco_not) {
		if (error)
			error->SetNoMem();
		return false;
	}

	m_sco_not->Register(this, &HfpSession::ScoDataNotify);
	m_hw_outq = 0;
	return true;
}

void HfpSession::
SndAsyncStop(void)
{
	if (m_sco_not) {
		delete m_sco_not;
		m_sco_not = 0;
	}
	BufStop();

	/* Cancel a potential pending SoundIo abort notification */
	if (m_abort.IsSet())
		m_sco_nas_pending = false;
}


/*
 * Input processing
 */

class AtCommand {
	friend class HfpSession;
	friend class HfpPendingCommandImpl;

	ListItem		m_links;
	HfpSession		*m_sess;
	HfpPendingCommand	*m_pend;
	bool			m_dynamic_cmdtext;

	bool iResponse(const char *buf) {
		if (!strcmp(buf, "OK")) {
			return OK();
		}
		if (!strcmp(buf, "ERROR")) {
			ERROR();
			return true;
		}
		return Response(buf);
	}

	bool Cancel(void) {
		return m_sess->CancelCommand(this);
	}

protected:
	HfpSession *GetSession(void) const { return m_sess; }
	DispatchInterface *GetDi(void) const { return m_sess->GetDi(); }
	void CompletePending(ErrorInfo *error, void *info) {
		HfpPendingCommand *cmdp = m_pend;
		if (cmdp) {
			m_pend = 0;
			(*cmdp)(cmdp, error, info);
		}
	}
public:
	const char	*m_command_text;
	virtual bool Response(const char */*buf*/)
		{ return false; };
	virtual bool OK() {
		CompletePending(0, 0);
		return true;
	}
	virtual void ERROR(void) {
		ErrorInfo simple_error;
		simple_error.Set(LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_COMMAND_REJECTED,
				 "Command rejected by device");
		CompletePending(&simple_error, 0);
	}

	void SetText(const char *cmd) {
		if (m_dynamic_cmdtext) {
			free((void*)m_command_text);
			m_command_text = NULL;
		}
		if (cmd) {
			m_command_text = strdup(cmd);
			m_dynamic_cmdtext = true;
		} else {
			m_dynamic_cmdtext = false;
		}
	}

	AtCommand(HfpSession *sessp, char *cmd = NULL)
		: m_sess(sessp), m_pend(0), m_dynamic_cmdtext(false),
		  m_command_text(NULL) {
		if (cmd) { SetText(cmd); }
	}
	AtCommand(HfpSession *sessp, const char *cmd)
		: m_sess(sessp), m_pend(0), m_dynamic_cmdtext(false),
		  m_command_text(cmd) {
	}
	virtual ~AtCommand() {
		ErrorInfo simple_error;
		assert(m_links.Empty());
		if (m_pend) {
			simple_error.Set(LIBHFP_ERROR_SUBSYS_BT,
					 LIBHFP_ERROR_BT_COMMAND_ABORTED,
					 "Command aborted");
			CompletePending(&simple_error, 0);
		}
		SetText(NULL);
	}
};

HfpPendingCommand::
~HfpPendingCommand()
{
}

class HfpPendingCommandImpl : public HfpPendingCommand {
	AtCommand	*m_cmd;

	void Unlink(void) {
		assert(!m_cmd->m_pend || (m_cmd->m_pend == this));
		m_cmd->m_pend = 0;
		m_cmd = 0;
	}

public:
	virtual bool Cancel(void) {
		if (!m_cmd)
			return false;
		return m_cmd->Cancel();
	}

	HfpPendingCommandImpl(AtCommand *cmdp) : m_cmd(cmdp) {
		assert(!m_cmd->m_pend);
		m_cmd->m_pend = this;
	}
	virtual ~HfpPendingCommandImpl() {
		if (m_cmd)
			Unlink();
	}
};


void HfpSession::
HfpDataReady(SocketNotifier *notp, int fh)
{
	size_t cons;
	ssize_t ret;
	int err;
	ErrorInfo error;

	assert(fh == m_rfcomm_sock);
	assert(notp == m_rfcomm_not);
	assert(IsRfcommConnected());

	if (!IsRfcommConnected()) {
		return;
	}

	assert(m_rsp_start + m_rsp_len < sizeof(m_rsp_buf));

	/* Fill m_rsp_buf, try to parse things */
	ret = read(m_rfcomm_sock, &m_rsp_buf[m_rsp_start + m_rsp_len],
		   sizeof(m_rsp_buf) - (m_rsp_start + m_rsp_len));

	if (ret < 0) {
		err = errno;
		GetDi()->LogWarn(&error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Read from RFCOMM socket: %s",
				 strerror(err));

		/*
		 * Some errors do not indicate loss of connection
		 * Others do not indicate voluntary loss of connection
		 */
		if (ReadErrorFatal(err)) {
			__Disconnect(&error, ReadErrorVoluntary(err));
		}
		return;
	}

	if (ret == 0) {
		/* Voluntary disconnection? */
		error.Set(LIBHFP_ERROR_SUBSYS_BT,
			  LIBHFP_ERROR_BT_SYSCALL,
			  "Connection closed by device");
		__Disconnect(&error, true);
		return;
	}

	assert((size_t)ret <= (sizeof(m_rsp_buf) - (m_rsp_start + m_rsp_len)));

	m_rsp_len += ret;

	/* Try to consume the buffer */
	do {
		cons = HfpConsume(&m_rsp_buf[m_rsp_start], m_rsp_len);

		if (!IsRfcommConnected())
			break;

		if (!cons) {
			/* Don't tolerate lines that are too long */
			if ((m_rsp_start + m_rsp_len) == sizeof(m_rsp_buf)) {
				if (m_rsp_start == 0) {
					GetDi()->LogWarn(&error,
					 LIBHFP_ERROR_SUBSYS_BT,
					 LIBHFP_ERROR_BT_PROTOCOL_VIOLATION,
							 "Device sent line "
							 "that is too long");
					__Disconnect(&error);
					return;
				}

				/* Compact the buffer */
				memmove(m_rsp_buf, &m_rsp_buf[m_rsp_start],
					m_rsp_len);
				m_rsp_start = 0;
			}

			return;
		}

		assert(cons <= m_rsp_len);

		if (cons == m_rsp_len) {
			m_rsp_start = m_rsp_len = 0;
		} else {
			m_rsp_start += cons;
			m_rsp_len -= cons;
		}

	} while (m_rsp_len);
}

static bool IsWS(char c) { return ((c == ' ') || (c == '\t')); }
static bool IsNL(char c) { return ((c == '\r') || (c == '\n')); }

size_t HfpSession::
HfpConsume(char *buf, size_t len)
{
	AtCommand *cmdp;
	size_t pos = 0;
	char c;

	/* Are we looking at white space?  Trim it! */
	c = buf[0];
	assert(c);
	if (IsWS(c) || IsNL(c)) {
		do {
			c = buf[++pos];
		} while ((pos < len) && (IsWS(c) || IsNL(c)));
		return pos;
	}

	/* Is there a newline anywhere in the buffer? */
	for (pos = 1; pos < len; pos++) {
		if (IsNL(buf[pos])) {
			buf[pos] = '\0';

			GetDi()->LogDebug(">> %s", buf);

			/* Do we have a waiting command? */
			if (!m_commands.Empty()) {
				cmdp = GetContainer(m_commands.next,
						    AtCommand, m_links);
				if (cmdp->iResponse(buf))
					DeleteFirstCommand();
			}

			if (IsRfcommConnected()) {
				/* Default consume handler */
				ResponseDefault(buf);
			}

			if (IsRfcommConnected() &&
			    (m_conn_state == BTS_Handshaking) &&
			    !IsCommandPending()) {
				/* Handshaking is done */
				HfpHandshakeDone();
			}

			if (pos < (len - 1)) {
				assert(buf[len - 1]);
			}

			return pos + 1;
		}
	}

	/* No newline, nothing to consume */
	return 0;
}

static bool
ParseGsmStringField(char *&buf, const char *&result)
{
	char *xbuf, *xres;
	int count;

	xbuf = buf;
	while (*xbuf && IsWS(*xbuf)) { xbuf++; }
	if (!*xbuf)
		return false;

	if (*xbuf == '"') {
		/* Find the close quote */
		xres = xbuf + 1;
		xbuf = strchr(xres, '"');
		if (!xbuf) { return false; }
		*xbuf = '\0';
		xbuf++;

	} else {
		xres = xbuf;
	}

	/* Find the comma */
	while (*xbuf && (*xbuf != ',')) xbuf++;
	if (*xbuf) {
		count = xbuf - xres;
		*(xbuf++) = '\0';

		/* Trim trailing whitespace */
		while (count && IsWS(xres[count - 1])) {
			xres[--count] = '\0';
		}
	}
	if (!*xres)
		xres = 0;
	buf = xbuf;
	result = xres;
	return true;
}

static bool
ParseGsmIntField(char *&buf, int &result)
{
	char *xbuf;
	char *endp;
	int value;

	xbuf = buf;
	while (*xbuf && IsWS(*xbuf)) { xbuf++; }
	if (!*xbuf)
		return false;

	value = strtol(xbuf, &endp, 10);
	if (endp != xbuf) {
		result = value;
		xbuf = endp;
	}

	while (*xbuf && IsWS(*xbuf)) { xbuf++; }
	if (*xbuf == ',') {
		xbuf++;
	}
	else if (*xbuf) {
		return false;
	}

	buf = xbuf;
	return true;
}

void *GsmResult::
operator new(size_t nb, const char *src_string, size_t extra)
{
	void *mem;
	char *cbuf;

	if (!src_string)
		src_string = "";
	if (!extra)
		extra = strlen(src_string) + 1;
	mem = malloc(nb + extra);
	if (mem) {
		memset(mem, 0, nb);
		cbuf = ((char *) mem) + nb;
		memcpy(cbuf, src_string, extra);
		((GsmResult *) mem)->src_string = cbuf;
		((GsmResult *) mem)->extra = extra;
	}
	return mem;
}

void GsmResult::
operator delete(void *mem)
{
	free(mem);
}

GenericAtCommandResult *GenericAtCommandResult::Parse(const char* result)
{
	GenericAtCommandResult *res = new (result, 0) GenericAtCommandResult;
	if (!res)
		return 0;
	return res;
}

GsmClipResult *GsmClipResult::
Parse(const char *clip)
{
	GsmClipResult *res;
	char *buf;

	res = new (clip, 0) GsmClipResult;
	if (!res)
		return 0;
	buf = res->src_string;

	if (!ParseGsmStringField(buf, res->number))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->type))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmStringField(buf, res->subaddr))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->satype))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmStringField(buf, res->alpha))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->cli_validity))
		goto bad;
	return res;

bad:
	delete res;
	return 0;
}

GsmClipResult *GsmClipResult::
ParseCcwa(const char *clip)
{
	GsmClipResult *res;
	char *buf;
	int class_drop;

	res = new (clip, 0) GsmClipResult;
	if (!res)
		return 0;
	buf = res->src_string;

	if (!ParseGsmStringField(buf, res->number))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->type))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, class_drop))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmStringField(buf, res->alpha))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->cli_validity))
		goto bad;
	return res;

bad:
	delete res;
	return 0;
}

bool GsmClipResult::
Compare(const GsmClipResult *clip) const
{
	const GsmClipResult *src = this;
	if (src->number || clip->number) {
		if (!src->number || !clip->number)
			return false;
		if (strcmp(src->number, clip->number))
			return false;
		if (src->type != clip->type)
			return false;
	}
	if (src->subaddr || clip->subaddr) {
		if (!src->subaddr || !clip->subaddr)
			return false;
		if (strcmp(src->subaddr, clip->subaddr))
			return false;
		if (src->satype != clip->satype)
			return false;
	}
	if (src->alpha || clip->alpha) {
		if (!src->alpha || !clip->alpha)
			return false;
		if (strcmp(src->alpha, clip->alpha))
			return false;
	}
	return true;
}

GsmClipResult *GsmClipResult::
Duplicate(void) const
{
	const GsmClipResult *src = this;
	GsmClipResult *res;
	const char *from;
	char *buf;

	res = new (src->src_string, src->extra) GsmClipResult;
	if (!res)
		return 0;
	buf = res->src_string;
	from = src->src_string;
	*res = *src;
	res->src_string = buf;
	memcpy(buf, from, src->extra);
	if (src->number)
		res->number = buf + (src->number - from);
	if (src->subaddr)
		res->subaddr = buf + (src->subaddr - from);
	if (src->alpha)
		res->alpha = buf + (src->alpha - from);
	return res;
}

GsmCnumResult *GsmCnumResult::
Parse(const char *cops)
{
	GsmCnumResult *res;
	char *buf;

	res = new (cops, 0) GsmCnumResult;
	if (!res)
		return 0;
	buf = res->src_string;

	if (!ParseGsmStringField(buf, res->alpha))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmStringField(buf, res->number))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmIntField(buf, res->type))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->speed))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->service))
		goto bad;
	return res;

bad:
	delete res;
	return 0;
}

GsmCopsResult *GsmCopsResult::
Parse(const char *cops)
{
	GsmCopsResult *res;
	char *buf;

	res = new (cops, 0) GsmCopsResult;
	if (!res)
		return 0;
	buf = res->src_string;

	if (!ParseGsmIntField(buf, res->mode))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmIntField(buf, res->format))
		goto bad;
	if (!*buf)
		goto bad;
	switch (res->format) {
	case 0:
	case 1:
		if (!ParseGsmStringField(buf, res->alpha))
			goto bad;
		break;
	case 2:
		if (!ParseGsmIntField(buf, res->nonalpha))
			goto bad;
		break;
	default:
		goto bad;
	}
	return res;

bad:
	delete res;
	return 0;
}

GsmClccResult *GsmClccResult::
Parse(const char *clcc)
{
	GsmClccResult *res;
	char *buf;

	res = new (clcc, 0) GsmClccResult;
	if (!res)
		return 0;
	buf = res->src_string;

	if (!ParseGsmIntField(buf, res->idx))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmIntField(buf, res->dir))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmIntField(buf, res->stat))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmIntField(buf, res->mode))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmIntField(buf, res->mpty))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmStringField(buf, res->number))
		goto bad;
	if (!*buf)
		goto bad;
	if (!ParseGsmIntField(buf, res->type))
		goto bad;
	if (!*buf)
		return res;
	if (!ParseGsmStringField(buf, res->alpha))
		goto bad;
	return res;

bad:
	delete res;
	return 0;
}


void HfpSession::
SetBvra(bool active)
{
	if (m_state_bvra != active) {
		m_state_bvra = active;
		if (cb_NotifyVoiceRecog.Registered())
			cb_NotifyVoiceRecog(this, active);
	}
}

void HfpSession::
ResponseDefault(char *buf)
{
	char *end;
	GsmClipResult *phnum;
	int indnum;

	if (!strncmp(buf, "+CIEV:", 6)) {
		/* Event notification */
		buf += 6;
		while (buf[0] && IsWS(buf[0])) { buf++; }
		if (!buf[0]) {
			/* Unparseable output? */
			GetDi()->LogWarn("Parse error on CIEV code");
			return;
		}

		indnum = strtol(buf, &buf, 0);
		while (buf[0] && (buf[0] != ',')) { buf++; }
		if (!buf[0] || !buf[1]) {
			/* Unparseable output? */
			GetDi()->LogWarn("Parse error on CIEV code");
			return;
		}
		buf++;

		UpdateIndicator(indnum, buf);
	}

	else if (!strncmp(buf, "RING", 4) && IsConnected()) {
		/* Incoming call notification */
		UpdateCallSetup(1, 1, 0, m_timeout_ring);
	}

	else if (!strncmp(buf, "+CLIP:", 6) && IsConnected()) {
		/* Line identification for incoming call */
		phnum = GsmClipResult::Parse(buf + 6);
		if (!phnum)
			GetDi()->LogWarn("Parse error on CLIP");
		UpdateCallSetup(1, 0, phnum, m_timeout_ring);
		delete phnum;
	}

	else if (!strncmp(buf, "+CCWA:", 6) && IsConnected()) {
		/* Call waiting + line identification for call waiting */
		phnum = GsmClipResult::ParseCcwa(buf + 6);
		if (!phnum)
			GetDi()->LogWarn("Parse error on CCWA");
		UpdateCallSetup(1, 2, phnum, m_timeout_ring_ccwa);
	}

	else if (!strncmp(buf, "+BVRA:", 6) && IsConnected()) {
		buf += 6;
		while (buf[0] && IsWS(buf[0])) { buf++; }
		indnum = strtol(buf, &end, 0);
		if ((end == buf) || (indnum < 0) || (indnum > 1)) {
			GetDi()->LogWarn("Parse error on BVRA");
		} else {
			SetBvra(indnum == 1);
		}
	}

	else if ((!strncmp(buf, "+VGM:", 5) || !strncmp(buf, "+VGM=", 5)) &&
		 IsConnected()) {
		buf += 5;
		while (buf[0] && IsWS(buf[0])) { buf++; }
		indnum = strtol(buf, &end, 0);
		if ((end == buf) || (indnum < 0) || (indnum > 15)) {
			GetDi()->LogWarn("Parse error on VGM");
		} else if (m_state_vgm != indnum) {
			m_state_vgm = indnum;
			if (cb_NotifyVolume.Registered())
				cb_NotifyVolume(this, true, false);
		}
	}

	else if ((!strncmp(buf, "+VGS:", 5) || !strncmp(buf, "+VGS=", 5)) &&
		 IsConnected()) {
		buf += 5;
		while (buf[0] && IsWS(buf[0])) { buf++; }
		indnum = strtol(buf, &end, 0);
		if ((end == buf) || (indnum < 0) || (indnum > 15)) {
			GetDi()->LogWarn("Parse error on VGS");
		} else if (m_state_vgs != indnum) {
			m_state_vgs = indnum;
			if (cb_NotifyVolume.Registered())
				cb_NotifyVolume(this, false, true);
		}
	}

	else if (!strncmp(buf, "+BSIR:", 6) && IsConnected()) {
		buf += 6;
		while (buf[0] && IsWS(buf[0])) { buf++; }
		indnum = strtol(buf, &end, 0);
		if ((end == buf) || (indnum < 0) || (indnum > 1)) {
			GetDi()->LogWarn("Parse error on BSIR");
		} else if (m_state_bsir != indnum) {
			m_state_bsir = (indnum == 1);
			if (cb_NotifyInBandRingTone.Registered())
				cb_NotifyInBandRingTone(this, indnum == 1);
		}
	}
}


/*
 * Command infrastructure
 * Commands:
 *	-are instances of AtCommand, or classes derived from AtCommand
 *	-contain exact text to send to the AG
 *	-include derived procedures
 */

void HfpSession::
CommandTimeout(TimerNotifier *timerp)
{
	ErrorInfo error;

	assert(timerp == m_command_timer);
	GetDi()->LogWarn(&error,
			 LIBHFP_ERROR_SUBSYS_BT,
			 LIBHFP_ERROR_BT_TIMEOUT,
			 "Audio Gateway Command Timeout");
	__Disconnect(&error, false);
}

void HfpSession::
DeleteFirstCommand(bool do_start)
{
	AtCommand *cmd;

	assert(!m_commands.Empty());
	cmd = GetContainer(m_commands.next, AtCommand, m_links);
	cmd->m_links.Unlink();
	delete cmd;

	m_command_timer->Cancel();
	if (do_start && !m_commands.Empty())
		(void) StartCommand(0);
}

bool HfpSession::
AddCommand(AtCommand *cmdp, bool top, ErrorInfo *error)
{
	bool was_empty;

	if (!cmdp) {
		GetDi()->LogWarn("Error allocating AT command(s)");
		if (error)
			error->SetNoMem();
		return false;
	}

	if (!IsRfcommConnected()) {
		delete cmdp;
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_NOT_CONNECTED,
				   "Device is not connected");
		return false;
	}

	was_empty = m_commands.Empty();

	if (top && !was_empty) {
		AtCommand *topcmd;
		topcmd = GetContainer(m_commands.next, AtCommand, m_links);
		topcmd->m_links.PrependItem(cmdp->m_links);
	} else {
		m_commands.AppendItem(cmdp->m_links);
	}

	if (!was_empty)
		return true;
	return StartCommand(error);
}

HfpPendingCommand *HfpSession::
PendingCommand(AtCommand *cmdp, ErrorInfo *error)
{
	HfpPendingCommand *pendp;

	if (!cmdp) {
		if (error)
			error->SetNoMem();
		return 0;
	}

	if (!IsConnected()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_NOT_CONNECTED,
				   "Device is not connected");
		delete cmdp;
		return 0;
	}

	pendp = new HfpPendingCommandImpl(cmdp);
	if (!pendp) {
		delete cmdp;
		if (error)
			error->SetNoMem();
		return 0;
	}

	if (!AddCommand(cmdp, false, error)) {
		delete pendp;
		return 0;
	}

	return pendp;
}

bool HfpSession::
SendCommand(const char *cmd, ErrorInfo *error)
{
	size_t cl;
	char stackbuf[64];
	StringBuffer sb(stackbuf, sizeof(stackbuf));

	if (!sb.AppendFmt("%s\r", cmd)) {
		if (error)
			error->SetNoMem();
		return false;
	}
	cl = strlen(sb.Contents());

	GetDi()->LogDebug("<< %s", cmd);
	return RfcommSend((const uint8_t *) sb.Contents(), cl, error);
}

bool HfpSession::
StartCommand(ErrorInfo *error)
{
	AtCommand *cmdp;

	if (m_commands.Empty())
		return true;

	cmdp = GetContainer(m_commands.next, AtCommand, m_links);

	if (!SendCommand(cmdp->m_command_text, error))
		return false;

	m_command_timer->Set(m_timeout_command);
	return true;
}

bool HfpSession::
CancelCommand(AtCommand *cmdp)
{
	/* Don't throw out the current command */
	if (&cmdp->m_links != m_commands.next) {
		cmdp->m_links.Unlink();
		delete cmdp;
		return true;
	}

	return false;
}


/*
 * Commands used for handshaking
 */

void HfpSession::
SetSupportedHoldRange(int start, int end)
{
	if ((start <= 0) && (end >= 0))
		m_chld_0 = true;
	if ((start <= 1) && (end >= 1))
		m_chld_1 = true;
	if ((start <= 2) && (end >= 2))
		m_chld_2 = true;
	if ((start <= 3) && (end >= 3))
		m_chld_3 = true;
	if ((start <= 4) && (end >= 4))
		m_chld_4 = true;
}

void HfpSession::
SetSupportedHoldModes(const char *hold_mode_list)
{
	char *alloc, *modes, *tok, *rend, *rex, *save;
	int st, end;
	ErrorInfo error;

	m_chld_0 = m_chld_1 = m_chld_1x = m_chld_2 = m_chld_2x = m_chld_3 =
		m_chld_4 = false;

	alloc = strdup(hold_mode_list);
	if (!alloc) {
		GetDi()->LogWarn("Allocation failure in %s", __FUNCTION__);
		error.SetNoMem();
		__Disconnect(&error, false);
		return;
	}

	modes = alloc;
	if (modes[0] != '(')
		goto parsefailed;
	modes++;

	end = strlen(modes);
	while (end && IsWS(modes[end - 1])) end--;
	if (!end || (modes[end - 1] != ')'))
		goto parsefailed;
	modes[end - 1] = '\0';
	end--;

	tok = strtok_r(modes, ",", &save);
	while (tok) {
		while (*tok && IsWS(*tok)) tok++;
		st = strtol(tok, &rend, 10);
		while (*rend && IsWS(*rend)) rend++;
		if (*rend == '-') {
			*(rend++) = '\0';
			end = strtol(rend, &rex, 10);
			while (*rex && IsWS(*rex)) rex++;
			if (*rex)
				goto parsefailed;
			if (st >= end)
				goto parsefailed;
			SetSupportedHoldRange(st, end);
		}
		else if (*rend == 'x') {
			if (!rend[1]) {
				if (st == 1)
					m_chld_1x = true;
				else if (st == 2)
					m_chld_2x = true;
			}
		}
		else if (!*rend) {
			SetSupportedHoldRange(st, st);
		}

		tok = strtok_r(0, ",", &save);
	}

	GetDi()->LogDebug("Hold modes:%s%s%s%s%s%s%s",
			  m_chld_0 ? " 0" : "",
			  m_chld_1 ? " 1" : "",
			  m_chld_1x ? " 1x" : "",
			  m_chld_2 ? " 2" : "",
			  m_chld_2x ? " 2x" : "",
			  m_chld_3 ? " 3" : "",
			  m_chld_4 ? " 4" : "");

	free(alloc);
	return;

parsefailed:
	GetDi()->LogWarn("AG sent unrecognized response to CHLD=?: \"%s\"",
			 hold_mode_list);
	free(alloc);
}

/* Generic At Command that stores the result */
class GenericAtCommand : public AtCommand {
public:
	GenericAtCommandResult *m_res;
	GenericAtCommand(HfpSession *sessp, const char *cmd) : AtCommand(sessp, cmd), m_res(0) {}
	bool Response(const char *buf) {
		m_res = GenericAtCommandResult::Parse(buf);
		return false;
	}
	bool OK(void) {
		ErrorInfo local_error;
		if (!m_res) {
			return AtCommand::OK();
		} else {
			CompletePending(0, m_res);
		}
		return true;
	}
};

/* Cellular Hold Command Test */
class ChldTCommand : public AtCommand {
public:
	ChldTCommand(HfpSession *sessp) : AtCommand(sessp, "AT+CHLD=?") {}
	bool Response(const char *buf) {
		if (!strncmp("+CHLD:", buf, 6)) {
			int pos = 6;
			while (IsWS(buf[pos])) { pos++; }
			GetSession()->SetSupportedHoldModes(&buf[pos]);
		}
		return false;
	}
};

/* Bluetooth Read Supported Features */
class BrsfCommand : public AtCommand {
	int		m_brsf;

public:
	BrsfCommand(HfpSession *sessp, int caps)
		: AtCommand(sessp), m_brsf(0) {
		char tmpbuf[32];
		sprintf(tmpbuf, "AT+BRSF=%d", caps);
		SetText(tmpbuf);
	}

	bool Response(const char *buf) {
		if (!strncmp(buf, "+BRSF:", 6)) {
			int pos = 6;
			while (IsWS(buf[pos])) { pos++; }

			/* Save the output */
			m_brsf = strtol(&buf[pos], NULL, 0);
		}
		return false;
	}

	bool OK(void) {
		GetSession()->SetSupportedFeatures(m_brsf);
		if (GetSession()->FeatureThreeWayCalling()) {
			(void) GetSession()->AddCommand(
				new ChldTCommand(GetSession()), false, 0);
		}
		return AtCommand::OK();
	}
	void ERROR(void) {
		/*
		 * Supported features can also come from the
		 * SDP record of the device, and if we initiated
		 * the connection, we should have them.  If not,
		 * we could get them, but we don't, at least not yet.
		 */
		AtCommand::ERROR();
	}
};

void HfpSession::
SetSupportedFeatures(int ag_features)
{
	assert(IsConnecting());
	m_brsf = ag_features;

	/* Default: in-band ring tones enabled */
	if (FeatureInBandRingTone())
		m_state_bsir = true;

	/* Default: echo cancelation/noise reduction enabled on AG */
	if (FeatureEcnr())
		m_state_ecnr = true;
}

/* Cellular Indicator Test */
class CindTCommand : public AtCommand {
public:
	CindTCommand(HfpSession *sessp) : AtCommand(sessp, "AT+CIND=?") {}

	bool Response(const char *buf) {
		if (!strncmp(buf, "+CIND:", 6)) {
			int pos = 6;
			int parens = 0;
			int indnum = 1;
			while (IsWS(buf[pos])) { pos++; }

			/* Parse the indicator types */
			buf += pos;
			while (*buf) {
				if (*buf == '(') {
					parens++;

					if ((parens == 1) && (buf[1] == '"')) {
						/* Found a name */
						buf += 2;
						pos = 0;
						while (buf[pos] &&
						       (buf[pos] != '"')) {
							pos++;
						}

						if (!buf[pos]) {
							/* Damaged result? */
							break;
						}

						/*
						 * New indicator record
						 */
						GetSession()->
							SetIndicatorNum(indnum,
								buf, pos);

						buf += (pos - 1);
					}
				}

				else if (*buf == ')') {
					parens--;
				}

				else if (!parens && (buf[0] == ',')) {
					indnum++;
				}

				buf++;
			}

			GetSession()->VerifyIndicators();
		}
		return false;
	}
};

class CindRCommand : public AtCommand {
public:
	CindRCommand(HfpSession *sessp) : AtCommand(sessp, "AT+CIND?") {}

	bool Response(const char *buf) {
		if (!strncmp(buf, "+CIND:", 6)) {
			int pos = 6;
			int indnum = 1;
			char *xbuf;

			/* Parse the indicator values */
			buf += pos;
			while (*buf) {
				while (IsWS(buf[0])) { buf++; }

				pos = 0;
				while (buf[pos] && (buf[pos] != ',')) {
					pos++;
				}

				while (pos && IsWS(buf[pos-1])) { pos--; }

				xbuf = (char*) malloc(pos + 1);
				memcpy(xbuf, buf, pos);
				xbuf[pos] = '\0';

				GetSession()->UpdateIndicator(indnum, xbuf);

				free(xbuf);
				buf += pos;
				if (buf[0] == ',') { buf++; }
				indnum++;
			}
		}
		return false;
	}
};

/* Indicator number parser support */
bool StrMatch(const char *string, const char *buf, int len)
{
	int xlen = strlen(string);
	if (xlen != len) { return false; }
	if (strncasecmp(string, buf, len)) { return false; }
	return true;
}

void HfpSession::
CleanupIndicators(void)
{
	if (m_inum_names) {
		int i;
		for (i = 0; i < m_inum_names_len; i++) {
			if (m_inum_names[i]) {
				free((void*)m_inum_names[i]);
			}
		}
		free(m_inum_names);
		m_inum_names_len = 0;
		m_inum_names = 0;
	}
}

void HfpSession::
ExpandIndicators(int min_size)
{
	int new_size;
	const char **inp;

	new_size = m_inum_names_len * 2;
	if (!new_size) {
		new_size = 8;
	}
	if (new_size < min_size) {
		new_size = min_size;
	}

	inp = (const char**) malloc(new_size * sizeof(*inp));

	if (m_inum_names_len) {
		memcpy(inp,
		       m_inum_names,
		       m_inum_names_len * sizeof(*inp));
	}
	memset(inp + m_inum_names_len,
	       0,
	       (new_size - m_inum_names_len) * sizeof(*inp));

	if (m_inum_names) {
		free(m_inum_names);
	}

	m_inum_names = inp;
	m_inum_names_len = new_size;
}

void HfpSession::
SetIndicatorNum(int indnum, const char *buf, int len)
{
	/* Only note indicators that we care something about */
	if (StrMatch("service", buf, len)) {
		m_inum_service = indnum;
	}
	else if (StrMatch("call", buf, len)) {
		m_inum_call = indnum;

		/* Don't propagate to clients */
		len = 0;
	}
	else if (StrMatch("callsetup", buf, len) ||
		 StrMatch("call_setup", buf, len)) {
		if (m_inum_callsetup == 0)
			m_inum_callsetup = indnum;

		/* Don't propagate to clients */
		len = 0;
	}
	else if (StrMatch("callheld", buf, len)) {
		m_inum_callheld = indnum;
	}
	else if (StrMatch("signal", buf, len)) {
		m_inum_signal = indnum;
	}
	else if (StrMatch("roam", buf, len)) {
		m_inum_roam = indnum;
	}
	else if (StrMatch("battchg", buf, len)) {
		m_inum_battchg = indnum;
	}

	/* Remember the name regardless */
	if (indnum >= m_inum_names_len) {
		ExpandIndicators(indnum + 1);
	}
	if (len && (m_inum_names[indnum] == NULL)) {
		char *ndup = (char*) malloc(len + 1);
		m_inum_names[indnum] = ndup;
		/* Convert to lower case while copying */
		while (len--) {
			*(ndup++) = tolower(*(buf++));
		}
		*ndup = '\0';
	}
}

bool HfpSession::
VerifyIndicators(void)
{
	bool res = true;

	if (m_inum_service < 0) {
		GetDi()->LogWarn("AG did not report indicator \"service\"");
		res = false;
	}
	if (m_inum_call < 0) {
		GetDi()->LogWarn("AG did not report indicator \"call\"");
		res = false;
	}

	return res;
}

void HfpSession::
UpdateIndicator(int indnum, const char *buf)
{
	int val;

	if (!indnum) {
		/* We don't know what to do with indicator 0 */
		GetDi()->LogWarn("Got update for indicator 0: \"%s\"", buf);
		return;
	}

	if (!IsConnected()) {
		return;
	}

	val = strtol(buf, NULL, 0);
	if (indnum == m_inum_call) {
		bool newstate = (val != 0);
		if (newstate != m_state_call) {
			m_state_call = newstate;
			if (cb_NotifyCall.Registered())
				cb_NotifyCall(this, true, false, false);

			/*
			 * If the call state has changed, and the
			 * callsetup state is presumed, we reset it.
			 */
			if (IsConnected() &&
			    (newstate == 1) &&
			    m_callsetup_presumed)
				UpdateCallSetup(0);
		}
		return;
	}
	if (indnum == m_inum_callsetup) {
		/*
		 * Certain Motorola RAZR V3 devices will send a
		 * callsetup=1 indicator, but will fail to send
		 * callsetup=0 if the call is remotely terminated
		 * before it is answered or rejected.
		 */
		if (val != 1)
			UpdateCallSetup(val);
		return;
	}

	if (indnum == m_inum_service) {
		m_state_service = (val != 0);
		if (!m_state_service) {
			if (m_timer)
				m_timer->Cancel();
			m_state_call = false;
			UpdateCallSetup(0);
		}
	}
	else if (indnum == m_inum_signal) {
		m_state_signal = val;
	}
	else if (indnum == m_inum_roam) {
		m_state_roam = val;
	}
	else if (indnum == m_inum_battchg) {
		m_state_battchg = val;
	}

	if ((indnum < 0) || (indnum >= m_inum_names_len)) {
		GetDi()->LogWarn("Undefined indicator %d", indnum);
		return;
	}

	if (!m_inum_names[indnum])
		/* Indicator not to be propagated to libhfp clients */
		return;

	/* Some AGs have both "callsetup" and "call_setup" indicators */
	if (cb_NotifyIndicator.Registered())
		cb_NotifyIndicator(this, m_inum_names[indnum], val);
}


void HfpSession::
Timeout(TimerNotifier *notp)
{
	/*
	 * Presumptive call setup timeout has elapsed,
	 * reset the callsetup value to 0.
	 */

	assert(notp == m_timer);
	assert(IsConnected());
	assert(m_callsetup_presumed);
	assert(m_state_callsetup != 0);
	UpdateCallSetup(0);
}

void HfpSession::
ClipTimeout(TimerNotifier */*notp*/)
{
	assert(IsConnected());
	assert(m_clip_state == CLIP_WAITING);
	m_clip_state = CLIP_FINAL;
	UpdateCallSetup(1, 1, 0, m_timeout_ring);
}

void HfpSession::
ClearClip(void)
{
	if (m_clip_state == CLIP_WAITING)
		m_clip_timer->Cancel();
	if (m_clip_value) {
		assert(m_clip_state == CLIP_FINAL);
		delete m_clip_value;
		m_clip_value = 0;
	}
	m_clip_state = CLIP_UNKNOWN;
}

/*
 * UpdateCallSetup is a complicated function that handles specific
 * state machine jobs related to the call, callsetup, and clip states.
 *
 * We track the callsetup state presumptively.  When an event occurs that
 * suggests the callsetup should be in a particular state, we presume
 * it to be in that state regardless of receiving an explicit indicator
 * update from the AG.  For example, if we get a RING event, we know that
 * there is an incoming call, even if the AG doesn't tell is so.  If we
 * presume an incomplete call, we associate a timeout with the presumption.
 * The timeout will be canceled if the AG tells us explicitly what the
 * callsetup state is, or we presume no incomplete call.  This way, we can
 * transparently support AGs that do not report callsetup state.  This is
 * important because callsetup reporting is not required per HFP 1.5.
 *
 * The AG informs us of ringing before it gives us the caller ID
 * information, but it's much cleaner to present the ring notification to
 * our client at the same time as the caller ID info.  We use the CLIP
 * timer to try to make this happen.
 */
void HfpSession::
UpdateCallSetup(int val, int ring, GsmClipResult *clip, int timeout_ms)
{
	bool upd_wc = false, upd_ac = false, do_ring = false;
	GsmClipResult *cpy = 0;
	ErrorInfo local_error;


	GetDi()->LogDebug("UpdateCallSetup: v:%d r:%d clip:\"%s\", to:%d",
			  val, ring, clip
			  ? (clip->number ? clip->number : "[PRIV]")
			  : "[NONE]", timeout_ms);


	if (m_callsetup_presumed && m_timer)
		m_timer->Cancel();
	if (IsConnected() && timeout_ms &&
	    (m_callsetup_presumed || (m_state_callsetup != val))) {
		m_callsetup_presumed = true;
		m_timer->Set(timeout_ms);
	} else {
		m_callsetup_presumed = false;
	}

	switch (val) {
	case 0:
		/* No incomplete call */
		assert(!clip);
		assert(!ring);
		assert(!timeout_ms);
		ClearClip();
		break;
	case 1:
		/* Incomplete incoming call */
		if ((m_state_callsetup != 0) &&
		    (m_state_callsetup != 1))
			ClearClip();

		/* Check the CLIP state, update it */
		switch (m_clip_state) {
		case CLIP_UNKNOWN:
			/* Was one provided? */
			assert(!m_clip_value);
			if (clip) {
				cpy = clip->Duplicate();
				if (cpy) {
					m_clip_value = cpy;
					m_clip_state = CLIP_FINAL;
				}
			} else if (ring) {
				if (m_clip_enabled) {
					/*
					 * We are now waiting for a CLIP,
					 * start the timer.
					 */
					m_clip_timer->Set(m_timeout_clip);
					m_clip_state = CLIP_WAITING;
				} else {
					m_clip_state = CLIP_FINAL;
				}
			}
			break;
		case CLIP_WAITING:
			assert(!m_clip_value);
			if (clip) {
				cpy = clip->Duplicate();
				if (cpy) {
					m_clip_value = cpy;
					m_clip_state = CLIP_FINAL;
					m_clip_timer->Cancel();
				}
				do_ring = true;
			}
			break;
		case CLIP_FINAL:
			if (clip && (!m_clip_value ||
				     !m_clip_value->Compare(clip))) {
				/*
				 * Did we miss an event here?
				 * A different caller ID value is being
				 * reported,
				 */
				cpy = clip->Duplicate();
				if (cpy) {
					m_clip_value = cpy;
					upd_wc = true;
				}
			}
			break;
		default:
			abort();
		}
		if (ring) {
			if ((ring == 1) && m_state_call) {
				m_state_call = false;
				upd_ac = true;
			}
			else if ((ring == 2) && !m_state_call) {
				m_state_call = true;
				upd_ac = true;
			}

			if (!do_ring)
				do_ring = (m_clip_state == CLIP_FINAL);
		}
		break;
	case 2:
	case 3:
		/* Incomplete outgoing call */
		assert(!ring);
		if ((m_state_callsetup != 0) &&
		    (m_state_callsetup != 2) &&
		    (m_state_callsetup != 3))
			ClearClip();

		assert(m_clip_state != CLIP_WAITING);
		if (m_clip_state != CLIP_FINAL) {
			assert(!m_clip_value);
			m_clip_state = CLIP_FINAL;
		}
		if (clip && (!m_clip_value ||
			     !m_clip_value->Compare(clip))) {
			cpy = clip->Duplicate();
			if (cpy) {
				m_clip_value = cpy;
				upd_wc = true;
			}
		}
		break;

	default:
		GetDi()->LogWarn(&local_error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_PROTOCOL_VIOLATION,
				 "Unrecognized callsetup value from device");
		__Disconnect(&local_error, true);
		return;
	}

	if (val != m_state_callsetup) {
		m_state_callsetup = val;
		upd_wc = true;
	}

	if (IsConnected() &&
	    (upd_ac || upd_wc || do_ring) &&
	    cb_NotifyCall.Registered())
		cb_NotifyCall(this, upd_ac, upd_wc, do_ring);
}


/* Cellular Modify Event Reporting */
class CmerCommand : public AtCommand {
public:
	CmerCommand(HfpSession *sessp)
		: AtCommand(sessp, "AT+CMER=3,0,0,1") {}
	void ERROR(void) {
		ErrorInfo error;
		/* This will render the HFP interface almost unusable */
		GetDi()->LogWarn(&error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_PROTOCOL_VIOLATION,
				 "Could not enable AG event reporting");
		GetSession()->__Disconnect(&error, true);
	}
};

/* Cellular Line Identification */
class ClipCommand : public AtCommand {
public:
	ClipCommand(HfpSession *sessp) : AtCommand(sessp, "AT+CLIP=1") {}
	bool OK(void) {
		GetSession()->m_clip_enabled = true;
		return AtCommand::OK();
	}
	/* It is not severely disruptive if this command fails */
};

/* Cellular Call Waiting */
class CcwaCommand : public AtCommand {
public:
	CcwaCommand(HfpSession *sessp) : AtCommand(sessp, "AT+CCWA=1") {}
	bool OK(void) {
		GetSession()->m_ccwa_enabled = true;
		return AtCommand::OK();
	}
};


bool HfpSession::
HfpHandshake(ErrorInfo *error)
{
	assert(m_conn_state == BTS_Handshaking);

	/* This gets cleaned up by RfcommSession */
	assert(m_rfcomm_not == 0);
	m_rfcomm_not = GetDi()->NewSocket(m_rfcomm_sock, false);
	m_rfcomm_not->Register(this, &HfpSession::HfpDataReady);

	if (!AddCommand(new BrsfCommand(this, GetService()->m_brsf_my_caps),
			false, error) ||
	    !AddCommand(new CindTCommand(this), false, error) ||
	    !AddCommand(new CmerCommand(this), false, error) ||
	    !AddCommand(new ClipCommand(this), false, error) ||
	    !AddCommand(new CcwaCommand(this), false, error))
		return false;

	return true;
}

void HfpSession::
HfpHandshakeDone(void)
{
	ErrorInfo error;

	assert(m_conn_state == BTS_Handshaking);

	/* Finished exclusive handshaking */
	m_conn_state = BTS_Connected;

	/* Query current state */
	if (!AddCommand(new CindRCommand(this), false, &error)) {
		__Disconnect(&error, false);
		return;
	}

	assert(IsConnected());

	if (cb_NotifyConnection.Registered())
		cb_NotifyConnection(this, 0);
}


/*
 * Commands
 */

/* Cellular Get Subscriber Number */
class CnumCommand : public AtCommand {
public:
	GsmCnumResult	*m_res;

	CnumCommand(HfpSession *sessp)
		: AtCommand(sessp, "AT+CNUM"), m_res(0) {}
	~CnumCommand() {
		if (m_res) {
			delete m_res;
			m_res = 0;
		}
	}

	bool Response(const char *buf) {
		ErrorInfo local_error;
		if (!strncmp(buf, "+CNUM:", 6)) {
			m_res = GsmCnumResult::Parse(buf + 6);
			/* We could save the error code but we don't */
		}
		return false;
	}

	bool OK(void) {
		ErrorInfo local_error;
		if (!m_res) {
			GetDi()->LogWarn(&local_error,
					 LIBHFP_ERROR_SUBSYS_BT,
					 LIBHFP_ERROR_BT_PROTOCOL_VIOLATION,
					 "Unacceptable response from device");
			CompletePending(&local_error, 0);
		} else {
			CompletePending(0, m_res);
		}
		return true;
	}
};

HfpPendingCommand *HfpSession::
CmdQueryNumber(ErrorInfo *error)
{
	return PendingCommand(new CnumCommand(this), error);
}

/* Cellular Operator Status (two-stage) */
class CopsCommand : public AtCommand {
public:
	bool		m_sent_fmt;
	GsmCopsResult	*m_res;
	CopsCommand(HfpSession *sessp)
		: AtCommand(sessp, "AT+COPS=3,0"),
		  m_sent_fmt(false), m_res(0) {}
	~CopsCommand() {
		if (m_res) {
			delete m_res;
			m_res = 0;
		}
	}
	bool Response(const char *buf) {
		ErrorInfo local_error;
		if (!m_sent_fmt)
			return false;
		if (!strncmp(buf, "+COPS:", 6)) {
			m_res = GsmCopsResult::Parse(buf + 6);
			/* We could save the error code but we don't */
		}
		return false;
	}
	bool OK(void) {
		ErrorInfo local_error;
		if (!m_sent_fmt) {
			/* Send the next stage of the command */
			if (!GetSession()->SendCommand("AT+COPS?",
						       &local_error)) {
				CompletePending(&local_error, 0);
			} else {
				m_sent_fmt = true;
			}
			return false;
		}

		if (!m_res) {
			GetDi()->LogWarn(&local_error,
					 LIBHFP_ERROR_SUBSYS_BT,
					 LIBHFP_ERROR_BT_PROTOCOL_VIOLATION,
					 "Unacceptable response from device");
			CompletePending(&local_error, 0);

		} else {
			CompletePending(0, m_res);
		}

		return true;
	}
};

HfpPendingCommand *HfpSession::
CmdQueryOperator(ErrorInfo *error)
{
	return PendingCommand(new CopsCommand(this), error);
}

/* Cellular List Current Calls */
class ClccCommand : public AtCommand {
public:
	ListItem	m_results;

	ClccCommand(HfpSession *sessp) : AtCommand(sessp, "AT+CLCC") {}
	~ClccCommand() {
		while (!m_results.Empty()) {
			GsmClccResult *resp;
			resp = GetContainer(m_results.next,
					    GsmClccResult,
					    m_links);
			resp->m_links.Unlink();
			delete resp;
		}
	}

	bool Response(const char *buf) {
		ErrorInfo local_error;
		if (!strncmp(buf, "+CLCC:", 6)) {
			GsmClccResult *resp;
			resp = GsmClccResult::Parse(buf + 6);
			if (resp)
				m_results.AppendItem(resp->m_links);
		}
		return false;
	}

	bool OK(void) {
		CompletePending(0, &m_results);
		return true;
	}
};

HfpPendingCommand *HfpSession::
CmdQueryCurrentCalls(ErrorInfo *error)
{
	return PendingCommand(new ClccCommand(this), error);
}

/* Bluetooth Voice Recognition Activation */
class BvraCommand : public AtCommand {
public:
	bool	m_enable;
	BvraCommand(HfpSession *sessp, bool enable)
		: AtCommand(sessp, enable ? "AT+BVRA=1" : "AT+BVRA=0"),
		  m_enable(enable) {}
	~BvraCommand() {}

	bool OK(void) {
		GetSession()->SetBvra(m_enable);
		return AtCommand::OK();
	}
};

HfpPendingCommand *HfpSession::
CmdSetVoiceRecog(bool enabled, ErrorInfo *error)
{
	return PendingCommand(new BvraCommand(this, enabled), error);
}

HfpPendingCommand *HfpSession::
CmdSetVolumeMic(uint8_t vol, ErrorInfo *error)
{
	char buf[32];
	sprintf(buf, "AT+VGM=%d", vol);
	return PendingCommand(new AtCommand(this, buf), error);
}

HfpPendingCommand *HfpSession::
CmdSetVolumeSpeaker(uint8_t vol, ErrorInfo *error)
{
	char buf[32];
	sprintf(buf, "AT+VGM=%d", vol);
	return PendingCommand(new AtCommand(this, buf), error);
}

/* Noise Reduction and Echo Cancellation */
class NrecCommand : public AtCommand {
public:
	bool	m_enable;
	NrecCommand(HfpSession *sessp, bool enable)
		: AtCommand(sessp, enable ? "AT+NREC=1" : "AT+NREC=0"),
		  m_enable(enable) {}
	~NrecCommand() {}

	bool OK(void) {
		GetSession()->m_state_ecnr = m_enable;
		return AtCommand::OK();
	}
};

HfpPendingCommand *HfpSession::
CmdSetEcnr(bool enabled, ErrorInfo *error)
{
	return PendingCommand(new NrecCommand(this, enabled), error);
}

class AtdCommand : public AtCommand {
public:
	AtdCommand(HfpSession *sessp, const char *phnum) : AtCommand(sessp) {
		char buf[64];
		sprintf(buf, "ATD%s;", phnum);
		SetText(buf);
	}
	/* Redial mode - Bluetooth Last Dialed Number */
	AtdCommand(HfpSession *sessp) : AtCommand(sessp, "AT+BLDN") {
	}
	bool OK(void) {
		if (!GetSession()->HasConnectingCall()) {
			GetSession()->UpdateCallSetup(2, 0, NULL,
					      GetSession()->m_timeout_dial);
		}
		return AtCommand::OK();
	}
};

class AtCommandClearCallSetup : public AtCommand {
public:
	AtCommandClearCallSetup(HfpSession *sessp, char *cmd)
		: AtCommand(sessp, cmd) {}
	AtCommandClearCallSetup(HfpSession *sessp, const char *cmd)
		: AtCommand(sessp, cmd) {}
	bool OK(void) {
		/* Emulate call setup change */
		GetSession()->UpdateCallSetup(0);
		return AtCommand::OK();
	}
};

HfpPendingCommand *HfpSession::
CmdAnswer(ErrorInfo *error)
{
	return PendingCommand(new AtCommandClearCallSetup(this, "ATA"),
			      error);
}

HfpPendingCommand *HfpSession::
CmdHangUp(ErrorInfo *error)
{
	return PendingCommand(new AtCommandClearCallSetup(this,"AT+CHUP"),
			      error);
}

bool HfpSession::
ValidPhoneNumChar(char c, ErrorInfo *error)
{
	if (isdigit(c) || (c == '#') || (c == '*') ||
	    (c == 'w') || (c == 'W'))
		return true;

	if (error)
		error->Set(LIBHFP_ERROR_SUBSYS_EVENTS,
			   LIBHFP_ERROR_EVENTS_BAD_PARAMETER,
			   "Invalid char in phone number");
	return false;
}

bool HfpSession::
ValidPhoneNum(const char *ph, ErrorInfo *error)
{
	int len = 0;
	if (!ph)
		return false;
	if ((*ph == '+') || (*ph == '>')) {
		ph++;
		len++;
	}
	while (*ph) {
		if (!ValidPhoneNumChar(*ph, error))
			return false;
		ph++;
		if (++len > PHONENUM_MAX_LEN) {
			if (error)
				error->Set(LIBHFP_ERROR_SUBSYS_EVENTS,
					   LIBHFP_ERROR_EVENTS_BAD_PARAMETER,
					   "Phone number is too long");
			return false;
		}
	}
	return true;
}

HfpPendingCommand *HfpSession::
CmdDial(const char *phnum, ErrorInfo *error)
{
	if (!ValidPhoneNum(phnum, error)) { return 0; }
	return PendingCommand(new AtdCommand(this, phnum), error);
}

HfpPendingCommand *HfpSession::
CmdRedial(ErrorInfo *error)
{
	return PendingCommand(new AtdCommand(this), error);
}

HfpPendingCommand *HfpSession::
CmdSendDtmf(char code, ErrorInfo *error)
{
	char buf[32];

	if (!ValidPhoneNumChar(code, error)) {
		GetDi()->LogWarn("CmdSendDtmf: Invalid DTMF code %d", code);
		return 0;
	}
	sprintf(buf, "AT+VTS=%c", code);
	return PendingCommand(new AtCommand(this, buf), error);
}

HfpPendingCommand *HfpSession::
CmdCallDropHeldUdub(ErrorInfo *error)
{
	if (!m_chld_0) {
		GetDi()->LogWarn("Requested CmdCallDropHeldUdub, "
				 "but AG does not claim support");
	}
	return PendingCommand(new AtCommandClearCallSetup(this,
							  "AT+CHLD=0"),
			      error);
}

HfpPendingCommand *HfpSession::
CmdCallSwapDropActive(ErrorInfo *error)
{
	if (!m_chld_1) {
		GetDi()->LogWarn("Requested CmdCallSwapDropActive, "
				 "but AG does not claim support");
	}
	return PendingCommand(new AtCommand(this, "AT+CHLD=1"), error);
}

HfpPendingCommand *HfpSession::
CmdCallDropIndex(unsigned int actnum, ErrorInfo *error)
{
	char buf[32];

	if (!m_chld_1x) {
		GetDi()->LogWarn("Requested CmdCallActive(%d), "
				 "but AG does not claim support", actnum);
	}
	sprintf(buf, "AT+CHLD=1%d", actnum);
	return PendingCommand(new AtCommand(this, buf), error);
}

HfpPendingCommand *HfpSession::
CmdCallSwapHoldActive(ErrorInfo *error)
{
	if (!m_chld_2) {
		GetDi()->LogWarn("Requested CmdCallSwapHoldActive, "
				 "but AG does not claim support");
	}
	return PendingCommand(new AtCommand(this, "AT+CHLD=2"), error);
}

HfpPendingCommand *HfpSession::
CmdCallPrivateConsult(unsigned int callnum, ErrorInfo *error)
{
	char buf[32];

	if (!m_chld_2x) {
		GetDi()->LogWarn("Requested CmdCallPrivateConsult(%d), "
				 "but AG does not claim support", callnum);
	}
	sprintf(buf, "AT+CHLD=2%d", callnum);
	return PendingCommand(new AtCommand(this, buf), error);
}

HfpPendingCommand *HfpSession::
CmdCallLink(ErrorInfo *error)
{
	if (!m_chld_3) {
		GetDi()->LogWarn("Requested CmdCallLink, "
				 "but AG does not claim support");
	}
	return PendingCommand(new AtCommand(this, "AT+CHLD=3"), error);
}

HfpPendingCommand *HfpSession::
CmdCallTransfer(ErrorInfo *error)
{
	if (!m_chld_4) {
		GetDi()->LogWarn("Requested CmdCallTransfer, "
				 "but AG does not claim support");
	}
	return PendingCommand(new AtCommand(this, "AT+CHLD=4"), error);
}

HfpPendingCommand *HfpSession::
CmdGenericAtCommand(const char *atCmd, ErrorInfo *error)
{
	return PendingCommand(new GenericAtCommand(this, atCmd), error);
}

} /* namespace libhfp */
