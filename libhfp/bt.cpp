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
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include <libhfp/bt.h>

namespace libhfp {


/*
 * Communicating with SDP servers requires a lot more parsing logic
 * than using the HCI command/event interfaces.  The libbluetooth SDP
 * functions do all of this.  Presently, as of time of writing,
 * commonly available libbluetooth does not have asynchronous SDP
 * interfaces.  Rather than reimplementing them, we use a helper thread.
 *
 * Eventually this will be replaced with an asynchronous alternative.
 */

/*
 * Some versions of libbluetooth define the supported features attribute ID
 * as below.
 */
#if !defined(SDP_ATTR_SUPPORTED_FEATURES)
#define SDP_ATTR_SUPPORTED_FEATURES SDP_SUPPORTED_FEATURES
#endif

int SdpAsyncTaskHandler::
SdpLookupChannel(SdpTaskParams &htp)
{
	sdp_session_t *sdp;
	sdp_list_t *srch = NULL, *attrs = NULL, *rsp = NULL;
	uuid_t svclass;
	uint16_t attr, attr2;
	int res, chan, brsf;
	int status = ENOENT;

	sdp = sdp_connect(BDADDR_ANY, &htp.m_bdaddr, SDP_RETRY_IF_BUSY);
	if (sdp == NULL) {
		return errno;
	}

	sdp_uuid16_create(&svclass, htp.m_svclass_id);
	srch = sdp_list_append(NULL, &svclass);
	attr = SDP_ATTR_PROTO_DESC_LIST;
	attrs = sdp_list_append(NULL, &attr);
	attr2 = SDP_ATTR_SUPPORTED_FEATURES;
	attrs = sdp_list_append(attrs, &attr2);

	res = sdp_service_search_attr_req(sdp, srch,
					  SDP_ATTR_REQ_INDIVIDUAL,
					  attrs, &rsp);

	if (res != 0) {
		status = errno;
		goto done;
	}

	for (; rsp && status; rsp = rsp->next) {
		sdp_record_t *rec = (sdp_record_t *) rsp->data;
		sdp_list_t *protos = 0;

		if (!sdp_get_access_protos(rec, &protos) &&
		    ((chan = sdp_get_proto_port(protos, RFCOMM_UUID)) != 0)) {

			htp.m_supported_features_present = false;

			/* See if there's a capabilities record */
			if (!sdp_get_int_attr(rec, SDP_ATTR_SUPPORTED_FEATURES,
					      &brsf)) {
				htp.m_supported_features_present = true;
				htp.m_supported_features = brsf;
			}

			htp.m_channel = chan;
			status = 0;
		}

		sdp_list_free(protos, 0);
	}

done:
	sdp_list_free(srch, 0);
	sdp_list_free(attrs, 0);
	if (rsp) { sdp_list_free(rsp, 0); }
	sdp_close(sdp);
	return status;
}


void SdpAsyncTaskHandler::
SdpTaskThread(int rqfd, int rsfd)
{
	SdpTaskParams itask;
	ssize_t res;

	/*
	 * Breaking the main process in GDB seems to result in
	 * SIGINT sent to children.  This is fatal unless we
	 * block the signal.
	 */
	signal(SIGINT, SIG_IGN);

	while (1) {
		res = read(rqfd, &itask, sizeof(itask));
		if (res < 0) {
			if ((errno == EINTR) ||
			    (errno == ENOMEM) ||
			    (errno == ENOBUFS))
				continue;
			break;
		}

		if (res != sizeof(itask))
			break;

		switch (itask.m_tasktype) {
		case SdpTaskParams::ST_SDP_LOOKUP:
			itask.m_errno = SdpLookupChannel(itask);
			break;
		default:
			assert(0);
		}

		itask.m_complete = true;
		if (write(rsfd, &itask, sizeof(itask)) != sizeof(itask))
			break;
	}
}

void SdpAsyncTaskHandler::
SdpDataReadyNot(SocketNotifier *notp, int fh)
{
	SdpTask *taskp;
	SdpTaskParams itask, *paramsp;
	ssize_t res;
	ErrorInfo error;

	assert(fh == m_rspipe);
	assert(notp == m_rspipe_not);

	if (!m_current_aborted) {
		assert(!m_tasks.Empty());
		taskp = GetContainer(m_tasks.next, SdpTask, m_sdpt_links);
		paramsp = &taskp->m_params;
	} else {
		taskp = 0;
		paramsp = &itask;
	}

	res = read(m_rspipe, paramsp, sizeof(*paramsp));
	if (res != sizeof(*paramsp)) {
		m_ei->LogWarn(&error,
			      LIBHFP_ERROR_SUBSYS_BT,
			      LIBHFP_ERROR_BT_SHUTDOWN,
			      "SDP thread terminated unexpectedly (%zd)",
			      res);
		m_hub->InvoluntaryStop(&error);
		return;
	}

	if (paramsp->m_complete) {
		if (taskp) {
			assert(!m_current_aborted);
			taskp->m_sdpt_links.Unlink();
		} else {
			assert(m_current_aborted);
			m_current_aborted = false;
		}

		if (!m_tasks.Empty())
			SdpNextQueue();
	}

	if (taskp)
		taskp->cb_Result(taskp);
}

void SdpAsyncTaskHandler::
SdpNextQueue(void)
{
	SdpTask *taskp;
	sighandler_t sigsave;
	ssize_t res;
	ErrorInfo error;

	assert(m_rqpipe >= 0);
	assert(!m_current_aborted);
	assert(!m_tasks.Empty());

	taskp = GetContainer(m_tasks.next, SdpTask, m_sdpt_links);

	/*
	 * We never put more than sizeof(SdpTaskParams) through the pipe
	 * before reading a reply, so it should never block.
	 */
	sigsave = signal(SIGPIPE, SIG_IGN);
	res = write(m_rqpipe, &taskp->m_params, sizeof(taskp->m_params));
	(void) signal(SIGPIPE, sigsave);

	if (res != sizeof(taskp->m_params)) {
		/* TODO: async involuntary stop */
		error.Set(LIBHFP_ERROR_SUBSYS_BT,
			  LIBHFP_ERROR_BT_SHUTDOWN,
			  "Short write to SDP task");
		m_hub->InvoluntaryStop(&error);
	}
}

bool SdpAsyncTaskHandler::
SdpQueue(SdpTask *taskp, ErrorInfo *error)
{
	bool was_idle;

	assert(taskp->m_sdpt_links.Empty());

	if (m_rqpipe < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	was_idle = (m_tasks.Empty() && !m_current_aborted);
	m_tasks.AppendItem(taskp->m_sdpt_links);
	if (was_idle)
		SdpNextQueue();
	return true;
}

void SdpAsyncTaskHandler::
SdpCancel(SdpTask *taskp)
{
	assert(!taskp->m_sdpt_links.Empty());
	if (taskp == GetContainer(m_tasks.next, SdpTask, m_sdpt_links))
		m_current_aborted = true;
	taskp->m_sdpt_links.Unlink();
}


bool SdpAsyncTaskHandler::
SdpCreateThread(ErrorInfo *error)
{
	int rqpipe[2], rspipe[2];
	pid_t cpid;
	int fd;

	assert(m_rqpipe == -1);
	assert(m_rspipe == -1);
	assert(m_pid == -1);
	assert(m_tasks.Empty());
	assert(!m_current_aborted);

	if (pipe(rqpipe) < 0) {
		fd = errno;
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "Create SDP request pipe: %s",
				   strerror(fd));
		return false;
	}

	if (pipe(rspipe) < 0) {
		fd = errno;
		close(rqpipe[0]); close(rqpipe[1]);
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "Create SDP result pipe: %s",
				   strerror(fd));
		return false;
	}

