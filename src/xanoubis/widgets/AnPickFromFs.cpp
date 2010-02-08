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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stdpaths.h>

#include "AnPickFromFs.h"

AnPickFromFs::AnPickFromFs(wxWindow *parent, wxWindowID id, const wxPoint & pos,
    const wxSize & size, long style, const wxString & name)
    : wxPanel(parent, id, pos, size, style, name)
{
	wxFlexGridSizer *mainSizer;

	mainSizer = new wxFlexGridSizer(2, 3, 0, 0);
	mainSizer->AddGrowableCol(1);
	mainSizer->SetFlexibleDirection(wxBOTH);
	mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	label_ = new wxStaticText(this, wxID_ANY, wxT("<no title>"),
	    wxDefaultPosition, wxDefaultSize, 0);
	label_->Wrap(-1);
	mainSizer->Add(label_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	inputTextCtrl_ = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	mainSizer->Add(inputTextCtrl_, 1,
	    wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

	pickButton_ = new wxButton(this, wxID_ANY, _("Pick..."),
	    wxDefaultPosition, wxDefaultSize, 0);
	pickButton_->SetToolTip(_("use right mouse button to alter pick mode"));
	mainSizer->Add(pickButton_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	fileMenuId_ = wxNewId();
	pickButtonMenu_.AppendRadioItem(fileMenuId_, _("Pick a file"));
	dirMenuId_ = wxNewId();
	pickButtonMenu_.AppendRadioItem(dirMenuId_, _("Pick a directory"));

	mainSizer->Add(0, 0, 1, wxEXPAND, 5);

	infoLabel_ = new wxStaticText(this, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize, 0);
	infoLabel_->Wrap(-1);
	infoLabel_->Hide();
	infoLabel_->SetFont(wxFont(8, 70, 90, 90, false, wxEmptyString));
	infoLabel_->SetForegroundColour(wxSystemSettings::GetColour(
	    wxSYS_COLOUR_GRAYTEXT));
	mainSizer->Add(infoLabel_, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	inputTextCtrl_->Connect(wxEVT_KILL_FOCUS,
	    wxFocusEventHandler(AnPickFromFs::onTextKillFocus), NULL, this);
	inputTextCtrl_->Connect(wxEVT_COMMAND_TEXT_ENTER,
	    wxCommandEventHandler(AnPickFromFs::onTextEnter), NULL, this);
	pickButton_->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnPickFromFs::onPickButton), NULL, this);
	pickButton_->Connect(wxEVT_RIGHT_DOWN,
	    wxMouseEventHandler(AnPickFromFs::onPickButtonMenu), NULL, this);

	this->SetSizer(mainSizer);
	this->Layout();

	fileName_ = wxEmptyString;
	setMode(MODE_NONE);
}

void
AnPickFromFs::setFileName(const wxString & fileName)
{
	fileName_ = fileName;
	inputTextCtrl_->ChangeValue(fileName);
}

wxString
AnPickFromFs::getFileName(void) const
{
	return (fileName_);
}

void
AnPickFromFs::setTitle(const wxString & label)
{
	label_->SetLabel(label);
	Layout();
	Refresh();
}

wxSize
AnPickFromFs::getTitleSize(void)
{
	return (label_->GetSize());
}

void
AnPickFromFs::setTitleMinSize(wxSize minSize)
{
	label_->SetMinSize(minSize);
	Layout();
	Refresh();
}

wxSize
AnPickFromFs::getTitleMinSize(void)
{
	return (label_->GetMinSize());
}

void
AnPickFromFs::setButtonLabel(const wxString & label)
{
	pickButton_->SetLabel(label);
	Layout();
	Refresh();
}

void
AnPickFromFs::setMode(int mode)
{
	pickerMode_ = mode;

	if (mode & (MODE_FILEOK | MODE_DIROK)) {
		pickButton_->Enable();
	} else {
		pickButton_->Disable();
	}

	/* Defaults */
	pickButtonMenu_.Check(fileMenuId_, false);
	pickButtonMenu_.Check(dirMenuId_, false);
	pickButtonMenu_.Enable(fileMenuId_, false);
	pickButtonMenu_.Enable(dirMenuId_, false);
	if (mode & MODE_FILEOK) {
		pickButtonMenu_.Check(fileMenuId_, true);
		pickButtonMenu_.Enable(fileMenuId_, true);
	}
	if (mode & MODE_DIROK) {
		pickButtonMenu_.Enable(dirMenuId_, true);
		if ((mode & MODE_FILEOK) == 0)
			pickButtonMenu_.Check(dirMenuId_, true);
	}
}

void
AnPickFromFs::adoptFileName(const wxString &fileName)
{
	char		 resolve[PATH_MAX];
	char		*resolved;
	wxString	 msg;

	startChange();
	if (fileName.IsEmpty() || fileName == wxT("any")
	    || (pickerMode_ & MODE_NORESOLVE)) {
		fileName_ = fileName;
	} else {
		resolved = realpath(fileName.fn_str(), resolve);
		if (resolved != NULL) {
			fileName_ = wxString::From8BitData(resolved);
			if (fileName_ != fileName) {
				msg = _("Symbolic link was resolved");
			} else {
				msg = wxEmptyString;
			}
		} else {
			msg = wxString::From8BitData(strerror(errno));
			msg.Prepend(_("Failure to resolve: "));
			fileName_ = fileName;
		}
		/* XXX ch: depending on the mode we could do furter checks */
		showInfo(msg);
	}
	if (inputTextCtrl_->GetValue() != fileName_) {
		inputTextCtrl_->ChangeValue(fileName_);
	}

	inputTextCtrl_->DiscardEdits();

	finishChange();
}

void
AnPickFromFs::showInfo(const wxString &info)
{
	wxWindow	*parent;
	wxString	 label;

	if (pickerMode_ & MODE_NOINFO)
		return;

	label = info;
	label.Prepend(wxT("("));
	label.Append(wxT(")"));

	infoLabel_->SetLabel(label);
	infoLabel_->Show(!info.IsEmpty()); /* show if not empty */

	parent = GetParent();
	if (parent != NULL) {
		parent->Layout();
		parent->Refresh();
	} else {
		Layout();
		Refresh();
	}
}

wxString
AnPickFromFs::getDialogPath(void) const
{
	wxFileName fn(fileName_);
	wxString path = fn.GetPath();

	if (path.IsEmpty())
		return (wxStandardPaths::Get().GetUserDataDir());
	else
		return (path);
}

void
AnPickFromFs::onTextKillFocus(wxFocusEvent &)
{
	/*
	 * Only validate on a set-Focus event if the user has modified
	 * the text in the text ctrl since we last called adoptFileName.
	 */
	if (inputTextCtrl_->IsModified()) {
		adoptFileName(inputTextCtrl_->GetValue());
	}
}

void
AnPickFromFs::onTextEnter(wxCommandEvent & event)
{
	adoptFileName(event.GetString());
}

void
AnPickFromFs::onPickButton(wxCommandEvent &)
{
	if (pickButtonMenu_.IsChecked(dirMenuId_)) {
		wxDirDialog	*dlg;
		long		 flags = wxDD_DEFAULT_STYLE;

		if ((pickerMode_ & MODE_NEW) == 0)
			flags |= wxDD_DIR_MUST_EXIST;

		dlg = new wxDirDialog(this,wxEmptyString, wxEmptyString, flags);

		wxBeginBusyCursor();
		dlg->SetPath(getDialogPath());
		wxEndBusyCursor();

		if (dlg->ShowModal() == wxID_OK) {
			adoptFileName(dlg->GetPath());
		}
		dlg->Destroy();
	} else {
		wxFileDialog	*dlg;
		long		 flags;
		wxFileName	 defaultPath;

		defaultPath.Assign(fileName_);
		flags  = wxDD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST;
		if (pickerMode_ & MODE_NEW)
			flags = wxFD_SAVE;

		dlg = new wxFileDialog(this,wxEmptyString, wxEmptyString,
		    wxEmptyString, wxEmptyString, flags);

		wxBeginBusyCursor();
		dlg->SetDirectory(getDialogPath());
		dlg->SetFilename(defaultPath.GetFullName());
		dlg->SetWildcard(wxT("*"));
		wxEndBusyCursor();

		if (dlg->ShowModal() == wxID_OK) {
			adoptFileName(dlg->GetPath());
		}
		dlg->Destroy();
	}
}

void
AnPickFromFs::onPickButtonMenu(wxMouseEvent &)
{
	PopupMenu(&pickButtonMenu_, ScreenToClient(wxGetMousePosition()));
}
