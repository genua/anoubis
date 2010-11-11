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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include <pwd.h>
#include <grp.h>

#include <wx/stdpaths.h>

#include "JobCtrl.h"
#include "MainFrame.h"
#include "MainUtils.h"
#include "main.h"

#include "Singleton.cpp"
template class Singleton<MainUtils>;

MainUtils::MainUtils(void) : Singleton<MainUtils>() {
	paths_.SetInstallPrefix(wxT(PACKAGE_PREFIX));

	if (!wxDirExists(paths_.GetUserDataDir())) {
		wxMkdir(paths_.GetUserDataDir());
	}

	/*
	 * Initialize all modules with zero early on. This is important
	 * to avoid bogus return values from wxGetApp().getModule(...)
	 * during startup.
	 */
	for (int i=0; i<ANOUBIS_MODULESNO; ++i) {
		modules_[i] = NULL;
	}

#ifdef LINUX
	grubPath_ = wxT("/boot/grub/menu.lst");
#else
	grubPath_ = wxT("");
#endif
	wxConfig::Get()->Read(wxT("/Options/GrubConfigPath"), &grubPath_);
}

MainUtils::~MainUtils(void)
{
	for (int i=0; i<ANOUBIS_MODULESNO; ++i) {
		delete modules_[i];
	}
}

uid_t
MainUtils::getUserIdByName(wxString name) const
{
	struct passwd	*pwd;

	pwd = getpwnam(name.fn_str());
	if (pwd) {
		return pwd->pw_uid;
	} else {
		return (uid_t)-1;
	}
}

/*
 * This function caches the last lookup to speed things up for cases
 * like the rule editor where we call this functions for each application
 * block.
 */
wxString
MainUtils::getUserNameById(uid_t uid) const
{
	static int	 lastuid = -1;
	static wxString	 lastname = wxEmptyString;
	struct passwd	*pwd;

	if (lastuid < 0 || uid != (uid_t)lastuid) {
		pwd = getpwuid(uid);
		if (pwd && pwd->pw_name) {
			lastuid = uid;
			lastname = wxString::From8BitData(pwd->pw_name,
			    strlen(pwd->pw_name));
		} else {
			lastuid = -1;
			lastname = wxEmptyString;
		}
	}
	return lastname;
}

/*
 * Get the group name for the specified gid.
 */
wxString
MainUtils::getGroupNameById(gid_t gid) const
{
	struct group	*group;

	group = getgrgid(gid);
	if (group && group->gr_name) {
		return wxString::From8BitData(
		    group->gr_name, strlen(group->gr_name));
	} else {
		return wxEmptyString;
	}
}

wxString
MainUtils::getDataDir(void)
{
	return (paths_.GetUserConfigDir());
}

void
MainUtils::status(wxString msg)
{
	MainFrame *mainFrame = dynamic_cast<MainFrame*>(
	    wxGetApp().GetTopWindow());
	if (mainFrame != NULL) {
		mainFrame->SetStatusText(msg, 0);
	}
}

void
MainUtils::initModules(MainFrame *mainFrame)
{
	modules_[OVERVIEW] = new ModOverview(mainFrame);
	modules_[ALF]      = new ModAlf(mainFrame);
	modules_[SFS]      = new ModSfs(mainFrame);
	modules_[SB]       = new ModSb(mainFrame);
#ifdef LINUX
	modules_[PG]       = new ModPlayground(mainFrame);
#endif
	modules_[ANOUBIS]  = new ModAnoubis(mainFrame);

	((ModOverview*)modules_[OVERVIEW])->addModules(modules_);
	mainFrame->addModules(modules_);

	/* XXX [ST]: BUG #424
	 * The following should be considered as a hack to update the state of
	 * Module ALF by calling the update()-method.
	 * Eventually the actual call has to be triggered by an event.
	 */
	((ModAlf*)modules_[ALF])->update();
	((ModSfs*)modules_[SFS])->update();
	((ModSfs*)modules_[SB])->update();
#ifdef LINUX
	((ModPlayground*)modules_[PG])->update();
#endif
	((ModAnoubis*)modules_[ANOUBIS])->update();
}