	cpid = fork();
	if (cpid < 0) {
		fd = errno;
		close(rqpipe[0]); close(rqpipe[1]);
		close(rspipe[0]); close(rspipe[1]);
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "Fork SDP handler: %s", strerror(fd));
		return false;
	}

	if (cpid > 0) {
		/* Close the unused pipe ends, save them, return */
		close(rqpipe[0]); close(rspipe[1]);
		m_rqpipe = rqpipe[1];
		m_rspipe = rspipe[0];
		m_pid = cpid;

		m_rspipe_not = m_ei->NewSocket(m_rspipe, false);
		if (m_rspipe_not == 0) {
			close(m_rspipe);
			m_rspipe = -1;
			SdpShutdown();
			if (error)
				error->SetNoMem();
			return false;
		}

		m_rspipe_not->Register(this, &SdpAsyncTaskHandler::
				       SdpDataReadyNot);
		return true;
	}

	/*
	 * SDP thread:
	 * Close all file handles except pipes
	 * Run main loop
	 */
	for (fd = 3; fd < 255; fd++) {
		if ((fd != rqpipe[0]) && (fd != rspipe[1]))
			close(fd);
	}

	SdpTaskThread(rqpipe[0], rspipe[1]);
	exit(0);
	return false;
}

void SdpAsyncTaskHandler::
SdpShutdown(void)
{
	if (m_rqpipe >= 0) {
		close(m_rqpipe);
		m_rqpipe = -1;
	}
	if (m_rspipe >= 0) {
		assert(m_rspipe_not != 0);
		delete m_rspipe_not;
		m_rspipe_not = 0;
		close(m_rspipe);
		m_rspipe = -1;
	}
	if (m_pid >= 0) {
		int err, status;
		err = kill(m_pid, SIGKILL);
		if (err)
			m_ei->LogWarn("Send sig to SDP helper process: %s",
				      strerror(errno));
		err = waitpid(m_pid, &status, 0);
		if (err < 0)
			m_ei->LogWarn("Reap SDP helper process: %s",
				      strerror(errno));
		m_pid = -1;
	}

	m_current_aborted = false;

	/* Notify failure of all pending tasks */
	while (!m_tasks.Empty()) {
		SdpTask *taskp;
		taskp = GetContainer(m_tasks.next, SdpTask, m_sdpt_links);
		taskp->m_params.m_complete = true;
		taskp->m_params.m_errno = ECONNRESET;
		taskp->m_sdpt_links.Unlink();
		taskp->cb_Result(taskp);
	}
}

void BtHci::
HciSetStatus(HciTask *taskp, int hcistatus)
{
	if (!hcistatus) {
		taskp->m_error.Clear();
		return;
	}
	taskp->m_error.Set(LIBHFP_ERROR_SUBSYS_BT,
			   LIBHFP_ERROR_BT_HCI,
			   "HCI command failed with status %d", hcistatus);
}

