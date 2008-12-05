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
#include "Task.h"

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
 *
 * If an error occured, an wxCommandEvent of type anEVT_SFSENTRY_ERROR is
 * created. The you can use SfsCtrl::getErrors() to receive the list of errors.
 * Note, that the list is cleared before the next command is executed!
 */
class SfsCtrl : public wxEvtHandler
{
	public:
		/**
		 * Result-codes of a command.
		 */
		enum CommandResult
		{
			RESULT_EXECUTE,	/*!< The command is scheduled and will
					     be executed. */
			RESULT_NOTCONNECTED,	/*!< You are not connected with
						     anoubisd. The command is
						     not scheduled. */
			RESULT_INVALIDARG,	/*!< You supplied an invalid
						     argument. The command is
						     not scheduled. */
			RESULT_BUSY	/*!< The controller is still busy with
					     another command. The new command
					     is not scheduled. */
		};

		SfsCtrl(void);

		/**
		 * Returns the root-directory.
		 *
		 * Files below the directory are part of the model.
		 *
		 * @return The root-directory
		 * @see SfsDirectory::getPath()
		 */
		wxString getPath() const;

		/**
		 * Updates the root-directory of the model.
		 *
		 * If the directory changes, the model is refreshed and a
		 * wxCommandEvent of type anEVT_SFSDIR_CHANGED is fired.
		 *
		 * @param path The new root-directory.
		 * @see SfsDirectory::setPath()
		 */
		void setPath(const wxString &);

		/**
		 * Returns the filename-filter.
		 *
		 * Only files, which filename contains the filter are insered
		 * into the model.
		 *
		 * @return The filename-filter
		 * @see SfsDirectory::getFilter()
		 */
		wxString getFilter() const;

		/**
		 * Update sthe filename-filter.
		 *
		 * The the filter-string changes, the model is refreshed and a
		 * wxCommandEvent of type anEVT_SFSDIR_CHANGED is fired.
		 *
		 * @param filter The new filename-filter
		 * @see SfsDirectory::setFilter()
		 */
		void setFilter(const wxString &);

		/**
		 * Tests, weather the filename-filter is inversed.
		 *
		 * If the filename-filter is inversed, only files, which
		 * filename not contains the filter are inserted into the
		 * model.
		 *
		 * @return true, if the inverse-flag is set, false otherwise.
		 * @see SfsDirectory::isFilterInversed()
		 */
		bool isFilterInversed() const;

		/**
		 * Updates the filter-inverse-flag.
		 *
		 * The the value of the flag changes, the model is refreshed
		 * and a wxCommandEvent of type anEVT_SFSDIR_CHANGED is fired.
		 *
		 * @param inversed The new value of the filter-inverse-flag.
		 * @see SfsDirectory::setFilterInversed()
		 */
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
		 * @param idx Index of SfsEntry to validate. If the index is
		 *           out of range, SfsCtrl::RESULT_INVALIDARG is
		 *           returned.
		 * @return The result of the command.
		 */
		CommandResult validate(unsigned int);

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
		 * @return The result of the command.
		 */
		CommandResult validateAll(void);

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
		 * @param idx Index of SfsEntry to register. If the index is
		 *            out of range, SfsCtrl::RESULT_INVALIDARG is
		 *            returned.
		 * @return The result of the command.
		 */
		CommandResult registerChecksum(unsigned int);

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
		 * @param idx Index of SfsEntry to unregister. If the index is
		 *            out of range, SfsCtrl::RESULT_INVALIDARG is
		 *            returned.
		 * @return The result of the command.
		 */
		CommandResult unregisterChecksum(unsigned int);

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
		 * @param idx Index of SfsEntry to update. If the index is out
		 *            of range, SfsCtrl::RESULT_INVALIDARG is
		 *            returned.
		 * @return The result of the command.
		 */
		CommandResult updateChecksum(unsigned int);

		/**
		 * Returns the instance of the SfsDirectory.
		 *
		 * This is the entry-point of the model.
		 *
		 * @return The entry-point of the model.
		 */
		SfsDirectory &getSfsDirectory();

		/**
		 * Returns a list of errors, which occured during the execution
		 * of the last command.
		 *
		 * You will receive a wxCommandEvent of type
		 * anEVT_SFSENTRY_ERROR if an error occured. Then you can
		 * receive the error-list by invoking this method.
		 *
		 * Please not, that the error-list is resetted, if you start
		 * another command!
		 *
		 * @return Errors collected during execution of the last
		 *         command.
		 */
		const wxArrayString &getErrors(void) const;

	protected:
		void OnRegistration(TaskEvent &);
		void OnSfsListArrived(TaskEvent &);
		void OnCsumCalc(TaskEvent &);
		void OnCsumGet(TaskEvent &);
		void OnCsumAdd(TaskEvent &);
		void OnCsumDel(TaskEvent &);

	private:
		SfsDirectory	sfsDir_;
		TaskList	taskList_;
		wxArrayString	errorList_;
		bool		comEnabled_;

		void enableCommunication(void);
		void disableCommunication(void);

		void pushTask(Task *);
		bool popTask(Task *);

		void sendDirChangedEvent(void);
		void sendEntryChangedEvent(int);
		void sendErrorEvent(void);

		/**
		 * Internal utility class, which helps to pop tasks from
		 * taskList_ and deleting them.
		 *
		 * The d'tor
		 * - invokes SfsCtrl::popTask() on the specified task
		 * - destroys the task, if it is stored in taskList_.
		 */
		class PopTaskHelper
		{
			public:
				PopTaskHelper(SfsCtrl *, Task *);
				~PopTaskHelper();

			private:
				SfsCtrl *sfsCtrl_;
				Task *task_;
		};
};

#endif	/* _SFSCTRL_H_ */
