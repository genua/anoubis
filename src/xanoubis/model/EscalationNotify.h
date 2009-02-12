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

#ifndef _ESCALATIONNOTIFY_H_
#define _ESCALATIONNOTIFY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#endif

#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#endif


#include <anoubis_msg.h>
#include <wx/string.h>
#include <typeinfo>

#include "Notification.h"
#include "NotifyAnswer.h"

#define IS_ESCALATIONOBJ(obj) \
	(typeid(*obj) == typeid(class EscalationNotify))

class EscalationNotify : public Notification {
	private:
		NotifyAnswer	*answer_;
		bool		 allowEdit_;
		wxString	 rulePath_;

		void assembleLogMessage(void);

	public:
		EscalationNotify(struct anoubis_msg *);
		~EscalationNotify(void);

		bool		 isAnswered(void);
		void		 answer(NotifyAnswer *);
		void		 setAnswer(NotifyAnswer *);
		NotifyAnswer	*getAnswer(void);
		anoubis_token_t	 getToken(void);
		anoubis_cookie_t getTaskCookie(void);
		wxString	 getBinaryName(void);
		bool		 getChecksum(unsigned char *);
		bool		 getCtxChecksum(unsigned char *);
		int		 getProtocolNo(void);
		int		 getDirectionNo(void);
		bool		 allowEdit(void);
		void		 setAllowEdit(bool value = true);
		bool		 allowOptions(void);
		wxString	 rulePath(void);
		void		 setRulePath(wxString);
};

#endif	/* _ESCALATIONNOTIFY_H_ */