void BtHci::
HciDataReadyNot(SocketNotifier *notp, int fh)
{
	char evbuf[HCI_MAX_EVENT_SIZE + 1];
	hci_event_hdr *hdr;
	ListItem tasks_done, *listp;
	HciTask *taskp;
	inquiry_info *infop = 0;
	inquiry_info_with_rssi *rssip = 0;
	uint8_t count = 0;
	ssize_t ret;
	bool inq_result_rssi = false;
	ErrorInfo error;

	assert(fh == m_hci_fh);
	assert(notp == m_hci_not);

	ret = read(m_hci_fh, evbuf, sizeof(evbuf));
	if (ret < 0) {
		if ((errno == EAGAIN) ||
		    (errno == EINTR) ||
		    (errno == ENOMEM) ||
		    (errno == ENOBUFS))
			return;

		GetDi()->LogWarn(&error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "HCI socket read error: %s",
				 strerror(errno));
		GetHub()->InvoluntaryStop(&error);
		return;
	}

	if (!ret) {
		GetDi()->LogWarn(&error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "HCI socket spontaneously closed");
		GetHub()->InvoluntaryStop(&error);
		return;
	}

	if (evbuf[0] != HCI_EVENT_PKT)
		return;

	if (ret < (1 + HCI_EVENT_HDR_SIZE)) {
		GetDi()->LogError(&error,
				  LIBHFP_ERROR_SUBSYS_BT,
				  LIBHFP_ERROR_BT_SYSCALL,
				  "HCI short read, expect: %d got: %zd",
				  HCI_EVENT_HDR_SIZE + 1, ret);
		GetHub()->InvoluntaryStop(&error);
		return;
	}

	hdr = (hci_event_hdr *) &evbuf[1];
	if (ret < (1 + HCI_EVENT_HDR_SIZE + hdr->plen)) {
		GetDi()->LogError(&error,
				  LIBHFP_ERROR_SUBSYS_BT,
				  LIBHFP_ERROR_BT_SYSCALL,
				  "HCI short read, expect: %d got: %zd",
				  HCI_EVENT_HDR_SIZE + hdr->plen + 1, ret);
		GetHub()->InvoluntaryStop(&error);
		return;
	}

	printf("#####2-0 evt:%d\n", hdr->evt);
	switch (hdr->evt) {
	case EVT_CMD_STATUS: {
		evt_cmd_status *statusp = (evt_cmd_status *) (hdr + 1);
		ret = sizeof(*statusp);
		if (hdr->plen != ret)
			goto invalid_struct;

		GetDi()->LogDebug("HCI Command status: 0x%02x 0x%02x 0x%04x",
				  statusp->status, statusp->ncmd,
				  statusp->opcode);

		/*
		 * Unfortunately the command status event isn't specific
		 * as to which command failed, it only lists an opcode.
		 * We may have many commands pending with the same opcode.
		 * So, we blast them all.
		 */
		listp = m_hci_tasks.next;
		while (listp != &m_hci_tasks) {
			taskp = GetContainer(listp, HciTask, m_hcit_links);
			listp = listp->next;

			if (taskp->m_opcode != statusp->opcode)
				continue;

			switch (statusp->status) {
			case 0:
				if (taskp->m_tasktype == HciTask::HT_INQUIRY)
					taskp->m_submitted = true;
				continue;

			case HCI_COMMAND_DISALLOWED:
				if (!taskp->m_submitted) {
					/* Mark for resubmit */
					taskp->m_resubmit = true;
					m_resubmit_needed = true;
				}
				continue;

			case HCI_REPEATED_ATTEMPTS:
				/* GOOD!! */
				continue;

			default:
				break;
			}

			/* End the task and return the error */
			taskp->m_complete = true;
			HciSetStatus(taskp, statusp->status);
			taskp->m_hcit_links.UnlinkOnly();
			tasks_done.AppendItem(taskp->m_hcit_links);
		}

		if (statusp->ncmd && m_resubmit_needed && !m_resubmit_set) {
			m_resubmit_set = true;
			m_resubmit->Set(1000);
		}

		break;
	}
	case EVT_INQUIRY_RESULT: {
		uint8_t *countp;
		countp = (uint8_t *) (hdr + 1);
		infop = (inquiry_info *) (countp + 1);
		ret = 1;
		if (hdr->plen < ret)
			goto invalid_struct;
		count = *countp;
		ret = 1 + (count * sizeof(*infop));
		if (hdr->plen != ret)
			goto invalid_struct;

		inq_result_rssi = false;

	do_next_inq:
		if (!count)
			break;
		listp = m_hci_tasks.next;
		while (listp != &m_hci_tasks) {
			taskp = GetContainer(listp, HciTask, m_hcit_links);
			listp = listp->next;

			if (taskp->m_tasktype == HciTask::HT_INQUIRY) {
				taskp->m_complete = false;
				bacpy(&taskp->m_bdaddr, &infop->bdaddr);
				taskp->m_pscan = infop->pscan_mode;
				taskp->m_pscan_rep = infop->pscan_rep_mode;
				taskp->m_clkoff = infop->clock_offset;
				taskp->m_devclass =
					(infop->dev_class[2] << 16) |
					(infop->dev_class[1] << 8) |
					infop->dev_class[0];
				taskp->m_hcit_links.UnlinkOnly();
				tasks_done.AppendItem(taskp->m_hcit_links);
			}
		}
		break;
	}
	case EVT_INQUIRY_RESULT_WITH_RSSI: {
		uint8_t *countp;
		countp = (uint8_t *) (hdr + 1);
		rssip = (inquiry_info_with_rssi *) (countp + 1);
		ret = 1;
		if (hdr->plen < ret)
			goto invalid_struct;
		count = *countp;
		ret = 1 + (count * sizeof(*rssip));
		if (hdr->plen != ret)
			goto invalid_struct;

		inq_result_rssi = true;

	do_next_inq_rssi:
		if (!count)
			break;
		listp = m_hci_tasks.next;
		while (listp != &m_hci_tasks) {
			taskp = GetContainer(listp, HciTask, m_hcit_links);
			listp = listp->next;

			if (taskp->m_tasktype == HciTask::HT_INQUIRY) {
				taskp->m_complete = false;
				bacpy(&taskp->m_bdaddr, &rssip->bdaddr);
				taskp->m_pscan_rep = rssip->pscan_rep_mode;
				taskp->m_clkoff = rssip->clock_offset;
				taskp->m_devclass =
					(rssip->dev_class[2] << 16) |
					(rssip->dev_class[1] << 8) |
					rssip->dev_class[0];
				taskp->m_hcit_links.UnlinkOnly();
				tasks_done.AppendItem(taskp->m_hcit_links);
			}
		}
		break;
	}
	case EVT_INQUIRY_COMPLETE: {
		uint8_t st;
		ret = 1;
		if (hdr->plen != ret)
			goto invalid_struct;
		st = *(uint8_t *) (hdr + 1);

		GetDi()->LogDebug("HCI Inquiry complete: 0x%02x", st);

		listp = m_hci_tasks.next;
		while (listp != &m_hci_tasks) {
			taskp = GetContainer(listp, HciTask, m_hcit_links);
			listp = listp->next;

			if (taskp->m_tasktype == HciTask::HT_INQUIRY) {
				taskp->m_complete = true;
				HciSetStatus(taskp, st);
				taskp->m_hcit_links.UnlinkOnly();
				tasks_done.AppendItem(taskp->m_hcit_links);
			}
		}
		break;
	}
	case EVT_REMOTE_NAME_REQ_COMPLETE: {
		int namelen;
		evt_remote_name_req_complete *namep;
		namep =	(evt_remote_name_req_complete *) (hdr + 1);
		ret = EVT_REMOTE_NAME_REQ_COMPLETE_SIZE - sizeof(namep->name);
		printf("#######2 -1\n");
		if (hdr->plen < ret)
			goto invalid_struct;
		namelen = hdr->plen - ret;
		assert(namelen < (int) sizeof(taskp->m_name));
		namep->name[namelen] = '\0';
		{
			char addr[32];
			ba2str(&namep->bdaddr, addr);
			GetDi()->LogDebug("HCI Name request complete (%d): "
					  "\"%s\" -> \"%s\"",
					  namep->status, addr, namep->name);
		}
		printf("#######2 %s\n", namep->name);

		listp = m_hci_tasks.next;
		while (listp != &m_hci_tasks) {
			taskp = GetContainer(listp, HciTask, m_hcit_links);
			listp = listp->next;

			if ((taskp->m_tasktype == HciTask::HT_READ_NAME) &&
			    !bacmp(&taskp->m_bdaddr, &namep->bdaddr)) {
				taskp->m_complete = true;
				HciSetStatus(taskp, namep->status);
				strcpy(taskp->m_name, (char *) namep->name);
				taskp->m_hcit_links.UnlinkOnly();
				tasks_done.AppendItem(taskp->m_hcit_links);
			}
		}
		break;
	}
	}

	while (!tasks_done.Empty()) {
		taskp = GetContainer(tasks_done.next, HciTask, m_hcit_links);
		taskp->m_hcit_links.Unlink();
		if (!taskp->m_complete)
			m_hci_tasks.AppendItem(taskp->m_hcit_links);
		taskp->cb_Result(taskp);
	}

	if (count--) {
		if (inq_result_rssi) {
			rssip++;
			goto do_next_inq_rssi;
		}
		infop++;
		goto do_next_inq;
	}

	return;

invalid_struct:
	GetDi()->LogError(&error,
			  LIBHFP_ERROR_SUBSYS_BT,
			  LIBHFP_ERROR_BT_PROTOCOL_VIOLATION,
			  "HCI structure size mismatch, expect: %d got: %zd",
			  hdr->plen, ret);
	GetHub()->InvoluntaryStop(&error);
}


