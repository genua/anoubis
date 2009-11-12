# Copyright (c) 2007 GeNUA mbH <info@genua.de>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

bin_PROGRAMS = xanoubis

apndir	= src/libapn
libapn  = $(top_builddir)/$(apndir)/libapn.a
bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
xanoubisdir    = src/xanoubis
anoubischatdir = src/libanoubischat
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
anoubisprotocoldir = src/libanoubisprotocol
libanoubisprotocol = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.a
anoubissigdir = src/libanoubissig
libanoubissig = $(top_builddir)/$(anoubissigdir)/libanoubissig.a
anoubisuidir = src/libanoubisui
libanoubisui = $(top_builddir)/$(anoubisuidir)/libanoubisui.a

xanoubis_CPPFLAGS = \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(apndir) \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client \
	-I$(top_srcdir)/$(anoubissigdir) \
	-I$(top_srcdir)/$(anoubisuidir) \
	-I$(top_srcdir)/$(anoubisuidir)/apnvm \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view  \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	-I$(top_srcdir)/$(xanoubisdir)/widgets \
	-I$(top_srcdir)/$(xanoubisdir)/pattern \
	-I$(top_srcdir)/$(xanoubisdir)/job \
	$(WX_CPPFLAGS) \
	$(AM_CPPFLAGS)

xanoubis_CXXFLAGS = \
	-fno-strict-aliasing \
	$(AM_CXXFLAGS) \
	$(WX_CXXFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NOTIFY_CFLAGS) \
	$(SSL_CFLAGS)

xanoubis_DEPEND_LIBS = \
	$(libapn) \
	$(libanoubischat) \
	$(libanoubisprotocol) \
	$(libanoubissig) \
	$(libanoubisui)

if NEEDBSDCOMPAT
xanoubis_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
xanoubis_DEPEND_LIBS += $(libbsdcompat)
endif

xanoubis_DEPENDENCIES = \
	$(xanoubis_DEPEND_LIBS)

xanoubis_LDFLAGS = $(AM_LDFLAGS) $(SSL_LDFLAGS)

xanoubis_LDADD = \
	@WX_LIBS@ \
	@GTK_LIBS@ \
	@GLIB_LIBS@ \
	@NOTIFY_LIBS@ \
	$(xanoubis_DEPEND_LIBS) \
	$(SSL_LIBS) \
	-lfl

programdir = $(datadir)/$(bin_PROGRAMS)
utildir = $(datadir)/$(bin_PROGRAMS)/utils
icondir = $(datadir)/$(bin_PROGRAMS)/icons
profiledir = $(datadir)/$(bin_PROGRAMS)/profiles
wizarddir = $(profiledir)/wizard
desktopdir = $(datadir)/$(bin_PROGRAMS)

EXTRA_DIST = \
	catalogs/xanoubis.pot \
	catalogs/de/xanoubis.po \
	catalogs/de/xanoubis.mo

