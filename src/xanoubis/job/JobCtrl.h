/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _JOBCTRL_H_
#define _JOBCTRL_H_

#include <wx/event.h>

#include "ComRegistrationTask.h"
#include "ComVersionTask.h"
#include "JobThread.h"
#include "Singleton.h"
#include "SynchronizedQueue.h"
#include "Task.h"

WX_DECLARE_LIST(JobThread, JobThreadList);

class ComThread;
class CsumCalcThread;
class Notification;

/**
 * The job controller.
 *
 * The JobCtrl-class is responsible for handling asynchronous tasks. A task can
 * be anything which might break the usability of the GUI. A task is executed
 * in a background thread.
 *
 * A task implements the Task-interface. The pure virtual function Task::exec()
 * contains the logic of the task.
 *
 * The background-thread(s) are started by calling start() and stopped again by
 * calling stop(). If the job-constroller is not started, no scheduled task is
 * executed.
 *
 * You also might what to call connected()/disconnect()! This establishs a
 * connection to anoubisd. This connection is used for any ComTask-derivated
 * tasks. getSocketPath()/setSocketPath() configures the unix-domain-socket
 * used for communication.
 *
 * The task-execution is scheduled by invoking JobCtrl::addTask(). When
 * task-execution has finished, a TaskEvent is generated which contains the
 * calculation-result.
 *
 * This class is designed as a singleton, thus you can access from wherever you
 * want.
 */
class JobCtrl : public wxEvtHandler, public Singleton<JobCtrl>
{
	public:
		/**
		 * State of connection to anoubisd.
		 */
		enum ConnectionState {
			CONNECTED = 0,    /*!< You are connected. */
			DISCONNECTED,     /*!< You are disconnected. */
			ERR_RW,           /*!< An error occured while
					       reading/writing the socket. */
			ERR_CONNECT,      /*!< An error occured while opening
					       the socket connection. */
			ERR_REG,          /*!< An error occured while
					       registering at the daemon. */
			ERR_VERSION_PROT, /*!< The connection could not be
					       established because of an
					       Anoubis protocol version
					       mismatch. */
			ERR_VERSION_APN,  /*!< The connection could not be
					       established because of an APN
					       parser version mismatch. */
			ERR_NO_KEY,       /*!< The daemon requests
					       client-authentication, but the
					       private key could not be
					       loaded. */
			ERR_KEYID,        /*!< The daemon requests
					       client-authentication but the
					       daemon asks for a certificate
					       which is not configured at the
					       client-side. */
			ERR_INV_KEY,	  /*!< Failure because of a invalid
					       key */
			ERR_INV_CERT,	  /*!< Failure because of a invalid
					       certifciate */
			ERR_AUTH_SYS_FAIL /*!< Internal System Error while
					       authentication */

		};

		~JobCtrl(void);

		/**
		 * Returns the singleton instance of the class.
		 */
		static JobCtrl *getInstance(void);

		/**
		 * Returns the path to socket used for communication with
		 * anoubisd.
		 *
		 * The GUI establishes the connection to anoubisd over a domain
		 * socket.
		 *
		 * @return Path to domain-socket
		 */
		wxString getSocketPath(void) const;

		/**
		 * Updates the path to the socket used for communication with
		 * anoubisd.
		 *
		 * If you do not specify your own path, a feasible
		 * default-value is used.
		 *
		 * @param path The new path to domain-socket
		 * @note The method needs to be called before JobCtrl::start()
		 *       is invoked!
		 */
		void setSocketPath(const wxString &);

		/**
		 * Starts the JobCtrl.
		 *
		 * This method creates the background-threads. As a result you
		 * are able to execute any scheduled tasks.
		 *
		 * @note You need to call the method to be able to execute your
		 *       scheduled tasks!
		 */
		bool start(void);

		/**
		 * Stops the JobCtrl.
		 *
		 * The background-threads are stopped. If this method is
		 * called, non of the scheduled tasks are processed.
		 */
		void stop(void);

		/**
		 * Establishes a connection to anoubisd.
		 *
		 * You need to call this method, if you want to execute any
		 * ComTask-derivated tasks.
		 *
		 * The procedure for establishing the connection is started in
		 * the background. You need to wait for a wxCommandEvent of
		 * type anEVT_COM_CONNECTION to test the connection.
		 *
		 * @return true is returned, if the background-thread was
		 *         started.
		 */
		bool connect(void);

		/**
		 * Disconnects again from anoubisd.
		 *
		 * This operation is started in the background. You need to
		 * wait for a wxCommandEvent of type anEVT_COM_CONNECTION to
		 * test the state of the operation.
		 */
		void disconnect(void);

		/**
		 * Tests whether a connection to anoubisd is already
		 * established.
		 *
		 * @return true if you have a connection, false otherwise.
		 */
		bool isConnected(void) const;

		/**
		 * Asks the send the answer of the specified daemon back to
		 * anoubisd.
		 *
		 * @param notify The requested notification
		 */
		void answerNotification(Notification *);

		/**
		 * Registers and schedules a task for execution.
		 *
		 * When the task is really executed highly depends on how many
		 * tasks are in the execution-queue.
		 *
		 * The JobCtrl does not take care about the memory probably
		 * allocated for the task! The user of JobCtrl has the
		 * responsibility to destroy the task again. A task can be
		 * safely destroyed, when the corresponding TaskEvent was
		 * fired.
		 *
		 * @param task The task to be scheduled
		 */
		void addTask(Task *);

		/**
		 * Returns the version of the Anoubis protocol used by the
		 * daemon.
		 * @return Remove Anoubis protocol version
		 */
		int getDaemonProtocolVersion(void) const;

		/**
		 * Returns the version of the APN parser used by the daemon.
		 * @return Version of remote APN parser
		 */
		int getDaemonApnVersion(void) const;

	protected:
		JobCtrl(void);

		/**
		 * Removes and returns a task from an execution-queue.
		 *
		 * @param type Type of task to be removed
		 * @return The requested task.
		 */
		Task *popTask(Task::Type);

		/**
		 * Invoked if a ComRegistrationTask is completed.
		 *
		 * The ComRegistrationTask is part of the (dis-)connection
		 * operation and scheduled during connect() resp. disconnect().
		 */
		void onDaemonRegistration(TaskEvent &);

		/**
		 * Invoked if a ComVersionTask is completed.
		 *
		 * The ComVersionTask is part of the connection-procedure
		 * and fetches a list of versions. There are compared with
		 * available client-versions.
		 */
		void onDaemonVersion(TaskEvent &);

		/**
		 * Will cleanup after a Connection Error.
		 */
		void onConnectionStateChange(wxCommandEvent &);

	private:
		wxString socketPath_;
		SynchronizedQueue<Task> *csumCalcTaskQueue_;
		SynchronizedQueue<Task> *comTaskQueue_;
		JobThreadList threadList_;
		ComRegistrationTask *regTask_;
		ComVersionTask *versionTask_;
		int protocolVersion_;
		int apnVersion_;

		ComThread	*findComThread(void) const;
		CsumCalcThread	*findCsumCalcThread(void) const;
		void		 stopThread(JobThread *);
		void		 sendComEvent(JobCtrl::ConnectionState);

	friend class ComThread;
	friend class JobThread;
	friend class Singleton<JobCtrl>;
};

#endif	/* _JOBCTRL_H_ */