bool BtHci::
HciSend(int fh, HciTask *taskp, void *data, size_t len, ErrorInfo *error)
{
	uint8_t *buf;
	hci_command_hdr *hdrp;
	ssize_t expect;
	ssize_t ret;
	int es;

	expect = 1 + sizeof(*hdrp) + len;
	buf = (uint8_t *) malloc(expect);
	if (!buf) {
		if (error)
			error->SetNoMem();
		return false;
	}

	buf[0] = HCI_COMMAND_PKT;
	hdrp = (hci_command_hdr *) &buf[1];
	hdrp->opcode = taskp->m_opcode;
	hdrp->plen = len;
	if (len)
		memcpy(hdrp + 1, data, len);

	GetDi()->LogDebug("HCI Submit 0x%04x", hdrp->opcode);

	while (1) {
		ret = send(fh, buf, expect, MSG_NOSIGNAL);
		if ((ret < 0) &&
		    ((errno == EAGAIN) ||
		     (errno == EINTR) ||
		     (errno == ENOMEM) ||
		     (errno == ENOBUFS)))
			continue;

		break;
	}

	free(buf);

	if (ret < 0) {
		es = errno;
		GetDi()->LogError(error,
				  LIBHFP_ERROR_SUBSYS_BT,
				  LIBHFP_ERROR_BT_SYSCALL,
				  "HCI write failed: %s", strerror(es));
		return false;
	}

	if (ret != expect) {
		GetDi()->LogError(error,
				  LIBHFP_ERROR_SUBSYS_BT,
				  LIBHFP_ERROR_BT_SYSCALL,
				  "HCI short write: expected: %zd got: %zd",
				  expect, ret);
		return false;
	}

	return true;
}

bool BtHci::
HciSubmit(int fh, HciTask *taskp, ErrorInfo *error)
{
	switch (taskp->m_tasktype) {
	case HciTask::HT_INQUIRY: {
		printf("#######1 HciTask::HT_INQUIRY\n");
		inquiry_cp req;
		taskp->m_opcode =
			htobs(cmd_opcode_pack(OGF_LINK_CTL, OCF_INQUIRY));
		req.lap[0] = 0x33;
		req.lap[1] = 0x8b;
		req.lap[2] = 0x9e;
		req.length = (taskp->m_timeout_ms + 1279) / 1280;
		req.num_rsp = 0;
		return HciSend(fh, taskp, &req, sizeof(req), error);
        //{
        //    inquiry_info *ii = NULL;
        //    int max_rsp, num_rsp;
        //    int dev_id, sock, len, flags;
        //    int i;
        //    char addr[19] = { 0 };
        //    char name[248] = { 0 };

        //    dev_id = hci_get_route(NULL);
        //    sock = hci_open_dev( dev_id );
        //    if (dev_id < 0 || sock < 0) {
        //        perror("opening socket");
        //        exit(1);
        //    }

        //    len  = 8;
        //    max_rsp = 255;
        //    flags = IREQ_CACHE_FLUSH;
        //    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
        //
        //    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
        //}
        break;

	}
	case HciTask::HT_READ_NAME: {
		printf("#######1 HciTask::HT_READ_NAME\n");
		remote_name_req_cp req;
		taskp->m_opcode =
			htobs(cmd_opcode_pack(OGF_LINK_CTL,
					      OCF_REMOTE_NAME_REQ));
		bacpy(&req.bdaddr, &taskp->m_bdaddr);

        char addr[19] = { 0 };
        ba2str(&req.bdaddr, addr);
        printf("### addr:%s\n", addr);

		req.pscan_mode = taskp->m_pscan;
		req.pscan_rep_mode = taskp->m_pscan_rep;
		req.clock_offset = taskp->m_clkoff;
		return HciSend(fh, taskp, &req, sizeof(req), error);
        //{
        //    char addr[19] = { 0 };
        //    ba2str(&req.bdaddr, addr);
        //    char name[248] = { 0 };
        //    memset(name, 0, sizeof(name));
        //    int ret = hci_read_remote_name(fh, &req.bdaddr, sizeof(name), name, 0);
        //    if (ret < 0) {
        //        strcpy(name, "[unknown]");
        //    }
        //    printf("%s  %s\n", addr, name);
        //    return ret;
        //}
	}
	default:
		abort();
	}
}

void BtHci::
HciResubmit(TimerNotifier *notp)
{
	ListItem *listp;
	HciTask *taskp;

	assert(notp == m_resubmit);
	m_resubmit_needed = false;
	m_resubmit_set = false;

	listp = m_hci_tasks.next;
	while (listp != &m_hci_tasks) {
		taskp = GetContainer(listp, HciTask, m_hcit_links);
		listp = listp->next;

		if (!taskp->m_resubmit)
			continue;

		taskp->m_resubmit = false;
		(void) HciSubmit(m_hci_fh, taskp, 0);
	}
}

