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

#include "ModAnoubisProfileDialogImpl.h"

ModAnoubisProfileDialogImpl::ModAnoubisProfileDialogImpl(wxWindow* parent)
    : ModAnoubisProfileDialogBase(parent, -1)
{
	ActionButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(
	       ModAnoubisProfileDialogImpl::OnActionButtonClick),
	    NULL, this);
	CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(
	       ModAnoubisProfileDialogImpl::OnCancelButtonClick),
	    NULL, this);
	HighCheckBox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
	    wxCommandEventHandler(ModAnoubisProfileDialogImpl::OnCheckBoxClick),
	    NULL, this);
	MediumCheckBox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
	    wxCommandEventHandler(ModAnoubisProfileDialogImpl::OnCheckBoxClick),
	    NULL, this);
	AdminCheckBox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
	    wxCommandEventHandler(ModAnoubisProfileDialogImpl::OnCheckBoxClick),
	    NULL, this);
}

ModAnoubisProfileDialogImpl::~ModAnoubisProfileDialogImpl(void)
{
}

bool
ModAnoubisProfileDialogImpl::isHighProfileSelected() const
{
	return HighCheckBox->IsChecked();
}

bool
ModAnoubisProfileDialogImpl::isMediumProfileSelected() const
{
	return MediumCheckBox->IsChecked();
}

bool
ModAnoubisProfileDialogImpl::isAdminProfileSelected() const
{
	return AdminCheckBox->IsChecked();
}

void ModAnoubisProfileDialogImpl::OnActionButtonClick(wxCommandEvent&)
{
	EndModal(wxID_OK);
}

void ModAnoubisProfileDialogImpl::OnCancelButtonClick(wxCommandEvent&)
{
	EndModal(wxID_CANCEL);
}

void ModAnoubisProfileDialogImpl::OnCheckBoxClick(wxCommandEvent&)
{
	ActionButton->Enable(canBeEnabled());
}

ModAnoubisRestoreProfileDialog::ModAnoubisRestoreProfileDialog(
    wxWindow* parent) : ModAnoubisProfileDialogImpl(parent)
{
	SetTitle(_("Restore Profile"));
	ActionButton->SetLabel(_("Restore"));
	DialogLabel->SetLabel(_("Select the profile(s) you want to restore:"));
}

bool
ModAnoubisRestoreProfileDialog::canBeEnabled() const
{
	bool ok = false;

	ok |= HighCheckBox->IsChecked();
	ok |= MediumCheckBox->IsChecked();
	ok |= AdminCheckBox->IsChecked();

	return (ok);
}

ModAnoubisImportProfileDialog::ModAnoubisImportProfileDialog(wxWindow* parent)
    : ModAnoubisProfileDialogImpl(parent)
{
	SetTitle(_("Import Profile"));
	ActionButton->SetLabel(_("Import"));
	DialogLabel->SetLabel(_("Select the profile you want to import:"));
}

bool
ModAnoubisImportProfileDialog::canBeEnabled() const
{
	return HighCheckBox->IsChecked() ^
	    MediumCheckBox->IsChecked() ^
	    AdminCheckBox->IsChecked();
}

ModAnoubisExportProfileDialog::ModAnoubisExportProfileDialog(wxWindow* parent)
    : ModAnoubisProfileDialogImpl(parent)
{
	SetTitle(_("Export Profile"));
	ActionButton->SetLabel(_("Export"));
	DialogLabel->SetLabel(_("Select the profile you want to export:"));
}

bool
ModAnoubisExportProfileDialog::canBeEnabled() const
{
	return HighCheckBox->IsChecked() ^
	    MediumCheckBox->IsChecked() ^
	    AdminCheckBox->IsChecked();
}
