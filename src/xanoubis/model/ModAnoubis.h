/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef __MODANOUBIS_H__
#define __MODANOUBIS_H__

#include "Module.h"
#include "NotifyList.h"
#include "NotifyAnswer.h"

enum ModAnoubisId {
	MODANOUBIS_ID_BASE = 13000,
	MODULE_ID_ENTRY(ANOUBIS,MAINPANEL),
	MODULE_ID_ENTRY(ANOUBIS,OVERVIEWPANEL),
	MODULE_ID_ENTRY(ANOUBIS,TOOLBAR)
};

class ModAnoubis : public Module
{
	private:
		size_t		notAnsweredListIdx_;
		size_t		msgListIdx_;
		size_t		answeredListIdx_;
		size_t		allListIdx_;
		NotifyList	notAnsweredList_;
		NotifyList	msgList_;
		NotifyList	answeredList_;
		NotifyList	allList_;

	public:
		ModAnoubis(wxWindow *);
		~ModAnoubis(void);

		int	getBaseId(void);
		int	getToolbarId(void);
		void	update(void);

		void	insertNotification(NotifyListEntry *);
		void	answerNotification(NotifyListEntry *, NotifyAnswer *);
		size_t	getElementNo(enum notifyListTypes);
		size_t	getListSize(enum notifyListTypes);

		NotifyListEntry *getFirst(enum notifyListTypes);
		NotifyListEntry *getPrevious(enum notifyListTypes);
		NotifyListEntry *getNext(enum notifyListTypes);
		NotifyListEntry *getLast(enum notifyListTypes);
};

#endif /* __MODANOUBIS_H__ */
