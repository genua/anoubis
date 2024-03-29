/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifndef _RULEWIZARDALFDLGADDSERVICE_H_
#define _RULEWIZARDALFDLGADDSERVICE_H_

#include "RuleWizardPanelsBase.h"

class Service;
class ServiceList;

/**
 * RuleWizard add service dialog for alf.
 * This dialog opens /etc/services and presents a list of it's entries.
 *
 * The user may choose one or more lines from the list or use the search
 * field to find list entries. With the button 'add selected service'
 * the selection is stored in a list and wxID_OK is returned.
 *
 * Or the user may use the custom section to specify the protocol and
 * a portnumber, -list or -range. With the button 'add custom service'
 * the input is stored in al list and wxID_OK is returned.
 *
 * With the cancel button the dialog is closed and wxID_CANCEL is returned.
 *
 * After the dialog is closed the input can be fetched by using getSelection().
 */
class RuleWizardAlfDlgAddService : public RuleWizardAlfDlgAddServiceBase
{
	public:
		/**
		 * Constructor of this dialog.
		 * @param[in] 1st The parent window (aka the caller)
		 */
		RuleWizardAlfDlgAddService(wxWindow *);

		/**
		 * Destructor of this dialog
		 */
		~RuleWizardAlfDlgAddService(void);

		/**
		 * Get the selected services.
		 *
		 * @param None.
		 * @return The list of services been selected.
		 */
		ServiceList *getSelection(void) const;

	private:
		/**
		 * The internal model of the service-list
		 */
		ServiceList *serviceList_;

		/**
		 * Internal list contains all selected services.
		 */
		ServiceList *selection_;

		/**
		 * Handle events from addButton.
		 * This will clear the selection list and refill it with the
		 * selected service(s) from the service list. In addition this
		 * will close the dialog with wxID_OK.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAddButton(wxCommandEvent &);

		/**
		 * Handle events from addCustomButton.
		 * This will clear the selection list and refill it with the
		 * custom input from the user. In addition this will close
		 * the dialog with wxID_OK.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onCustomAddButton(wxCommandEvent &);

		/**
		 * Handle events from cancelButton.
		 * This will clear the selection list and close the dialog
		 * with wxID_CANCEL.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onCancelButton(wxCommandEvent &);

		/**
		 * Handle select events from serviceListCtrl.
		 * This will enable the addButton if something is selected.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onServiceListSelect(wxListEvent &);

		/**
		 * Handle deselect events from serviceListCtrl.
		 * This will disable the addButton if the last seleciton
		 * is removed.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onServiceListDeselect(wxListEvent &);

		/**
		 * Handle events from searhTextCtrl.
		 * This will perform the search of given text within the list
		 * of shown services. If the search text is a number, the
		 * portnumbers are compared. In any other case the service name
		 * is compared. On match the corresponing line is selected and
		 * the search text is cleared.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onSearchTextEnter(wxCommandEvent &);
};

#endif	/* _RULEWIZARDALFDLGADDSERVICE_H_ */