bool BtHci::
HciInit(int hci_id, ErrorInfo *error)
{
	struct hci_filter flt;
	int fh, err;

	assert(m_hci_fh == -1);
	assert(!m_hci_not);
	assert(m_hci_tasks.Empty());

	fh = hci_open_dev(hci_id);
	if (fh < 0) {
		err = errno;
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Could not open HCI: %s", strerror(err));
		return false;
	}

	if (hci_devba(hci_id, &m_bdaddr) < 0) {
		err = errno;
		close(fh);
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Get HCI adapter address: %s",
				 strerror(err));
		return false;
	}

	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
	hci_filter_set_event(EVT_CMD_STATUS, &flt);
	hci_filter_set_event(EVT_INQUIRY_RESULT, &flt);
	hci_filter_set_event(EVT_INQUIRY_RESULT_WITH_RSSI, &flt);
	hci_filter_set_event(EVT_INQUIRY_COMPLETE, &flt);
	hci_filter_set_event(EVT_REMOTE_NAME_REQ_COMPLETE, &flt);

	if (setsockopt(fh, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		err = errno;
		close(fh);
		GetDi()->LogWarn(error,
				 LIBHFP_ERROR_SUBSYS_BT,
				 LIBHFP_ERROR_BT_SYSCALL,
				 "Could not set filter on HCI: %s",
				 strerror(err));
		return false;
	}

	m_hci_not = GetDi()->NewSocket(fh, false);
	if (!m_hci_not) {
		close(fh);
		if (error)
			error->SetNoMem();
		return false;
	}

	m_resubmit = GetDi()->NewTimer();
	if (!m_resubmit) {
		close(fh);
		delete m_hci_not;
		m_hci_not = 0;
		if (error)
			error->SetNoMem();
		return false;
	}

	m_resubmit->Register(this, &BtHci::HciResubmit);
	m_hci_not->Register(this, &BtHci::HciDataReadyNot);

	m_hci_id = hci_id;
	m_hci_fh = fh;
	return true;
}

void BtHci::
HciShutdown(void)
{
	if (m_resubmit) {
		delete m_resubmit;
		m_resubmit = 0;
	}

	if (m_hci_not) {
		delete m_hci_not;
		m_hci_not = 0;
	}

	if (m_hci_fh >= 0) {
		close(m_hci_fh);
		m_hci_fh = -1;
	}

	/* Notify failure of all pending tasks */
	while (!m_hci_tasks.Empty()) {
		HciTask *taskp;
		taskp = GetContainer(m_hci_tasks.next, HciTask, m_hcit_links);
		taskp->m_complete = true;
		taskp->m_error.Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth HCI disconnected");
		taskp->m_hcit_links.Unlink();
		taskp->cb_Result(taskp);
	}
}


bool BtHci::
Queue(HciTask *taskp, ErrorInfo *error)
{
	assert(taskp->cb_Result.Registered());
	assert(taskp->m_hcit_links.Empty());

	if (m_hci_fh < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth HCI disconnected");
		return false;
	}

	if (!HciSubmit(m_hci_fh, taskp, error)) {
		assert(!error || error->IsSet());
		return false;
	}

	m_hci_tasks.AppendItem(taskp->m_hcit_links);
	assert(!error || !error->IsSet());
	return true;
}

void BtHci::
Cancel(HciTask *taskp)
{
	assert(!taskp->m_hcit_links.Empty());
	taskp->m_hcit_links.Unlink();

	if ((m_hci_fh >= 0) &&
	    (taskp->m_tasktype == HciTask::HT_INQUIRY)) {
		/* Send an INQUIRY CANCEL command to the HCI */
		taskp->m_opcode = htobs(cmd_opcode_pack(OGF_LINK_CTL,
							OCF_INQUIRY_CANCEL));
		(void) HciSend(m_hci_fh, taskp, 0, 0, 0);
	}
}

bool BtHci::
GetScoMtu(uint16_t &mtu, uint16_t &pkts, ErrorInfo *error)
{
	hci_dev_info di;

	if (m_hci_fh < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	di.dev_id = m_hci_id;
	if (ioctl(m_hci_fh, HCIGETDEVINFO, (void *) &di) < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "HCIGETDEVINFO: %s", strerror(errno));
		return false;
	}

	mtu = di.sco_mtu;
	pkts = di.sco_pkts;
	return true;
}

/* This only works as superuser */
bool BtHci::
SetScoMtu(uint16_t mtu, uint16_t pkts, ErrorInfo *error)
{
	hci_dev_req dr;
	uint16_t *mtup;

	if (m_hci_fh < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	dr.dev_id = m_hci_id;
	mtup = (uint16_t *) &dr.dev_opt;
	mtup[0] = htobs(mtu);
	mtup[1] = htobs(pkts);
	if (ioctl(m_hci_fh, HCISETSCOMTU, (void *) &dr) < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "HCISETSCOMTU: %s", strerror(errno));
		return false;
	}

	return true;
}

bool BtHci::
GetScoVoiceSetting(uint16_t &vs, ErrorInfo *error)
{
	if (m_hci_fh < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	if (hci_read_voice_setting(m_hci_fh, &vs, 1000)) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "hci_read_voice_setting: %s",
				   strerror(errno));
		return false;
	}
	return true;
}

/* This only works as superuser */
bool BtHci::
SetScoVoiceSetting(uint16_t vs, ErrorInfo *error)
{
	if (m_hci_fh < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	if (hci_write_voice_setting(m_hci_fh, vs, 1000)) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "hci_write_voice_setting: %s",
				   strerror(errno));
		return false;
	}
	return true;
}

bool BtHci::
GetDeviceClassLocal(uint32_t &devclass)
{
	int res;
	uint8_t cls[3];

	/*
	 * This is a synchronous operation, and a broken HCI may be able
	 * to hang our main thread for the 1s timeout.
	 */
	if (m_hci_fh < 0)
		return false;

	res = hci_read_class_of_dev(m_hci_fh, cls, 1000);
	if (res < 0)
		return false;

	devclass = (cls[2] << 16) | (cls[1] << 8) | cls[0];
	return true;
}

bool BtHci::
SetDeviceClassLocal(uint32_t devclass)
{
	int res;

	if (m_hci_fh < 0)
		return false;

	res = hci_write_class_of_dev(m_hci_fh, devclass, 1000);

	return (res < 0);
}


BtManaged::
~BtManaged()
{
	assert(m_del_links.Empty());
}

void BtManaged::
DeadRemove(void)
{
	assert(!m_del_links.Empty());
	m_del_links.Unlink();
}

void BtManaged::
Put(void)
{
	assert(m_refs);
	if (!--m_refs) {
		m_hub->DeadObject(this);
	}
}


BtHub::
BtHub(DispatchInterface *eip)
	: m_sdp(NULL), m_ei(eip), m_inquiry_task(0),
	  m_sdp_handler(this, eip), m_hci(0),
	  m_sdp_not(0), m_timer(0),
	  m_autorestart(false), m_autorestart_timeout(5000),
	  m_autorestart_set(false), m_cleanup_set(false)
{
	m_timer = eip->NewTimer();
	m_timer->Register(this, &BtHub::Timeout);
}

BtHub::
~BtHub()
{
	Stop();
	delete m_timer;
}