dist_util_SCRIPTS = utils/xanoubis_guessApp.sh
dist_icon_DATA = \
	icons/General_alert_16.png \
	icons/General_alert_48.png \
	icons/General_error_16.png \
	icons/General_error_48.png \
	icons/General_ok_16.png \
	icons/General_ok_48.png \
	icons/General_problem_16.png \
	icons/General_problem_48.png \
	icons/General_question_16.png \
	icons/General_question_48.png \
	icons/General_symlink_16.png \
	icons/General_symlink_48.png \
	icons/General_downarrow_16.png \
	icons/General_rightarrow_16.png \
	icons/ModAlf_alert_16.png \
	icons/ModAlf_alert_48.png \
	icons/ModAlf_black_16.png \
	icons/ModAlf_black_48.png \
	icons/ModAlf_error_16.png \
	icons/ModAlf_error_48.png \
	icons/ModAlf_grey_16.png \
	icons/ModAlf_grey_48.png \
	icons/ModAlf_ok_16.png \
	icons/ModAlf_ok_48.png \
	icons/ModAlf_problem_16.png \
	icons/ModAlf_problem_48.png \
	icons/ModAlf_question_16.png \
	icons/ModAlf_question_48.png \
	icons/ModAnoubis_alert_16.png \
	icons/ModAnoubis_alert_48.png \
	icons/ModAnoubis_black_16.png \
	icons/ModAnoubis_black_48.png \
	icons/ModAnoubis_error_16.png \
	icons/ModAnoubis_error_48.png \
	icons/ModAnoubis_grey_16.png \
	icons/ModAnoubis_grey_48.png \
	icons/ModAnoubis_ok_16.png \
	icons/ModAnoubis_ok_48.png \
	icons/ModAnoubis_problem_16.png \
	icons/ModAnoubis_problem_48.png \
	icons/ModAnoubis_question_16.png \
	icons/ModAnoubis_question_48.png \
	icons/ModOverview_alert_16.png \
	icons/ModOverview_alert_48.png \
	icons/ModOverview_black_16.png \
	icons/ModOverview_black_48.png \
	icons/ModOverview_error_16.png \
	icons/ModOverview_error_48.png \
	icons/ModOverview_grey_16.png \
	icons/ModOverview_grey_48.png \
	icons/ModOverview_ok_16.png \
	icons/ModOverview_ok_48.png \
	icons/ModOverview_problem_16.png \
	icons/ModOverview_problem_48.png \
	icons/ModOverview_question_16.png \
	icons/ModOverview_question_48.png \
	icons/ModSb_black_48.png \
	icons/ModSb_error_48.png \
	icons/ModSb_ok_48.png \
	icons/ModSfs_alert_16.png \
	icons/ModSfs_alert_48.png \
	icons/ModSfs_black_16.png \
	icons/ModSfs_black_48.png \
	icons/ModSfs_error_16.png \
	icons/ModSfs_error_48.png \
	icons/ModSfs_grey_16.png \
	icons/ModSfs_grey_48.png \
	icons/ModSfs_ok_16.png \
	icons/ModSfs_ok_48.png \
	icons/ModSfs_problem_16.png \
	icons/ModSfs_problem_48.png \
	icons/ModSfs_question_16.png \
	icons/ModSfs_question_48.png \
	icons/xanoubis.png
dist_profile_SCRIPTS = \
	profiles/high \
	profiles/medium \
	profiles/admin
dist_wizard_SCRIPTS = \
	profiles/wizard/alf \
	profiles/wizard/sandbox
dist_desktop_DATA = \
	xanoubis.desktop

