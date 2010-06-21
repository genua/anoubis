/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _MODPLAYGROUNDMAINPANELIMPL_H_
#define _MODPLAYGROUNDMAINPANELIMPL_H_

#include "AnEvents.h"

#include "ModPlaygroundPanelsBase.h"

class ModPlaygroundMainPanelImpl : public ModPlaygroundMainPanelBase
{
	public:
		/**
		 * Constructor of ModPlaygroundMainPanelImpl.
		 * @param[in] 1st The parent window and ID.
		 */
		ModPlaygroundMainPanelImpl(wxWindow*, wxWindowID);

		/**
		 * Destructor of ModPlaygroundMainPanelImpl.
		 * @param None.
		 */
		~ModPlaygroundMainPanelImpl(void);

	private:
		/**
		 * Handle <Return> events of AppComboBox.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAppStartEnter(wxCommandEvent &);

		/**
		 * Handle onClick events to 'Start Playground' button.
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		virtual void onAppStart(wxCommandEvent &);

		/**
		 * Start application.
		 * This method will take the input from combo box and tries
		 * to start it. On succes the history (combo box) is updated.
		 * On error a dialog is shown.
		 * @param None.
		 * @return Nothing.
		 */
		void startApplication(void);

		/**
		 * Convert string to argv array.
		 * This method creates tokens by splitting the given string
		 * at whitespace characters. These tokens are stored in an array
		 * with number of tokens + 1. The last element is NULL for
		 * termination.
		 * @note It is the responibility of the caller to free(3) the
		 *	allocated memory of argv.
		 * @param[in] 1st The string to tokenize.
		 * @return A pointer to argv or NULL in case of error.
		 */
		char **convertStringToArgV(const wxString &) const;
};


#endif	/* _MODPLAYGROUNDMAINPANELIMPL_H_ */
