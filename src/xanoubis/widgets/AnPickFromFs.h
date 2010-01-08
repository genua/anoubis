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

#ifndef _ANPICKFROMFS_H_
#define _ANPICKFROMFS_H_

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include "Policy.h"

/**
 * This realizes the Pick-From-Filesystem-Widgets.
 *
 * The PickFromFs widget is an ordinary panel containing the widgets for
 * choosing a file. It's possible to enter the filename manualy or to hit
 * the pick-button, which will open a gtk file picker dialog.\n
 * \n
 * \b Usage (with the wxformbuilder):\n
 * - Add a normal wxPanel
 * - Set the property 'subclass.name' to "AnPickFromFs"
 * - Set the property 'subclass.include' to "AnPickFromFs.h"
 * - Disable the stretching of the panel. (It's Alt-s or the icon with the
 *   square and the arrow on it's top and on the right side.)
 * - The panel can't be extedned by additional widgets.
 * - The panel is a subject and it's parent should be an observer to be
 *   notified about a picked file.
 * - The parent has to set the title with setTitle()
 * - The parent has to set the mode with setMode(). (The pickButton is
 *   disabled untill set and mode is not NONE).
 * - In case of MODE_NEWFILE or MODE_NEWDIR, no error messages about item
 *   is not existing, is disabled.
 */
class AnPickFromFs : public wxPanel, public Subject
{
	public:
		/**
		 * Picker Modes.
		 */
		enum Modes {
			MODE_NONE = 0,
			MODE_FILE,	/**< use FileDialog only */
			MODE_NEWFILE,	/**< use FileDialog, no error */
			MODE_DIR,	/**< use DirDialog only */
			MODE_NEWDIR,	/**< use DirDialog, no error */
			MODE_BOTH,	/**< use both (choose by menu) */
		};

		/**
		 * Constructor of the details panel.
		 * It has to have the same signature as a ordinary wxPanel,
		 * so wxformbuilder can just exchange wxPanel with AnPickFromFs.
		 */
		AnPickFromFs(wxWindow *parent, wxWindowID id = wxID_ANY,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxTAB_TRAVERSAL,
		    const wxString& name = wxPanelNameStr);

		/**
		 * Set file name.
		 * With this method the widgets are initialized with the
		 * given filename.
		 * @param 1st The filename this widget should display.
		 * @return Nothing.
		 */
		void setFileName(const wxString &);

		/**
		 * Get file name.
		 * After the user has picked a file, the parent can use
		 * this method to get the picked file name.
		 * @paran None.
		 * @return The picked file name.
		 */
		wxString getFileName(void) const;

		/**
		 * Set title.
		 * Set the text bevore the text control.
		 * @param 1st The new title.
		 * @return Nothing.
		 */
		void setTitle(const wxString &);

		/**
		 * Get title size.
		 * Get the size of the title.
		 * @param None
		 * @return Size of title.
		 */
		wxSize getTitleSize(void);

		/**
		 * Set title mininal size.
		 * Set the minimal size of the title. Use this to fix
		 * alignment. It's not elegant but it works (somehow).
		 * @param 1st New minimal size.
		 * @return Nothing.
		 */
		void setTitleMinSize(const wxSize);

		/**
		 * Get title minimal size.
		 * Get the minimal size of the title.
		 * @param None
		 * @return Minimal size of title.
		 */
		wxSize getTitleMinSize(void);

		/**
		 * Set button label.
		 * Use this method to set an alternate text on the pick button.
		 * @param 1st The new label of the pick button.
		 * @return Nothing.
		 */
		void setButtonLabel(const wxString &);

		/**
		 * Set mode of picker
		 * Use this to influence which dialog the pickButton will
		 * open. In case of both, the user can choose by right mouse
		 * button.
		 * @param 1st The new mode.
		 * @return Nothing.
		 */
		void setMode(enum Modes);

	protected:
		/**
		 * This is the text just left to the textCtrl. It is set to the
		 * value of window_name (set within the properties in the
		 * wxformbuilder).\n
		 */
		wxStaticText *label_;

		/**
		 * This is the info text below the textCtrl, informing the
		 * user about errors.
		 */
		wxStaticText *infoLabel_;

		/**
		 * This is the text control a user can enter the file manually.
		 * This will also show the file picked by file chooser dialog.
		 */
		wxTextCtrl *inputTextCtrl_;

		/**
		 * This button will open the file chooser dialog.
		 */
		wxButton *pickButton_;

	private:
		/**
		 * Keep the picked filename.
		 */
		wxString fileName_;

		/**
		 * Keep widget mode here.
		 */
		enum Modes pickerMode_;

		/**
		 * This is the Id of the menu entry for file.
		 */
		long fileMenuId_;

		/**
		 * This is the Id of the menu entry for directory.
		 */
		long dirMenuId_;

		/**
		 * The pupup-menu of the pick button
		 */
		wxMenu pickButtonMenu_;

		/**
		 * Adopt file name.
		 * This method performs the resolving of symlinks, updates
		 * the infoLabel with information and error messages and
		 * stores the result in fileName_.
		 * @param 1st The filename to be stored.
		 * @return Nothing.
		 */
		void adoptFileName(const wxString &);

		/**
		 * Show information.
		 * Use this method to update the infoLabel_.
		 * Use an empty string to disable/hide the info text.
		 * @param 1st Text to be shown if not
		 * @return Nothing.
		 */
		void showInfo(const wxString &);

		/**
		 * Handle focus events from inputTextCtrl (e.g on hit <tab>).
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onTextKillFocus(wxFocusEvent &);

		/**
		 * Handle events from inputTextCtrl.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onTextEnter(wxCommandEvent &);

		/**
		 * Handle events from pickButton.
		 * This will open a file chooser and on 'ok' the choosen
		 * file is written to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPickButton(wxCommandEvent &);

		/**
		 * Handle mouse events from pickButton.
		 * This will open the context menue of the pickButton
		 * on right mouse click.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPickButtonMenu(wxMouseEvent &);
};

#endif	/* _ANPICKFROMFS_H_ */