install-data-hook:
	-chmod 444 $(DESTDIR)/$(profiledir)/*
	-chmod 755 $(DESTDIR)/$(profiledir)/wizard
	-chmod 444 $(DESTDIR)/$(profiledir)/wizard/*

install-data-local: i18n-install

xanoubis_SOURCES= \
	model/AlertNotify.cpp \
	model/AlertNotify.h \
	model/AlfOverviewTable.cpp \
	model/AlfOverviewTable.h \
	model/ApnVersion.cpp \
	model/ApnVersion.h \
	model/EscalationNotify.cpp \
	model/EscalationNotify.h \
	model/LocalCertificate.cpp \
	model/LocalCertificate.h \
	model/LogNotify.cpp \
	model/LogNotify.h \
	model/ModAlf.cpp \
	model/ModAlf.h \
	model/ModAnoubis.cpp \
	model/ModAnoubis.h \
	model/ModOverview.cpp \
	model/ModOverview.h \
	model/ModSb.cpp \
	model/ModSb.h \
	model/ModSfs.cpp \
	model/ModSfs.h \
	model/Module.cpp \
	model/Module.h \
	model/Notification.cpp \
	model/Notification.h \
	model/NotificationPerspective.cpp \
	model/NotificationPerspective.h \
	model/NotifyAnswer.cpp \
	model/NotifyAnswer.h \
	model/PrivKey.cpp \
	model/PrivKey.h \
	model/StatusNotify.cpp \
	model/StatusNotify.h \
	model/DaemonAnswerNotify.cpp \
	model/DaemonAnswerNotify.h \
	model/SfsDirectory.cpp \
	model/SfsDirectory.h \
	model/SfsEntry.cpp \
	model/SfsEntry.h \
	model/PolicyRuleSet.cpp \
	model/PolicyRuleSet.h \
	model/AlfAppPolicy.cpp \
	model/AlfAppPolicy.h \
	model/AlfCapabilityFilterPolicy.cpp \
	model/AlfCapabilityFilterPolicy.h \
	model/AlfFilterPolicy.cpp \
	model/AlfFilterPolicy.h \
	model/AppPolicy.cpp \
	model/AppPolicy.h \
	model/ContextAppPolicy.cpp \
	model/ContextAppPolicy.h \
	model/ContextFilterPolicy.cpp \
	model/ContextFilterPolicy.h \
	model/DefaultFilterPolicy.cpp \
	model/DefaultFilterPolicy.h \
	model/FilterPolicy.cpp \
	model/FilterPolicy.h \
	model/Policy.cpp \
	model/Policy.h \
	model/SbAccessFilterPolicy.cpp \
	model/SbAccessFilterPolicy.h \
	model/SbAppPolicy.cpp \
	model/SbAppPolicy.h \
	model/SfsAppPolicy.cpp \
	model/SfsAppPolicy.h \
	model/SfsFilterPolicy.cpp \
	model/SfsFilterPolicy.h \
	model/SfsDefaultFilterPolicy.cpp \
	model/SfsDefaultFilterPolicy.h \
	model/PolicyChecks.cpp \
	model/PolicyChecks.h \
	model/PolicyUtils.cpp \
	model/PolicyUtils.h \
	model/ListCtrlColumn.cpp \
	model/ListCtrlColumn.h \
	model/SimpleOverviewRow.cpp \
	model/SimpleOverviewRow.h \
	model/SimpleOverviewTable.cpp \
	model/SimpleOverviewTable.h \
	view/DlgBackupPolicyBase.h \
	view/DlgBackupPolicyBase.cpp \
	view/DlgBackupPolicyImpl.h \
	view/DlgBackupPolicyImpl.cpp \
	view/DlgLogViewerBase.cpp \
	view/DlgLogViewerBase.h \
	view/DlgLogViewer.cpp \
	view/DlgLogViewer.h \
	view/DlgProfileSelection.cpp \
	view/DlgProfileSelection.h \
	view/DlgRuleEditorBase.cpp \
	view/DlgRuleEditorBase.h \
	view/DlgRuleEditor.cpp \
	view/DlgRuleEditor.h \
	view/DlgRuleEditorAppPage.cpp \
	view/DlgRuleEditorAppPage.h \
	view/DlgRuleEditorFilterActionPage.cpp \
	view/DlgRuleEditorFilterActionPage.h \
	view/DlgRuleEditorFilterAddressPage.cpp \
	view/DlgRuleEditorFilterAddressPage.h \
	view/DlgRuleEditorFilterCapabilityPage.cpp \
	view/DlgRuleEditorFilterCapabilityPage.h \
	view/DlgRuleEditorFilterContextPage.cpp \
	view/DlgRuleEditorFilterContextPage.h \
	view/DlgRuleEditorFilterNetworkPage.cpp \
	view/DlgRuleEditorFilterNetworkPage.h \
	view/DlgRuleEditorFilterPermissionPage.cpp \
	view/DlgRuleEditorFilterPermissionPage.h \
	view/DlgRuleEditorFilterSfsPage.cpp \
	view/DlgRuleEditorFilterSfsPage.h \
	view/DlgRuleEditorFilterSubjectPage.cpp \
	view/DlgRuleEditorFilterSubjectPage.h \
	view/DlgRuleEditorPage.cpp \
	view/DlgRuleEditorPage.h \
	view/DlgUpgradeAsk.cpp \
	view/DlgUpgradeAsk.h \
	view/MainFrameBase.cpp \
	view/MainFrameBase.h \
	view/MainFrame.cpp \
	view/MainFrame.h \
	view/ModAlfMainPanelImpl.cpp \
	view/ModAlfMainPanelImpl.h \
	view/ModAlfOverviewPanelImpl.cpp \
	view/ModAlfOverviewPanelImpl.h \
	view/ModAlfPanelsBase.cpp \
	view/ModAlfPanelsBase.h \
	view/ModAnoubisMainPanelImpl.cpp \
	view/ModAnoubisMainPanelImpl.h \
	view/ModAnoubisOverviewPanelImpl.cpp \
	view/ModAnoubisOverviewPanelImpl.h \
	view/ModAnoubisPanelsBase.cpp \
	view/ModAnoubisPanelsBase.h \
	view/ModOverviewMainPanelImpl.cpp \
	view/ModOverviewMainPanelImpl.h \
	view/ModOverviewPanelsBase.cpp \
	view/ModOverviewPanelsBase.h \
	view/ModSfsDetailsDlg.cpp \
	view/ModSfsDetailsDlg.h \
	view/ModSfsListCtrl.cpp \
	view/ModSfsListCtrl.h \
	view/ModSfsMainPanelImpl.cpp \
	view/ModSfsMainPanelImpl.h \
	view/ModSfsOverviewPanelImpl.cpp \
	view/ModSfsOverviewPanelImpl.h \
	view/ModSfsPanelsBase.cpp \
	view/ModSfsPanelsBase.h \
	view/ModSbMainPanelImpl.cpp \
	view/ModSbMainPanelImpl.h \
	view/ModSbOverviewPanelImpl.cpp \
	view/ModSbOverviewPanelImpl.h \
	view/ModSbPanelsBase.cpp \
	view/ModSbPanelsBase.h \
	view/OverviewAttrProvider.cpp \
	view/OverviewAttrProvider.h \
	view/RuleWizard.cpp \
	view/RuleWizard.h \
	view/RuleWizardPage.cpp \
	view/RuleWizardPage.h \
	view/RuleWizardProgramPage.cpp \
	view/RuleWizardProgramPage.h \
	view/RuleWizardContextPage.cpp \
	view/RuleWizardContextPage.h \
	view/RuleWizardContextExceptionPage.cpp \
	view/RuleWizardContextExceptionPage.h \
	view/RuleWizardAlfOverwritePage.cpp \
	view/RuleWizardAlfOverwritePage.h \
	view/RuleWizardAlfClientPage.cpp \
	view/RuleWizardAlfClientPage.h \
	view/RuleWizardAlfClientPortsPage.cpp \
	view/RuleWizardAlfClientPortsPage.h \
	view/RuleWizardAlfDlgAddService.cpp \
	view/RuleWizardAlfDlgAddService.h \
	view/RuleWizardSandboxPage.cpp \
	view/RuleWizardSandboxPage.h \
	view/RuleWizardSandboxOverwritePage.cpp \
	view/RuleWizardSandboxOverwritePage.h \
	view/RuleWizardSandboxExecutePage.cpp \
	view/RuleWizardSandboxExecutePage.h \
	view/RuleWizardSandboxExecuteFilesPage.cpp \
	view/RuleWizardSandboxExecuteFilesPage.h \
	view/RuleWizardSandboxReadPage.cpp \
	view/RuleWizardSandboxReadPage.h \
	view/RuleWizardSandboxReadFilesPage.cpp \
	view/RuleWizardSandboxReadFilesPage.h \
	view/RuleWizardSandboxWritePage.cpp \
	view/RuleWizardSandboxWritePage.h \
	view/RuleWizardSandboxWriteFilesPage.cpp \
	view/RuleWizardSandboxWriteFilesPage.h \
	view/RuleWizardFinalPage.cpp \
	view/RuleWizardFinalPage.h \
	view/RuleWizardHistory.cpp \
	view/RuleWizardHistory.h \
	view/RuleWizardPanelsBase.cpp \
	view/RuleWizardPanelsBase.h \
	view/TrayIcon.cpp \
	view/TrayIcon.h \
	view/VersionListCtrl.h \
	view/VersionListCtrl.cpp \
	control/AnConfig.cpp \
	control/AnConfig.h \
	control/AnShortcuts.cpp \
	control/AnShortcuts.h \
	control/AnEvents.cpp \
	control/AnEvents.h \
	control/Debug.h \
	control/Debug.cpp \
	control/KeyCtrl.cpp \
	control/KeyCtrl.h \
	control/RuleEditorAddPolicyVisitor.cpp \
	control/RuleEditorAddPolicyVisitor.h \
	control/ModSbAddPolicyVisitor.cpp \
	control/ModSbAddPolicyVisitor.h \
	control/ModSfsAddPolicyVisitor.cpp \
	control/ModSfsAddPolicyVisitor.h \
	control/NotificationCtrl.cpp \
	control/NotificationCtrl.h \
	control/RuleSetClearModifiedVisitor.cpp \
	control/RuleSetClearModifiedVisitor.h \
	control/RuleSetSearchPolicyVisitor.cpp \
	control/RuleSetSearchPolicyVisitor.h \
	control/SfsCtrl.cpp \
	control/SfsCtrl.h \
	control/SimpleOverviewVisitor.cpp \
	control/SimpleOverviewVisitor.h \
	control/VersionCtrl.cpp \
	control/VersionCtrl.h \
	control/PolicyCtrl.cpp \
	control/PolicyCtrl.h \
	control/ProcCtrl.cpp \
	control/ProcCtrl.h \
	pattern/PolicyVisitor.cpp \
	pattern/PolicyVisitor.h \
	pattern/Singleton.cpp \
	pattern/Singleton.h \
	pattern/SynchronizedQueue.h \
	pattern/Subject.cpp \
	pattern/Subject.h \
	pattern/Observer.cpp \
	pattern/Observer.h \
	widgets/AnDetails.cpp \
	widgets/AnDetails.h \
	widgets/AnIconList.cpp \
	widgets/AnIconList.h \
	widgets/AnListClass.h \
	widgets/AnListClassProperty.cpp \
	widgets/AnListClassProperty.h \
	widgets/AnListColumn.cpp \
	widgets/AnListColumn.h \
	widgets/AnListCtrl.cpp \
	widgets/AnListCtrl.h \
	widgets/AnListProperty.h \
	widgets/AnMessageDialog.cpp \
	widgets/AnMessageDialog.h \
	widgets/AnPolicyNotebook.cpp \
	widgets/AnPolicyNotebook.h \
	widgets/AnPickFromFs.cpp \
	widgets/AnPickFromFs.h \
	widgets/AnStatusBar.cpp \
	widgets/AnStatusBar.h \
	job/ComCsumAddTask.cpp \
	job/ComCsumAddTask.h \
	job/ComCsumDelTask.cpp \
	job/ComCsumDelTask.h \
	job/ComCsumGetTask.cpp \
	job/ComCsumGetTask.h \
	job/ComCsumHandler.cpp \
	job/ComCsumHandler.h \
	job/ComPolicyRequestTask.cpp \
	job/ComPolicyRequestTask.h \
	job/ComPolicySendTask.cpp \
	job/ComPolicySendTask.h \
	job/ComRegistrationTask.cpp \
	job/ComRegistrationTask.h \
	job/ComSfsListTask.cpp \
	job/ComSfsListTask.h \
	job/DummyTask.h \
	job/DummyTask.cpp \
	job/ComTask.cpp \
	job/ComTask.h \
	job/ComThread.cpp \
	job/ComThread.h \
	job/ComVersionTask.cpp \
	job/ComVersionTask.h \
	job/CsumCalcTask.cpp \
	job/CsumCalcTask.h \
	job/CsumCalcThread.cpp \
	job/CsumCalcThread.h \
	job/JobCtrl.cpp \
	job/JobCtrl.h \
	job/JobThread.cpp \
	job/JobThread.h \
	job/Task.cpp \
	job/Task.h \
	job/TaskEvent.cpp \
	job/TaskEvent.h \
	ctassert.h \
	main.cpp \
	main.h \
	libapn.h

#
# For unit tests of the components of xanoubis, we build a shared library.
# This library is for testing purpose only. We tear apart the unittest and
# the test candidate. Thus we are able to run tests and baseline-tests
# propperly.
# To reduce the compile time and to test what's in the final product, we
# reuse the already compiled objects of xanoubis. Ensure those are
# built with -fPIC.
#
if WITH_TESTS
noinst_PROGRAMS = libGUIcomponents.so
libGUIcomponents_so_SOURCES =
libGUIcomponents_so_DEPENDENCIES = $(xanoubis_OBJECTS)
libGUIcomponents_so_LINK = \
	$(CXX) -fstack-protector -shared -Wl,-soname,$@ \
	    -o $@ $(xanoubis_OBJECTS)
endif

i18n-extract:
	cd $(top_srcdir)/$(xanoubisdir) && \
	mkdir -p catalogs && \
	$(XGETTEXT) -C -k_ -kwxPLURAL:1,2 -kwxTRANSLATE \
	    -o catalogs/xanoubis.pot $(xanoubis_SOURCES)

i18n-merge:
	cd $(top_srcdir)/$(xanoubisdir) && \
	for po in catalogs/*/*.po; do \
	echo "Merging $$po"; \
	$(MSGMERGE) -Uv --backup=off $$po catalogs/xanoubis.pot; \
	done

