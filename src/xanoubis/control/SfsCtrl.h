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
 * Array is a list of indexes.
 */
WX_DEFINE_ARRAY_INT(unsigned int, IndexArray);

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
			RESULT_NEEDPASS,	/*!< The controller tries to
						     get the private key, but
						     it is not loaded. */
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
		 * Tests whether signature-support is enabled.
		 *
		 * If it is enabled, the checksums are automatically signed.
		 * The signed checksum is sent to anoubisd instead of the plain
		 * checksum.
		 *
		 * Note, that the support can be disabled without
		 * user-interaction! When the private key or certificate gets
		 * unavailable, the support is disabled automatically because
		 * of a dependency-problem (you need a private key and
		 * certificate). But loading both of them again, will enable
		 * the support.
		 *
		 * @return true is returned, if the signature-support is
		 *         enabled, false otherwise.
		 * @see KeyCtrl::canUseLocalKeys()
		 */
		bool isSignatureEnabled(void) const;

		/**
		 * Enables the signature-support.
		 *
		 * Before the support is enabled, it checks the dependencies.
		 * You need a private key and a certificate. The support can
		 * only be enabled, if you have both of them. Disabling the
		 * feature does not check for the dependencies.
		 *
		 * @param enable Set to true, is you want to enable signatures
		 * @return true is returned, if the signature-support was
		 *         successfully enabled or diabled. On error false is
		 *         returned. In case of you want to enable
		 *         signature-support, either the private key or the
		 *         certificate is not available.
		 * @see KeyCtrl::canUseLocalKeys()
		 */
		bool setSignatureEnabled(bool);

		/**
		 * Validates the SfsEntries at the specified indexes.
		 *
		 * The SfsEntry-instances might be updated. For each changed
		 * entry a wxCommandEvent of type anEVT_SFSENTRY_CHANGED is
		 * fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. But you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param arr The array contains indexes of SfsEntry-instances
		 *            to be validated.
		 * @return The result of the command. If one of the indexes is
		 *         out of range, SfsCtrl::RESULT_INVALIDARG is returned
		 *         and the validation stops (remaining indexes are not
		 *         validated).
		 */
		CommandResult validate(const IndexArray &);

		/**
		 * Validates the SfsEntry at the specified index.
		 *
		 * This is a shortcut for the validate(IndexArray) method, if
		 * you only want to validate an single SfsEntry.
		 *
		 * @param idx Index of SfsEntry to validate. If the index is
		 *           out of range, SfsCtrl::RESULT_INVALIDARG is
		 *           returned.
		 * @return The result of the command.
		 * @see validate(IndexArray)
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
		 * Registers the checksums of the SfsEntries at the specified
		 * indexes at anoubisd.
		 *
		 * The SfsEntry-instances might be updated. For each changed
		 * entry a wxCommandEvent of type anEVT_SFSENTRY_CHANGED is
		 * fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. But you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * The array contains indexes of SfsEntry-instances
		 *            to be validated.
		 *
		 * @param idx The array contains indexes of SfsEntry-instances
		 *            to be registered.
		 * @return The result of the command. If one of the indexes is
		 *         out of range, SfsCtrl::RESULT_INVALIDARG is returned
		 *         and the operation stops (remaining entries are not
		 *         registered).
		 */
		CommandResult registerChecksum(const IndexArray &);

		/**
		 * Registers the checksum of the SfsEntry at the given index
		 * at anoubisd.
		 *
		 * This is a shortcut for the registerChecksum(IndexArray)
		 * method, if you only want to register an single SfsEntry.
		 *
		 * @param idx Index of SfsEntry to register. If the index is
		 *            out of range, SfsCtrl::RESULT_INVALIDARG is
		 *            returned.
		 * @return The result of the command.
		 * @see registerChecksum(IndexArray)
		 */
		CommandResult registerChecksum(unsigned int);

		/**
		 * Removes the checksum of the SfsEntries at the given indexes
		 * from anoubisd.
		 *
		 * The SfsEntry-instances might be updated. For each changed
		 * entry a wxCommandEvent of type anEVT_SFSENTRY_CHANGED is
		 * fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx The array contains indexes of SfsEntry-instances
		 *            to be unregistered.
		 * @return The result of the command. If one of the indexes is
		 *         out of range, SfsCtrl::RESULT_INVALIDARG is returned
		 *         and the operation stops (remaining entries are not
		 *         unregistered).
		 */
		CommandResult unregisterChecksum(const IndexArray &);

		/**
		 * Removes the checksum of the SfsEntry at the given index from
		 * anoubisd.
		 *
		 * This is a shortcut for the unregisterChecksum(IndexArray)
		 * method, if you only want to unregister an single SfsEntry.
		 *
		 * @param idx Index of SfsEntry to unregister. If the index is
		 *            out of range, SfsCtrl::RESULT_INVALIDARG is
		 *            returned.
		 * @return The result of the command.
		 * @see unregisterChecksum(IndexArray)
		 */
		CommandResult unregisterChecksum(unsigned int);

		/**
		 * Updates the checksum of the SfsEntries at the given indexes
		 * at anoubisd.
		 *
		 * The SfsEntry-instances might be updated. For each changed
		 * entry a wxCommandEvent of type anEVT_SFSENTRY_CHANGED is
		 * fired.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx The array contains indexes of SfsEntry-instances
		 *            to be updated.
		 * @return The result of the command. If one of the indexes is
		 *         out of range, SfsCtrl::RESULT_INVALIDARG is returned
		 *         and the operation stops (remaining entries are not
		 *         updated).
		 */
		CommandResult updateChecksum(const IndexArray &);

		/**
		 * Updates the checksum of the SfsEntry at the given index at
		 * anoubisd.
		 *
		 * This is a shortcut for the updateChecksum(IndexArray)
		 * method, if you only want to update an single SfsEntry.
		 *
		 * Note: This is a non-blocking procedure. It means that method
		 * leaves at soon as possible. Bu you can rely on the
		 * anEVT_SFSENTRY_CHANGED-event.
		 *
		 * @param idx Index of SfsEntry to update. If the index is out
		 *            of range, SfsCtrl::RESULT_INVALIDARG is
		 *            returned.
		 * @return The result of the command.
		 * @see updateChecksum(IndexArray)
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
		bool		sigEnabled_;

		void enableCommunication(void);
		void disableCommunication(void);

		void createComCsumGetTasks(const wxString &, bool, bool);
		void createComCsumAddTasks(const wxString &);
		void createComCsumDelTasks(const wxString &);
		void createSfsListTasks(uid_t, const wxString &);
		void createCsumCalcTask(const wxString &);

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
