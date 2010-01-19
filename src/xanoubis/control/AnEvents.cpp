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

#include "AnEvents.h"

#include "Singleton.cpp"

AnEvents::~AnEvents(void)
{
	/*
	 * As it's not possible to flush the table of events by
	 * ourself (because we can't iterate over the table and have
	 * no function to do a Disconnect()) -- thus there's nothing
	 * to do here.
	 */
}

AnEvents *
AnEvents::getInstance(void) {
	return Singleton<AnEvents>::instance();
}

AnEvents::AnEvents(void) : Singleton<AnEvents>()
{
	/* Nothing to do here. */
}

#ifdef ANEVENTS_IDENT_BCAST_ENABLE
DEFINE_LOCAL_EVENT_TYPE(ANEVENTS_IDENT_BCAST_EVENTNAME)
#endif /* ANEVENTS_IDENT_BCAST_ENABLE */

DEFINE_LOCAL_EVENT_TYPE(anEVT_WIZARD_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_LOGVIEWER_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_RULEEDITOR_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ESCALATIONS_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ANOUBISOPTIONS_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SHOW_RULE)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ESCALATIONNOTIFY_OPTIONS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ALERTNOTIFY_OPTIONS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_COM_CONNECTION)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SEND_AUTO_CHECK)
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALERTS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_ESCALATIONS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALF_ESCALATIONS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_SFS_ESCALATIONS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_SB_ESCALATIONS)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ADD_NOTIFYANSWER)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ANSWER_ESCALATION)
DEFINE_LOCAL_EVENT_TYPE(anEVT_LOAD_RULESET)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SEND_RULESET)
DEFINE_LOCAL_EVENT_TYPE(anEVT_LOAD_KEY)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ESCALATION_RULE_ERROR)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SFSOPERATION_FINISHED)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SFSDIR_CHANGED)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SFSENTRY_CHANGED)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SFSENTRY_ERROR)
DEFINE_LOCAL_EVENT_TYPE(anEVT_POLICY_CHANGE)
DEFINE_LOCAL_EVENT_TYPE(anEVT_BACKUP_POLICY)
DEFINE_LOCAL_EVENT_TYPE(anEVT_UPDATE_PERSPECTIVE)
DEFINE_LOCAL_EVENT_TYPE(anEVT_SFSBROWSER_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_ANOUBISOPTIONS_UPDATE)
DEFINE_LOCAL_EVENT_TYPE(anEVT_UPGRADENOTIFY);
DEFINE_LOCAL_EVENT_TYPE(anEVT_ROW_SIZECHANGE);
DEFINE_LOCAL_EVENT_TYPE(anEVT_ROW_UPDATE);