bool BtHub::
AddService(BtService *svcp, ErrorInfo *error)
{
	ListItem unstarted;

	assert(!error || !error->IsSet());
	assert(svcp->m_links.Empty());
	assert(!svcp->m_hub);
	svcp->m_hub = this;

	if (IsStarted()) {
		unstarted.AppendItem(svcp->m_links);
		if (!svcp->Start(error)) {
			if (!svcp->m_links.Empty())
				svcp->m_links.Unlink();
			svcp->m_hub = 0;
			assert(!error || error->IsSet());
			return false;
		}

		svcp->m_links.UnlinkOnly();
	}

	m_services.AppendItem(svcp->m_links);
	return true;
}

void BtHub::
RemoveService(BtService *svcp)
{
	assert(svcp->m_hub == this);
	assert(!svcp->m_links.Empty());

	svcp->m_links.Unlink();
	if (IsStarted())
		svcp->Stop();
	svcp->m_hub = 0;
}

bool BtHub::
Start(ErrorInfo *error)
{
	ListItem unstarted;
	BtService *svcp;
	int hci_id;

	assert(!error || !error->IsSet());

	if (IsStarted())
		return true;

	/*
	 * Simple test for Bluetooth protocol support
	 */
	hci_id = hci_get_route(NULL);
	if (hci_id < 0) {
		if (errno == EAFNOSUPPORT) {
			m_ei->LogError(error,
				       LIBHFP_ERROR_SUBSYS_BT,
				       LIBHFP_ERROR_BT_NO_SUPPORT,
				       "Your kernel is not configured with "
				       "support for Bluetooth.");
			SetAutoRestart(false);
			return false;
		}

		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "Error retrieving HCI route: %s",
				   strerror(errno));
		return false;
	}

	m_sdp = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
	if (m_sdp == NULL) {
		/* Common enough to put in the debug pile */
		m_ei->LogDebug(error,
			       LIBHFP_ERROR_SUBSYS_BT,
			       LIBHFP_ERROR_BT_SYSCALL,
			       "Error connecting to local SDP");
		return false;
	}

	if (!m_sdp_handler.SdpCreateThread(error))
		goto failed;

	m_hci = new BtHci(this);
	if (!m_hci) {
		if (error)
			error->SetNoMem();
		goto failed;
	}
	if (!m_hci->HciInit(hci_id, error))
		goto failed;

	m_sdp_not = m_ei->NewSocket(sdp_get_socket(m_sdp), false);
	if (m_sdp_not == 0) {
		if (error)
			error->SetNoMem();
		goto failed;
	}
	m_sdp_not->Register(this, &BtHub::SdpConnectionLost);

	/* Start registered services */
	if (!m_services.Empty())
		unstarted.AppendItemsFrom(m_services);

	while (!unstarted.Empty()) {
		svcp = GetContainer(unstarted.next, BtService, m_links);
		svcp->m_links.UnlinkOnly();
		m_services.AppendItem(svcp->m_links);

		if (!svcp->Start(error)) {

			/* Ugh */
			ListItem started;
			if (!svcp->m_links.Empty()) {
				svcp->m_links.UnlinkOnly();
				unstarted.AppendItem(svcp->m_links);
			}
			if (!m_services.Empty())
				started.AppendItemsFrom(m_services);
			while (!started.Empty()) {
				svcp = GetContainer(started.next,
						    BtService,
						    m_links);
				svcp->m_links.UnlinkOnly();
				m_services.AppendItem(svcp->m_links);
				svcp->Stop();
			}

			if (!unstarted.Empty())
				m_services.AppendItemsFrom(unstarted);

			goto failed;
		}
	}

	return true;

failed:
	if (m_sdp_not) {
		delete m_sdp_not;
		m_sdp_not = 0;
	}
	m_sdp_handler.SdpShutdown();
	if (m_hci) {
		m_hci->HciShutdown();
		m_hci->Put();
		m_hci = 0;
	}
	sdp_close(m_sdp);
	m_sdp = NULL;
	assert(!error || error->IsSet());
	return false;
}


void BtHub::
__Stop(void)
{
	ListItem started;
	BtService *svcp;

	if (m_sdp) {
		assert(m_sdp_not != 0);
		delete m_sdp_not;
		m_sdp_not = 0;
		sdp_close(m_sdp);
		m_sdp = NULL;
	}

	if (!m_services.Empty())
		started.AppendItemsFrom(m_services);

	while (!started.Empty()) {
		svcp = GetContainer(started.next, BtService, m_links);
		svcp->m_links.UnlinkOnly();
		m_services.AppendItem(svcp->m_links);
		svcp->Stop();
	}

	m_sdp_handler.SdpShutdown();

	if (m_hci) {
		m_hci->HciShutdown();
		m_hci->Put();
		m_hci = 0;
	}

	assert(m_inquiry_task == 0);
}

void BtHub::
Stop(void)
{
	__Stop();

	if (m_autorestart) {
		if (m_autorestart_set) {
			m_autorestart_set = false;
			m_timer->Cancel();
		}
		m_autorestart = false;
	}
}

void BtHub::
SetAutoRestart(bool autorestart)
{
	if (m_autorestart == autorestart)
		return;

	m_autorestart = autorestart;
	if (autorestart && !m_cleanup_set && !IsStarted()) {
		/*
		 * Initial autorestart attempts are zero-wait
		 */
		m_autorestart_set = true;
		m_timer->Set(0);
	}
}

void BtHub::
SdpConnectionLost(SocketNotifier *notp, int fh)
{
	ErrorInfo error;

	assert(fh == sdp_get_socket(m_sdp));
	assert(notp == m_sdp_not);

	/* Lost our SDP connection?  Bad business */
	m_ei->LogWarn(&error,
		      LIBHFP_ERROR_SUBSYS_BT,
		      LIBHFP_ERROR_BT_SHUTDOWN,
		      "Lost local SDP connection");
	InvoluntaryStop(&error);
}

void BtHub::
InvoluntaryStop(ErrorInfo *reason)
{
	ListItem *listp;
	bool was_started = IsStarted();

	__Stop();

	if (m_autorestart && !m_cleanup_set) {
		if (m_autorestart_set) {
			m_autorestart_set = false;
			m_timer->Cancel();
		}
		m_autorestart_set = true;
		m_timer->Set(m_autorestart_timeout);
	}
	ListForEach(listp, &m_devices) {
		BtDevice *devp = GetContainer(listp, BtDevice, m_index_links);
		devp->__DisconnectAll(reason);
		if (IsStarted())
			return;
	}
	if (was_started && cb_NotifySystemState.Registered())
		cb_NotifySystemState(reason);
}

