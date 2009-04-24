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

#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/menu.h>

class IndexArray;
class SfsCtrl;
class SfsEntry;

/**
 * List-control displays SfsEntry-instances.
 *
 * A list-control prepared to display SfsEntry-instances. You can setup an
 * filter (ModSfsListCtrl::DisplayOption) to specify what kind of entries
 * should be displayed.
 *
 * The row-index does not necessarily correspond to the index of the SfsEntry
 * in the SfsDirectory! But you can use getSfsIndexAt() resp. getListIndexOf()
 * to map the indexes to each other.
 *
 * The complete list is refreshed with refreshList(), an individual row is
 * refreshed by calling refreshEntry().
 *
 * @note Yoou need to setup the SfsCtrl after instantiation of the class
 *       (setSfsCtrl())!
 */
class ModSfsListCtrl : public wxListCtrl
{
	public:
		/**
		 * Display options.
		 *
		 * Describes which SfsEntry-instances should be displayed.
		 *
		 * @see refreshList()
		 */
		enum DisplayOption
		{
			SHOW_ALL = 0,	/*!< All entries of the model are
					     displayed. On filter is
					     applied. */
			SHOW_EXISTING,	/*!< Only existing files are displayed.
					     @see SfsEntry::fileExists() */
			SHOW_CHANGED,	/*!< Only entries with a non-matching
					     checksum are displayed.
					  @see SfsEntry::isChecksumChanged() */
			SHOW_CHECKSUM,	/*!< Only entries with a rgistered
					     checksum are displayed.
					     @see SfsEntry::haveChecksum() */
			SHOW_ORPHANED	/*!< Only orphaned files are displayed.
					     @see SfsEntry::fileExists() */
		};

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
		 * Returns the index of the SfsEntry at the specified
		 * list-index.
		 *
		 * This method is used to map an list-entry to an SfsEntry.
		 * The resulting index can be used to fetch an SfsEntry
		 * (SfsDirectory::getEntry()).
		 *
		 * @param listIdx Row-index of the list. The method returns
		 *                the index of the SfsEntry, which is displayed
		 *                at the row.
		 * @return Index of SfsEntry in SfsDirectory. If the specified
		 *         list-index is out of range (no such row), -1 is
		 *         returned.
		 */
		int getSfsIndexAt(unsigned int) const;

		/**
		 * Returns the list-index, where the SfsEntry at the specified
		 * index is displayed.
		 *
		 * This method is used to map an SfsEntry to a list-entry. Use
		 * the method, if you want to know, where an SfsEntry is
		 * displayed.
		 *
		 * @param sfsIdx Index of requested SfSEntry in the
		 *               SfsDirectory.
		 * @return Row-index of the view, where the requested SfsEntry
		 *         is displayed. If the SfsEntry is not displayed, -1
		 *         is returned.
		 */
		int getListIndexOf(unsigned int) const;

		/**
		 * Returns sfs-indexes of selected rows.
		 *
		 * This function returns SfsEntry indicies. Most of the
		 * time these are not identical to list indices.
		 *
		 * @return Indexes of selected SfsEntry-instances.
		 */
		IndexArray getSfsIndexes(void) const;

		/**
		 * Refreshes the complete list.
		 *
		 * All rows are removed and, according to the specified
		 * view-filter, re-created.
		 *
		 * @param option Specifies which entries from the model should
		 *               be displayed.
		 */
		void refreshList(DisplayOption);

		/**
		 * Refreshes a single row of the list.
		 *
		 * The row is not removed but the SfsEntry displayed at the row
		 * is re-read and the row is filled with refreshed data.
		 *
		 * @param idx Row-index to be refreshed
		 */
		void refreshEntry(unsigned int);

	private:
		/**
		 * Columns of the list.
		 */
		enum Columns {
			COLUMN_FILE = 0,	/*!<  File column */
			COLUMN_CHECKSUM,	/*!<  checksum column */
			COLUMN_SIGNATURE,	/*!<  signaure column */
			COLUMN_EOL		/*!<  Behind last column */
		};

		/**
		 * Tests whether the specified SfsEntry can be displayed.
		 *
		 * The SfsEntry can be displayed, if it matches with the
		 * specified DisplayOption.
		 *
		 * @param entry SfsEntry to be checked
		 * @param option The display filter to check against
		 * @return true is returned, if the SfsEntry can be displayed,
		 *         false otherwise.
		 */
		bool canDisplay(const SfsEntry &, DisplayOption) const;

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
		 * The SfsCtrl.
		 *
		 * Used to get a reference to the model.
		 */
		SfsCtrl		*sfsCtrl_;

		/**
		 * An ok-icon.
		 */
		wxIcon		*okIcon_;

		/**
		 * A warning-icon.
		 */
		wxIcon		*warnIcon_;

		/**
		 * An error-icon.
		 */
		wxIcon		*errorIcon_;

		/**
		 * An icon representing a symlink.
		 */
		wxIcon		*symlinkIcon_;

		/**
		 * The image-list assigned with the list contains all icons
		 * referenced by the list.
		 */
		wxImageList	imageList_;

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
};

#endif	/* _MODSFSLISTCTRL_H_ */