Module *
MainUtils::getModule(enum moduleIdx idx)
{
	return (modules_[idx]);
}

bool
MainUtils::connectCommunicator(bool doConnect)
{
	if (doConnect == JobCtrl::instance()->isConnected()) {
		/* No change of state */
		return (false);
	}

	if (doConnect) {
		return (JobCtrl::instance()->connect());
	} else {
		JobCtrl::instance()->disconnect();
		return (true);
	}
}

void
MainUtils::checkBootConf(void)
{
	if (grubPath_.IsEmpty())
		return;

	/*
	 * Now check if a new kernel has been installed since the last
	 * start of xanoubis.
	 */
	time_t		savedTime = 0, lastTime = 0;
	wxString	msg;
	struct stat	sbuf;

	wxConfig::Get()->Read(wxT("/Options/GrubModifiedTime"), &savedTime, 0);
	if (savedTime) {
		int ret = stat(grubPath_.fn_str(), &sbuf);
		if (ret != -1)
			lastTime = sbuf.st_mtime;
		if (savedTime < lastTime) {
			msg = _("The Boot Loader configuration has been"
			    " updated. Please make sure to boot an Anoubis"
			    " Kernel.");
			AnMessageDialog dlg(wxGetApp().GetTopWindow(), msg,
			    _("Warning"), wxOK | wxICON_WARNING);
			dlg.onNotifyCheck(
			    wxT("/Options/ShowKernelUpgradeMessage"));
			dlg.ShowModal();
			wxConfig::Get()->Write(
			    wxT("/Options/GrubModifiedTime"), time(NULL));
		}
	} else {
		wxConfig::Get()->Write(
		    wxT("/Options/GrubModifiedTime"), time(NULL));
	}
}

wxString
MainUtils::getGrubPath(void)
{
	return grubPath_;
}

void
MainUtils::autoStart(bool autostart)
{
	wxString deskFile = wxStandardPaths::Get().GetDataDir() +
	    wxT("/xanoubis.desktop");
	wxString kdeDeskFile = deskFile + wxT(".kde");
	wxString gnomeDeskFile = deskFile + wxT(".gnome");
	wxString kPath = paths_.GetUserConfigDir() + wxT("/.kde/Autostart");
	wxString gPath = paths_.GetUserConfigDir() + wxT("/.config/autostart");
	wxString kAutoFile = kPath + wxT("/xanoubis.desktop");
	wxString gAutoFile = gPath + wxT("/xanoubis.desktop");

	if (autostart == true) {
		if (wxDirExists(kPath) == false) {
			wxFileName::Mkdir(kPath, 0777, wxPATH_MKDIR_FULL);
		}
		if (wxDirExists(gPath) == false) {
			wxFileName::Mkdir(gPath, 0777, wxPATH_MKDIR_FULL);
		}

		if (wxFileExists(kAutoFile) == false) {
			if (wxFileExists(kdeDeskFile))
				wxCopyFile(kdeDeskFile, kAutoFile);
			else
				wxCopyFile(deskFile, kAutoFile);
		}
		if (wxFileExists(gAutoFile) == false) {
			if (wxFileExists(gnomeDeskFile))
				wxCopyFile(gnomeDeskFile, gAutoFile);
			else
				wxCopyFile(deskFile, gAutoFile);
		}
	} else {
		if (wxFileExists(kAutoFile) == true) {
			if (wxRemove(kAutoFile) != 0) {
				wxString msg = wxString::Format(_
				    ("Couldn't remove Autostart file: %ls"),
				    kAutoFile.c_str());
				anMessageBox(msg, _("Error"), wxICON_ERROR);
			}
		}
		if (wxFileExists(gAutoFile) == true) {
			if (wxRemove(gAutoFile) != 0) {
				wxString msg = wxString::Format(_
				    ("Couldn't remove Autostart file: %ls"),
				    gAutoFile.c_str());
				anMessageBox(msg, _("Error"), wxICON_ERROR);
			}
		}
	}
}