void BtHub::
ClearInquiryFlags(void)
{
	BtDevice *devp;
	for (devp = GetFirstDevice(); devp; devp = GetNextDevice(devp)) {
		if (devp->m_inquiry_found) {
			devp->m_inquiry_found = false;
			devp->Put();
		}
	}
}

void BtHub::
HciInquiryResult(HciTask *taskp)
{
	printf("#####1- HciInquiryResult\n");
	BtDevice *devp;

	assert(taskp == m_inquiry_task);

	if (taskp->m_complete) {
	//if (0) {
        if (taskp->m_error.IsSet()) {
			m_ei->LogWarn("Inquiry aborted: %s",
				      taskp->m_error.Desc());
		}

		m_inquiry_task = 0;
		ClearInquiryFlags();

		cb_InquiryResult(0, taskp->m_error.IsSet()
				 ? &taskp->m_error : 0);
		delete taskp;
		return;
	}

	/* Look up the device, maybe create it */
    //ag++
    //char dest_addr[] = "84:8E:0C:24:73:29";
    //bdaddr_t bdaddr;
	//str2ba(dest_addr, &bdaddr);
    //devp = GetDevice(bdaddr, true);

	//devp = GetDevice(taskp->m_bdaddr, true);
	if (!devp)
		return;
	if (devp->m_inquiry_found) {
		devp->Put();
		return;
	}

	devp->m_inquiry_found = true;
	devp->m_inquiry_pscan = taskp->m_pscan;
	devp->m_inquiry_pscan_rep = taskp->m_pscan_rep;
	devp->m_inquiry_clkoff = taskp->m_clkoff;
	devp->m_inquiry_class = taskp->m_devclass;
	cb_InquiryResult(devp, 0);
	return;
}

bool BtHub::
StartInquiry(int timeout_ms, ErrorInfo *error)
{
	HciTask *taskp;

	if (!IsStarted()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	if (m_inquiry_task) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_ALREADY_STARTED,
				   "Inquiry already in progress");
		return false;
	}

	/*
	 * If the client is going to request a scan, they had
	 * better have registered a result callback!
	 */
	assert(cb_InquiryResult.Registered());

	taskp = new HciTask;
	if (taskp == 0) {
		if (error)
			error->SetNoMem();
		return false;
	}

	taskp->m_tasktype = HciTask::HT_INQUIRY;
	taskp->m_timeout_ms = timeout_ms;
	taskp->cb_Result.Register(this, &BtHub::HciInquiryResult);

	if (!m_hci->Queue(taskp)) {
		delete taskp;
		return false;
	}

	m_inquiry_task = taskp;
	return true;
}

void BtHub::
StopInquiry(void)
{
	HciTask *taskp;

	if (!IsStarted() || !m_inquiry_task)
		return;

	taskp = m_inquiry_task;
	m_inquiry_task = 0;

	m_hci->Cancel(taskp);
	delete taskp;
	ClearInquiryFlags();
}

bool BtHub::
SdpTaskSubmit(SdpTask *taskp, ErrorInfo *error)
{
	assert(taskp->cb_Result.Registered());

	if (!IsStarted()) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	return m_sdp_handler.SdpQueue(taskp, error);
}

void BtHub::
SdpTaskCancel(SdpTask *taskp)
{
	return m_sdp_handler.SdpCancel(taskp);
}

/*
 * FIXME: These sdp_record functions can block the caller forever
 * waiting on the local SDP daemon
 */
bool BtHub::
SdpRecordRegister(sdp_record_t *recp, ErrorInfo *error)
{
	if (!m_sdp) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	if (sdp_record_register(m_sdp, recp, 0) < 0) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SYSCALL,
				   "Register SDP record: %s", strerror(errno));
		return false;
	}

	return true;
}

void BtHub::
SdpRecordUnregister(sdp_record_t *recp)
{
	if (!m_sdp || sdp_record_unregister(m_sdp, recp) < 0)
		GetDi()->LogDebug("Failed to unregister SDP record");
}

BtDevice *BtHub::
FindClientDev(bdaddr_t const &bdaddr)
{
	ListItem *listp;
	BtDevice *devp;
	ListForEach(listp, &m_devices) {
		devp = GetContainer(listp, BtDevice, m_index_links);
		if (!bacmp(&bdaddr, &devp->m_bdaddr)) {
			devp->Get();
			return devp;
		}
	}
	return 0;
}

BtDevice *BtHub::
CreateClientDev(bdaddr_t const &bdaddr)
{
	BtDevice *devp;
	char buf[32];

	ba2str(&bdaddr, buf);
	m_ei->LogDebug("Creating record for BDADDR %s", buf);

	/* Call the factory */
	if (cb_BtDeviceFactory.Registered())
		devp = cb_BtDeviceFactory(bdaddr);
	else
		devp = DefaultDevFactory(bdaddr);

	if (devp != NULL) {
		devp->m_hub = this;
		m_devices.AppendItem(devp->m_index_links);
	}
	return devp;
}

BtDevice *BtHub::
DefaultDevFactory(bdaddr_t const &bdaddr)
{
	return new BtDevice(this, bdaddr);
}


void BtHub::
DeadObject(BtManaged *objp)
{
	assert(!objp->m_refs);
	assert(objp->m_del_links.Empty());
	m_dead_objs.AppendItem(objp->m_del_links);

	if (!m_cleanup_set) {
		/* Schedule immediate deletion */
		if (m_autorestart_set) {
			m_autorestart_set = false;
			m_timer->Cancel();
		}
		m_cleanup_set = true;
		m_timer->Set(0);
	}
}


BtDevice *BtHub::
GetDevice(bdaddr_t const &raddr, bool create)
{
	BtDevice *devp;
	devp = FindClientDev(raddr);
	if (!devp && create) {
		devp = CreateClientDev(raddr);
	}
	return devp;
}

BtDevice *BtHub::
GetDevice(const char *raddr, bool create)
{
	bdaddr_t bdaddr;
	str2ba(raddr, &bdaddr);
	return GetDevice(bdaddr, create);
}

BtDevice *BtHub::
GetFirstDevice(void)
{
	if (m_devices.Empty())
		return 0;
	return GetContainer(m_devices.next, BtDevice, m_index_links);
}

BtDevice *BtHub::
GetNextDevice(BtDevice *devp)
{
	if (devp->m_index_links.next == &m_devices)
		return 0;
	return GetContainer(devp->m_index_links.next, BtDevice, m_index_links);
}