i18n-format:
	cd $(top_srcdir)/$(xanoubisdir) && \
	mkdir -p catalogs && \
	for po in catalogs/*/*.po; do \
	mo=`echo $$po | sed -e "s/\.po$\/\.mo/"`; \
	echo "Compiling $$mo"; \
	$(MSGFMT) -vc -o $$mo $$po; \
	done

i18n-install: i18n-check
	cd $(top_srcdir)/$(xanoubisdir) && \
	for mo in `ls -1 catalogs/*/xanoubis.mo 2>/dev/null`; do \
	bn=`basename $$mo`; \
	dn=`dirname $$mo`; \
	dest=$(DESTDIR)$(programdir)/$$dn/$$bn; \
	$(install_sh_DATA) $$mo $$dest; \
	done

i18n-info:
	@echo "*** Targets ***"; \
	echo "i18n-extract"; \
	echo "     Scans sources and writes result (untranslated strings)"; \
	echo "     into template (catalogs/xanoubis.pot)."; \
	echo "i18n-merge"; \
	echo "     Merges template with already existing translations."; \
	echo "i18n-format"; \
	echo "     Converts translations into binary format."; \
	echo "*** Usage ***"; \
	echo "You need to call all three targets to update the translation!"; \
	echo "*** Creating new language ***"; \
	echo "cd <xanoubis-src-dir>/catalogs"; \
	echo "mkdir <lang-code>"; \
	echo "msginit -o <lang-code>/xanoubis.po -l <lang-code>"

i18n-check:
	cd $(top_srcdir)/$(xanoubisdir) && \
	for po in catalogs/*/*.po; do \
	echo "Checking $$po"; \
	[ "`msggrep -T -e ".*" -v -o- $$po;`xxx" = "xxx" ] || exit 1; \
	done

flawfinder_FLAGS=\
	--dataonly \
	--quiet \
	--allowlink \
	--minlevel=1 \
	--context

# bsd lint and splint do not support C++ at all.
splint lint:
flawfinder: flawfinder_report

include $(top_srcdir)/lint/Makefile.lint

flawfinder_report: $(xanoubis_SOURCES)
	@sh $(top_builddir)/lint/flawfinder $(DEBUG) -o $@ -- \
		$(flawfinder_FLAGS) $^

clean-local:
	@rm -f $(xanoubis_SOURCES_lint)
	@rm -f $(xanoubis_SOURCES_splint)
	@rm -f $(xanoubis_SOURCES_flawfinder)
	@rm -f flawfinder_report
