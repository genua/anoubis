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

#ifndef _SFSCTRL_H_
#define _SFSCTRL_H_

#include <wx/event.h>

#include "SfsDirectory.h"
#include "ComSfsListTask.h"

/**
 * The SfsController acts as a bridge between an SfsDirectory and the view.
 * Operations on the directory are started here and changes on the directory
 * are reported back to the view.
 *
 * If the content of the assigned SfsDirectory has changed, an wxCommandEvent
 * with event-type anEVT_SFSDIR_CHANGED is created.
 *
 * If at least one of the attributes of an SfsEntry has changed, an
 * wxCommandEvent with event-type anEVT_SFSENTRY_CHANGED is created. The
 * int-attribute contains the index of the SfsEntry inside the SfsDirectory.
 */
class SfsCtrl : public wxEvtHandler
{
	public:
		SfsCtrl(void);

		/**
		 * @see SfsDirectory::getPath()
		 */
		wxString getPath() const;
		void setPath(const wxString &);

		/**
		 * @see SfsDirectory::getFilter()
		 */
		wxString getFilter() const;
		void setFilter(const wxString &);

		/**
		 * @see SfsDirectory::isFilterInversed()
		 */
		bool isFilterInversed() const;
		void setFilterInversed(bool);

		/**
		 * Validates the SfsEntry at the specified index.
		 *
		 * The checksumAttr- & signatureAttr-attributes are updated. If
		 * an attribute has changed, an wxCommandEvent of type
		 * anEVT_SFSENTRY_CHANGED is fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx Index of SfsEntry to validate
		 * @return false is returned, if you have no connection to
		 *         anoubis.
		 */
		bool validate(unsigned int);

		/**
		 * Validates all SfsEntry-instances under the directory.
		 *
		 * The checksumAttr- & signatureAttr-attributes are updated. If
		 * an attribute has changed, an wxCommandEvent of type
		 * anEVT_SFSENTRY_CHANGED is fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-events.
		 *
		 * @return false is returned, if you have no connection to
		 *         anoubis.
		 */
		bool validateAll(void);

		/**
		 * Registers the checksum of the SfsEntry at the given index
		 * at anoubisd.
		 *
		 * The checksumAttr- & signatureAttr-attributes are updated. If
		 * an attribute has changed, an wxCommandEvent of type
		 * anEVT_SFSENTRY_CHANGED is fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx Index of SfsEntry to register
		 * @return false is returned, if you have no connection to
		 *         anoubis.
		 */
		bool registerChecksum(unsigned int);

		/**
		 * Removes the checksum of the SfsEntry at the given index from
		 * anoubisd.
		 *
		 * The checksumAttr- & signatureAttr-attributes are updated. If
		 * an attribute has changed, an wxCommandEvent of type
		 * anEVT_SFSENTRY_CHANGED is fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx Index of SfsEntry to unregister
		 * @return false is returned, if you have no connection to
		 *         anoubis.
		 */
		bool unregisterChecksum(unsigned int);

		/**
		 * Updates the checksum of the SfsEntry at the given index at
		 * anoubisd.
		 *
		 * The checksumAttr- & signatureAttr-attributes are updated. If
		 * an attribute has changed, an wxCommandEvent of type
		 * anEVT_SFSENTRY_CHANGED is fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx Index of SfsEntry to update
		 * @return false is returned, if you have no connection to
		 *         anoubis.
		 */
		bool updateChecksum(unsigned int);

		/**
		 * Returns the instance of the SfsDirectory.
		 */
		SfsDirectory &getSfsDirectory();

	protected:
		void OnRegistration(TaskEvent &);
		void OnSfsListArrived(TaskEvent &);
		void OnCsumCalc(TaskEvent &);
		void OnCsumGet(TaskEvent &);
		void OnCsumAdd(TaskEvent &);
		void OnCsumDel(TaskEvent &);

	private:
		SfsDirectory	sfsDir_;
		ComSfsListTask	sfsListTask_;
		bool		comEnabled_;

		void enableCommunication(void);
		void disableCommunication(void);

		void sendDirChangedEvent(void);
		void sendEntryChangedEvent(int);
};

#endif	/* _SFSCTRL_H_ */