void BtHub::
Timeout(TimerNotifier *notp)
{
	assert(notp == m_timer);

	m_autorestart_set = false;
	if (!IsStarted() && !m_cleanup_set && m_autorestart) {
		/* Do or do not.  There is no try. */
		Start();
		if (IsStarted() && cb_NotifySystemState.Registered())
			cb_NotifySystemState(0);
	}

	while (!m_dead_objs.Empty()) {
		BtManaged *objp;
		objp = GetContainer(m_dead_objs.next, BtManaged, m_del_links);
		assert(!objp->m_refs);
		objp->m_del_links.Unlink();
		if (objp->cb_NotifyDestroy.Registered())
			objp->cb_NotifyDestroy(objp);
		delete objp;
	}

	m_cleanup_set = false;

	if (!IsStarted() && m_autorestart) {
		m_autorestart_set = true;
		m_timer->Set(m_autorestart_timeout);
	}
}


BtDevice::
BtDevice(BtHub *hubp, bdaddr_t const &bdaddr)
	: BtManaged(hubp), m_bdaddr(bdaddr), m_inquiry_found(false),
	  m_name_resolved(false), m_name_task(0)
{
	/* Scribble something down for the name */
	ba2str(&bdaddr, m_dev_name);
}

BtDevice::
~BtDevice()
{
	BtHci *hcip;

	assert(!m_inquiry_found);
	GetDi()->LogDebug("Destroying record for %s", GetName());
	if (!m_index_links.Empty())
		m_index_links.Unlink();
	if (m_name_task) {
		hcip = GetHub()->GetHci();
		if (hcip)
			hcip->Cancel(m_name_task);
		delete m_name_task;
		m_name_task = 0;
	}
}

void BtDevice::
GetAddr(char (&namebuf)[32]) const
{
	ba2str(&m_bdaddr, namebuf);
}

bool BtDevice::
ResolveName(ErrorInfo *error)
{
	printf("#### 2-1 ResolveName\n");
	HciTask *taskp;
	BtHci *hcip;

	if (m_name_task)
		return true;

	hcip = GetHub()->GetHci();
	if (!hcip) {
		if (error)
			error->Set(LIBHFP_ERROR_SUBSYS_BT,
				   LIBHFP_ERROR_BT_SHUTDOWN,
				   "Bluetooth subsystem shut down");
		return false;
	}

	taskp = new HciTask;
	if (taskp == 0) {
		if (error)
			error->SetNoMem();
		return false;
	}

	taskp->m_tasktype = HciTask::HT_READ_NAME;
	bacpy(&taskp->m_bdaddr, &m_bdaddr);
	taskp->m_timeout_ms = 5000;
	if (m_inquiry_found) {
		taskp->m_pscan = m_inquiry_pscan;
		taskp->m_pscan_rep = m_inquiry_pscan_rep;
		taskp->m_clkoff = m_inquiry_clkoff;
	}
	taskp->cb_Result.Register(this, &BtDevice::NameResolutionResult);

	if (!hcip->Queue(taskp, error)) {
		delete taskp;
		return false;
	}

	m_name_resolved = false;
	m_name_task = taskp;
	return true;
}

void BtDevice::
NameResolutionResult(HciTask *taskp)
{
	assert(taskp == m_name_task);

	m_name_resolved = !taskp->m_error.IsSet();

	if (m_name_resolved) {
		strncpy(m_dev_name, taskp->m_name, sizeof(m_dev_name));
		m_dev_name[sizeof(m_dev_name) - 1] = '\0';
	}

	m_name_task = 0;

	if (cb_NotifyNameResolved.Registered())
		cb_NotifyNameResolved(this,
				      m_name_resolved ? m_dev_name : NULL,
				      m_name_resolved ? 0 : &taskp->m_error);

	delete taskp;
}

void BtDevice::
AddSession(BtSession *sessp)
{
	assert(sessp->m_dev_links.Empty());
	m_sessions.AppendItem(sessp->m_dev_links);
	Get();
}

void BtDevice::
RemoveSession(BtSession *sessp)
{
	assert(!sessp->m_dev_links.Empty());
	assert(sessp->m_dev == this);
	sessp->m_dev_links.Unlink();
	Put();
}

BtSession *BtDevice::
FindSession(BtService const *svcp) const
{
	ListItem *listp;
	ListForEach(listp, &m_sessions) {
		BtSession *sessp;
		sessp = GetContainer(listp, BtSession, m_dev_links);
		if (sessp->m_svc == svcp) {
			sessp->Get();
			return sessp;
		}
	}
	return 0;
}

void BtDevice::
__DisconnectAll(ErrorInfo *reason)
{
	ListItem sesslist;
	BtSession *sessp;

	if (!m_sessions.Empty())
		sesslist.AppendItemsFrom(m_sessions);

	while (!sesslist.Empty()) {
		sessp = GetContainer(sesslist.next, BtSession, m_dev_links);
		assert(sessp->m_dev == this);
		sessp->m_dev_links.UnlinkOnly();
		m_sessions.AppendItem(sessp->m_dev_links);
		sessp->__Disconnect(reason);
	}
}

BtService::
BtService(void)
	: m_hub(0)
{
}

BtService::
~BtService()
{
	/* We had best have been unregistered */
	assert(m_links.Empty());
}

void BtService::
AddSession(BtSession *sessp)
{
	assert(sessp->m_svc_links.Empty());
	m_sessions.AppendItem(sessp->m_svc_links);
}

void BtService::
RemoveSession(BtSession *sessp)
{
	assert(sessp->m_svc == this);
	assert(!sessp->m_svc_links.Empty());
	sessp->m_svc_links.Unlink();
}

BtSession *BtService::
FindSession(BtDevice const *devp) const
{
	return devp->FindSession(this);
}

BtSession *BtService::
GetFirstSession(void) const
{
	if (m_sessions.Empty())
		return 0;

	return GetContainer(m_sessions.next, BtSession, m_svc_links);
}

BtSession *BtService::
GetNextSession(BtSession *sessp) const
{
	if (sessp->m_svc_links.next == &m_sessions)
		return 0;

	return GetContainer(sessp->m_svc_links.next, BtSession, m_svc_links);
}


BtSession::
BtSession(BtService *svcp, BtDevice *devp)
	: BtManaged(devp->GetHub()), m_dev(devp), m_svc(svcp) {
	devp->AddSession(this);
	svcp->AddSession(this);
}

BtSession::
~BtSession()
{
	m_dev->RemoveSession(this);
	m_svc->RemoveSession(this);
}


} /* namespace libhfp */
