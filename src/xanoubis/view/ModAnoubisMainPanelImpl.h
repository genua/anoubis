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

#ifndef __ModAnoubisMainPanelImpl__
#define __ModAnoubisMainPanelImpl__

#include "ModAnoubisPanelsBase.h"
#include "Notification.h"

class ModAnoubisMainPanelImpl : public ModAnoubisMainPanelBase
{
	private:
		enum notifyListTypes	 list_;
		Notification		*currentNotify_;
		bool			systemNotifyEnabled_;
		int			systemNotifyTimeout_;

		void answer(bool);
		void displayAlert(void);
		void displayEscalation(void);
		void displayLog(void);

	protected:
		void OnTypeChoosen(wxCommandEvent& event);
		void OnFirstBtnClick(wxCommandEvent& event);
		void OnPreviousBtnClick(wxCommandEvent& event);
		void OnNextBtnClick(wxCommandEvent& event);
		void OnLastBtnClick(wxCommandEvent& event);
		void OnAllowBtnClick(wxCommandEvent& event);
		void OnDenyBtnClick(wxCommandEvent& event);
		void OnToggleNotification(wxCommandEvent& event);
		void OnNotificationTimeout(wxSpinEvent& event);

	public:
		ModAnoubisMainPanelImpl(wxWindow* parent, wxWindowID id);
		void update(void);
};

#endif /* __ModAnoubisMainPanelImpl__ */
