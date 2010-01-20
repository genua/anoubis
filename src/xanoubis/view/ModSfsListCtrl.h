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

#ifndef _MODSFSLISTCTRL_H_
#define _MODSFSLISTCTRL_H_

#include <wx/menu.h>

#include <AnListCtrl.h>
#include <SfsDirectory.h>

class IndexArray;
class SfsCtrl;
class SfsEntry;
class wxProgressDialog;

/**
 * List-control displays SfsEntry-instances.
 *
 * A list-control prepared to display SfsEntry-instances.
 *
 * The complete list is refreshed with refreshList(), an individual row is
 * refreshed by calling refreshEntry().
 *
 * @note Yoou need to setup the SfsCtrl after instantiation of the class
 *       (setSfsCtrl())!
 */
class ModSfsListCtrl : public AnListCtrl, private SfsDirectoryScanHandler
{
	public:
		/**
		 * Constructs the sfs-list.
		 *
		 * @param parent Parent window. Must not be NULL.
		 * @param id Window identifier. A value of -1 indicates a
		 *           default value.
		 * @param pos Window position.
		 * @param size Window size. If the default size (-1, -1) is
		 *             specified then the window is sized
		 *             appropriately.
		 * @param style Window style.
		 */
		ModSfsListCtrl(wxWindow *, wxWindowID, const wxPoint &,
		    const wxSize &, long);

		/**
		 * D'tor.
		 */
		~ModSfsListCtrl(void);

		/**
		 * Returns the assigned SfsCtrl-instance.
		 *
		 * @return Assigned SfsCtrl.
		 */
		SfsCtrl *getSfsCtrl(void) const;

		/**
		 * Assigns a SfsCtrl to the view.
		 *
		 * The list gets a reference to the model through the
		 * controller, so you need to assign the controller right after
		 * contruction of the object.
		 *
		 * @param sfsCtrl The controller to be assigned
		 */
		void setSfsCtrl(SfsCtrl *);

		/**
		 * Returns indexes of selected rows.
		 *
		 * @return Indexes of selected SfsEntry-instances.
		 */
		IndexArray getSelectedIndexes(void) const;

	private:
		/**
		 * Invoked if a row is activated (aka double-click).
		 *
		 * Displays the ModSfsDetailsDlg for the selected SfsEntry.
		 */
		void OnListItemActivated(wxListEvent&);

		/**
		 * Invoked if a row was selected with a right mouse-click.
		 * Updates and displays the popup-menu.
		 */
		void OnListItemRightClicked(wxListEvent&);

		/**
		 * Invoked if the "Show details" menu-item was selected in the
		 * popup-menu.
		 *
		 * Displays the ModSfsDetailsDlg for the selected SfsEntry.
		 */
		void OnPopupShowDetailsSelected(wxCommandEvent&);

		/**
		 * Invoked if the "Resolve link" menu-item was selected in the
		 * popup-menu.
		 *
		 * Resolves the symlink and switches to the new path.
		 */
		void OnPopupResolveLinkSelected(wxCommandEvent&);

		/**
		 * Implementation of SfsDirectoryScanHandler::scanUpdate().
		 */
		void scanUpdate(unsigned int, unsigned int);

		/**
		 * Implementation of SfsDirectoryScanHandler::scanFinished().
		 *
		 * Destroys scanProgressDlg_.
		 */
		void scanFinished(bool);

		/**
		 * The SfsCtrl.
		 *
		 * Used to get a reference to the model.
		 */
		SfsCtrl		*sfsCtrl_;

		/**
		 * The popup-menu of the list.
		 */
		wxMenu		popupMenu_;

		/**
		 * Currently selected row.
		 *
		 * Used by the popup-menu to select an SfsEntry.
		 */
		int		currentSelection_;

		/**
		 * Progress dialog displayed when a filesystem-scan is running.
		 */
		wxProgressDialog	*scanProgressDlg_;
};

#endif	/* _MODSFSLISTCTRL_H_ */
