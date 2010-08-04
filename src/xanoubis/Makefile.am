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
noinst_DATA = potfiles.frag

CLEANFILES = $(noinst_DATA)

include $(top_srcdir)/src/Makefile.include

xanoubisdir    = src/xanoubis

xanoubis_CPPFLAGS = \
	$(anoubis_cppflags) \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view  \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	-I$(top_srcdir)/$(xanoubisdir)/widgets \
	-I$(top_srcdir)/$(xanoubisdir)/pattern \
	-I$(top_srcdir)/$(xanoubisdir)/job \
	$(WX_CPPFLAGS)

xanoubis_cxxflags = \
	-fno-strict-aliasing \
	$(WX_CXXFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NOTIFY_CFLAGS) \
	$(SSL_CFLAGS)

xanoubis_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	$(xanoubis_cxxflags)

xanoubis_LDFLAGS = $(AM_LDFLAGS) $(SSL_LDFLAGS)

xanoubis_ldadd = \
	@WX_LIBS@ \
	@GTK_LIBS@ \
	@GLIB_LIBS@ \
	@NOTIFY_LIBS@ \
	$(SSL_LIBS) \
	-lfl

xanoubis_LDADD = $(xanoubis_ldadd) $(libanoubis)

programdir = $(datadir)/$(bin_PROGRAMS)
utildir = $(datadir)/$(bin_PROGRAMS)/utils
icondir = $(datadir)/$(bin_PROGRAMS)/icons
profiledir = $(datadir)/$(bin_PROGRAMS)/profiles
wizarddir = $(profiledir)/wizard
desktopdir = $(datadir)/$(bin_PROGRAMS)

dist_util_SCRIPTS = utils/xanoubis_guessApp.sh
dist_icon_DATA = \
	icons/General_alert_16.png \
	icons/General_alert_48.png \
	icons/General_downarrow_16.png \
	icons/General_error_16.png \
	icons/General_error_48.png \
	icons/General_ok_16.png \
	icons/General_ok_48.png \
	icons/General_problem_16.png \
	icons/General_problem_48.png \
	icons/General_question_16.png \
	icons/General_question_48.png \
	icons/General_rightarrow_16.png \
	icons/General_symlink_16.png \
	icons/General_symlink_48.png \
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
	icons/ModAnoubis_alert_20.png \
	icons/ModAnoubis_alert_24.png \
	icons/ModAnoubis_alert_32.png \
	icons/ModAnoubis_alert_48.png \
	icons/ModAnoubis_black_16.png \
	icons/ModAnoubis_black_20.png \
	icons/ModAnoubis_black_24.png \
	icons/ModAnoubis_black_32.png \
	icons/ModAnoubis_black_48.png \
	icons/ModAnoubis_error_16.png \
	icons/ModAnoubis_error_20.png \
	icons/ModAnoubis_error_24.png \
	icons/ModAnoubis_error_32.png \
	icons/ModAnoubis_error_48.png \
	icons/ModAnoubis_grey_16.png \
	icons/ModAnoubis_grey_20.png \
	icons/ModAnoubis_grey_24.png \
	icons/ModAnoubis_grey_32.png \
	icons/ModAnoubis_grey_48.png \
	icons/ModAnoubis_ok_16.png \
	icons/ModAnoubis_ok_20.png \
	icons/ModAnoubis_ok_24.png \
	icons/ModAnoubis_ok_32.png \
	icons/ModAnoubis_ok_48.png \
	icons/ModAnoubis_problem_16.png \
	icons/ModAnoubis_problem_20.png \
	icons/ModAnoubis_problem_24.png \
	icons/ModAnoubis_problem_32.png \
	icons/ModAnoubis_problem_48.png \
	icons/ModAnoubis_question_16.png \
	icons/ModAnoubis_question_20.png \
	icons/ModAnoubis_question_24.png \
	icons/ModAnoubis_question_32.png \
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
	icons/ModPlayground_alert_16.png \
	icons/ModPlayground_alert_48.png \
	icons/ModPlayground_black_16.png \
	icons/ModPlayground_black_48.png \
	icons/ModPlayground_error_16.png \
	icons/ModPlayground_error_48.png \
	icons/ModPlayground_ok_16.png \
	icons/ModPlayground_ok_48.png \
	icons/ModPlayground_problem_16.png \
	icons/ModPlayground_problem_48.png \
	icons/ModPlayground_question_16.png \
	icons/ModPlayground_question_48.png \
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

install-data-local:

xanoubis_SOURCES= \
	model/AlertNotify.cpp \
	model/AlertNotify.h \
	model/AlfAppPolicy.cpp \
	model/AlfAppPolicy.h \
	model/AlfCapabilityFilterPolicy.cpp \
	model/AlfCapabilityFilterPolicy.h \
	model/AlfFilterPolicy.cpp \
	model/AlfFilterPolicy.h \
	model/AlfOverviewTable.cpp \
	model/AlfOverviewTable.h \
	model/ApnVersion.cpp \
	model/ApnVersion.h \
	model/AppPolicy.cpp \
	model/AppPolicy.h \
	model/ContextAppPolicy.cpp \
	model/ContextAppPolicy.h \
	model/ContextFilterPolicy.cpp \
	model/ContextFilterPolicy.h \
	model/DaemonAnswerNotify.cpp \
	model/DaemonAnswerNotify.h \
	model/DefaultConversions.h \
	model/DefaultFilterPolicy.cpp \
	model/DefaultFilterPolicy.h \
	model/EscalationNotify.cpp \
	model/EscalationNotify.h \
	model/FilterPolicy.cpp \
	model/FilterPolicy.h \
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
	model/ModPlayground.cpp \
	model/ModPlayground.h \
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
	model/PlaygroundFileEntry.cpp \
	model/PlaygroundFileEntry.h \
	model/PlaygroundInfoEntry.cpp \
	model/PlaygroundInfoEntry.h \
	model/Policy.cpp \
	model/Policy.h \
	model/PolicyChecks.cpp \
	model/PolicyChecks.h \
	model/PolicyRowProvider.cpp \
	model/PolicyRowProvider.h \
	model/PolicyRuleSet.cpp \
	model/PolicyRuleSet.h \
	model/PolicyUtils.cpp \
	model/PolicyUtils.h \
	model/PrivKey.cpp \
	model/PrivKey.h \
	model/Profile.cpp \
	model/Profile.h \
	model/SbAccessFilterPolicy.cpp \
	model/SbAccessFilterPolicy.h \
	model/SbAppPolicy.cpp \
	model/SbAppPolicy.h \
	model/SbEntry.cpp \
	model/SbEntry.h \
	model/SbModel.cpp \
	model/SbModel.h \
	model/SbModelRowProvider.cpp \
	model/SbModelRowProvider.h \
	model/SbOverviewTable.cpp \
	model/SbOverviewTable.h \
	model/Service.cpp \
	model/Service.h \
	model/ServiceList.cpp \
	model/ServiceList.h \
	model/SfsAppPolicy.cpp \
	model/SfsAppPolicy.h \
	model/SfsDefaultFilterPolicy.cpp \
	model/SfsDefaultFilterPolicy.h \
	model/SfsDirectory.cpp \
	model/SfsDirectory.h \
	model/SfsEntry.cpp \
	model/SfsEntry.h \
	model/SfsFilterPolicy.cpp \
	model/SfsFilterPolicy.h \
	model/SfsOverviewTable.cpp \
	model/SfsOverviewTable.h \
	model/SimpleOverviewRow.cpp \
	model/SimpleOverviewRow.h \
	model/SimpleOverviewTable.cpp \
	model/SimpleOverviewTable.h \
	model/StatusNotify.cpp \
	model/StatusNotify.h \
	model/StringListModel.cpp \
	model/StringListModel.h \
	view/DlgBackupPolicyBase.cpp \
	view/DlgBackupPolicyBase.h \
	view/DlgBackupPolicyImpl.cpp \
	view/DlgBackupPolicyImpl.h \
	view/DlgLogViewer.cpp \
	view/DlgLogViewer.h \
	view/DlgLogViewerBase.cpp \
	view/DlgLogViewerBase.h \
	view/DlgPlaygroundCommitFileListImpl.cpp \
	view/DlgPlaygroundCommitFileListImpl.h \
	view/DlgProfileSelection.cpp \
	view/DlgProfileSelection.h \
	view/DlgRuleEditor.cpp \
	view/DlgRuleEditor.h \
	view/DlgRuleEditorAppPage.cpp \
	view/DlgRuleEditorAppPage.h \
	view/DlgRuleEditorAttrProvider.cpp \
	view/DlgRuleEditorAttrProvider.h \
	view/DlgRuleEditorBase.cpp \
	view/DlgRuleEditorBase.h \
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
	view/DlgRuleEditorListProperty.cpp \
	view/DlgRuleEditorListProperty.h \
	view/DlgRuleEditorPage.cpp \
	view/DlgRuleEditorPage.h \
	view/DlgUpgradeAsk.cpp \
	view/DlgUpgradeAsk.h \
	view/MainFrame.cpp \
	view/MainFrame.h \
	view/MainFrameBase.cpp \
	view/MainFrameBase.h \
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
	view/ModPlaygroundMainPanelImpl.cpp \
	view/ModPlaygroundMainPanelImpl.h \
	view/ModPlaygroundOverviewPanelImpl.cpp \
	view/ModPlaygroundOverviewPanelImpl.h \
	view/ModPlaygroundPanelsBase.cpp \
	view/ModPlaygroundPanelsBase.h \
	view/ModPlaygroundRowProperty.cpp \
	view/ModPlaygroundRowProperty.h \
	view/ModSbMainPanelImpl.cpp \
	view/ModSbMainPanelImpl.h \
	view/ModSbOverviewPanelImpl.cpp \
	view/ModSbOverviewPanelImpl.h \
	view/ModSbPanelsBase.cpp \
	view/ModSbPanelsBase.h \
	view/ModSfsDetailsDlg.cpp \
	view/ModSfsDetailsDlg.h \
	view/ModSfsGenerateKeyDlg.cpp \
	view/ModSfsGenerateKeyDlg.h \
	view/ModSfsListCtrl.cpp \
	view/ModSfsListCtrl.h \
	view/ModSfsListProperty.cpp \
	view/ModSfsListProperty.h \
	view/ModSfsMainPanelImpl.cpp \
	view/ModSfsMainPanelImpl.h \
	view/ModSfsOverviewPanelImpl.cpp \
	view/ModSfsOverviewPanelImpl.h \
	view/ModSfsPanelsBase.cpp \
	view/ModSfsPanelsBase.h \
	view/OverviewAttrProvider.cpp \
	view/OverviewAttrProvider.h \
	view/PlaygroundListProperty.cpp \
	view/PlaygroundListProperty.h \
	view/ProfileListCtrl.cpp \
	view/ProfileListCtrl.h \
	view/RuleWizard.cpp \
	view/RuleWizard.h \
	view/RuleWizardAlfClientPage.cpp \
	view/RuleWizardAlfClientPage.h \
	view/RuleWizardAlfClientPortsPage.cpp \
	view/RuleWizardAlfClientPortsPage.h \
	view/RuleWizardAlfDlgAddService.cpp \
	view/RuleWizardAlfDlgAddService.h \
	view/RuleWizardAlfOverwritePage.cpp \
	view/RuleWizardAlfOverwritePage.h \
	view/RuleWizardContextExceptionPage.cpp \
	view/RuleWizardContextExceptionPage.h \
	view/RuleWizardContextPage.cpp \
	view/RuleWizardContextPage.h \
	view/RuleWizardFinalPage.cpp \
	view/RuleWizardFinalPage.h \
	view/RuleWizardHistory.cpp \
	view/RuleWizardHistory.h \
	view/RuleWizardPage.cpp \
	view/RuleWizardPage.h \
	view/RuleWizardPanelsBase.cpp \
	view/RuleWizardPanelsBase.h \
	view/RuleWizardProgramPage.cpp \
	view/RuleWizardProgramPage.h \
	view/RuleWizardSandboxExecuteFilesPage.cpp \
	view/RuleWizardSandboxExecuteFilesPage.h \
	view/RuleWizardSandboxExecutePage.cpp \
	view/RuleWizardSandboxExecutePage.h \
	view/RuleWizardSandboxOverwritePage.cpp \
	view/RuleWizardSandboxOverwritePage.h \
	view/RuleWizardSandboxPage.cpp \
	view/RuleWizardSandboxPage.h \
	view/RuleWizardSandboxProperty.cpp \
	view/RuleWizardSandboxProperty.h \
	view/RuleWizardSandboxReadFilesPage.cpp \
	view/RuleWizardSandboxReadFilesPage.h \
	view/RuleWizardSandboxReadPage.cpp \
	view/RuleWizardSandboxReadPage.h \
	view/RuleWizardSandboxWriteFilesPage.cpp \
	view/RuleWizardSandboxWriteFilesPage.h \
	view/RuleWizardSandboxWritePage.cpp \
	view/RuleWizardSandboxWritePage.h \
	view/SfsOverviewAttrProvider.cpp \
	view/SfsOverviewAttrProvider.h \
	view/TrayIcon.cpp \
	view/TrayIcon.h \
	view/VersionListCtrl.cpp \
	view/VersionListCtrl.h \
	control/AnConfig.cpp \
	control/AnConfig.h \
	control/AnEvents.cpp \
	control/AnEvents.h \
	control/Debug.cpp \
	control/Debug.h \
	control/KeyCtrl.cpp \
	control/KeyCtrl.h \
	control/MainUtils.cpp \
	control/MainUtils.h \
	control/NotificationCtrl.cpp \
	control/NotificationCtrl.h \
	control/PlaygroundCtrl.cpp \
	control/PlaygroundCtrl.h \
	control/PolicyCtrl.cpp \
	control/PolicyCtrl.h \
	control/ProcCtrl.cpp \
	control/ProcCtrl.h \
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
	pattern/IndexTree.h \
	pattern/IndexTree_Impl.h \
	pattern/Observer.cpp \
	pattern/Observer.h \
	pattern/PolicyVisitor.cpp \
	pattern/PolicyVisitor.h \
	pattern/Singleton.cpp \
	pattern/Singleton.h \
	pattern/Subject.cpp \
	pattern/Subject.h \
	pattern/SynchronizedQueue.h \
	widgets/AnDetails.cpp \
	widgets/AnDetails.h \
	widgets/AnGenericRowProvider.cpp \
	widgets/AnGenericRowProvider.h \
	widgets/AnGrid.cpp \
	widgets/AnGrid.h \
	widgets/AnGridColumn.cpp \
	widgets/AnGridColumn.h \
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
	widgets/AnMultiRowProvider.cpp \
	widgets/AnMultiRowProvider.h \
	widgets/AnPickFromFs.cpp \
	widgets/AnPickFromFs.h \
	widgets/AnPolicyNotebook.cpp \
	widgets/AnPolicyNotebook.h \
	widgets/AnRowProvider.h \
	widgets/AnStatusBar.cpp \
	widgets/AnStatusBar.h \
	widgets/AnTable.cpp \
	widgets/AnTable.h \
	job/ComCSMultiTask.cpp \
	job/ComCSMultiTask.h \
	job/ComCsumAddTask.cpp \
	job/ComCsumAddTask.h \
	job/ComCsumDelTask.cpp \
	job/ComCsumDelTask.h \
	job/ComCsumGetTask.cpp \
	job/ComCsumGetTask.h \
	job/ComPolicyRequestTask.cpp \
	job/ComPolicyRequestTask.h \
	job/ComPolicySendTask.cpp \
	job/ComPolicySendTask.h \
	job/ComRegistrationTask.cpp \
	job/ComRegistrationTask.h \
	job/ComSfsListTask.cpp \
	job/ComSfsListTask.h \
	job/ComTask.cpp \
	job/ComTask.h \
	job/ComThread.cpp \
	job/ComThread.h \
	job/ComVersionTask.cpp \
	job/ComVersionTask.h \
	job/CsumCalcTask.cpp \
	job/CsumCalcTask.h \
	job/DummyTask.cpp \
	job/DummyTask.h \
	job/FsThread.cpp \
	job/FsThread.h \
	job/JobCtrl.cpp \
	job/JobCtrl.h \
	job/JobThread.cpp \
	job/JobThread.h \
	job/PlaygroundFilesTask.cpp \
	job/PlaygroundFilesTask.h \
	job/PlaygroundListTask.cpp \
	job/PlaygroundListTask.h \
	job/PlaygroundTask.cpp \
	job/PlaygroundTask.h \
	job/PlaygroundUnlinkTask.cpp \
	job/PlaygroundUnlinkTask.h \
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

if HAVE_LIBWXGUITESTING

# We want wxGuiTesting and for that we need to compile GUIcomponents
# with special compiler- and linker-flags
libGUIcomponents_so_SOURCES  = $(xanoubis_SOURCES)
libGUIcomponents_so_CPPFLAGS = $(xanoubis_CPPFLAGS) -DUSE_WXGUITESTING
libGUIcomponents_so_CXXFLAGS = $(shared_AM_CXXFLAGS) $(xanoubis_cxxflags)
libGUIcomponents_so_LDFLAGS  = $(shared_AM_LDFLAGS) $(SSL_LDFLAGS) \
	-Wl,-soname,$@
libGUIcomponents_so_LDADD    = $(xanoubis_ldadd) \
	-L $(libanoubisbuilddir) -lanoubis
libGUIcomponents_so_LINK     = $(CXX) $(libGUIcomponents_so_LDFLAGS) -o $@

else # ! HAVE_LIBWXGUITESTING
# No wxGuiTesting; maybe we can cheat and just link objects together
if FPIC_HACK
libGUIcomponents_so_SOURCES =
libGUIcomponents_so_DEPENDENCIES = $(xanoubis_OBJECTS)
libGUIcomponents_so_LINK = \
	$(CXX) $(shared_AM_LDFLAGS) -Wl,-soname,$@ \
	    -o $@ $(xanoubis_OBJECTS)

else # ! FPIC_HACK

libGUIcomponents_so_SOURCES  = $(xanoubis_SOURCES)
libGUIcomponents_so_CPPFLAGS = $(xanoubis_CPPFLAGS)
libGUIcomponents_so_CXXFLAGS = $(shared_AM_CXXFLAGS) $(xanoubis_cxxflags)
libGUIcomponents_so_LDFLAGS  = $(shared_AM_LDFLAGS) $(SSL_LDFLAGS) \
	-Wl,-soname,$@
libGUIcomponents_so_LDADD    = $(xanoubis_ldadd) \
	-L $(libanoubisbuilddir) -lanoubis
libGUIcomponents_so_LINK     = $(CXX) $(libGUIcomponents_so_LDFLAGS) -o $@

endif # ! FPIC_HACK
endif # ! HAVE_LIBWXGUITESTING
endif # WITH_TESTS

potfiles.frag:
	echo $(xanoubis_SOURCES) > potfiles.frag
	perl -pi -e 's/\s+/\n/gm' potfiles.frag
	perl -pi -e 's/^/src\/xanoubis\//' potfiles.frag

flawfinder_FLAGS=\
	--dataonly \
	--quiet \
	--allowlink \
	--minlevel=1 \
	--context

# bsd lint and splint do not support C++ at all.
sources_splint =
sources_lint =
sources_flawfinder = $(xanoubis_SOURCES:%=%_flawfinder)

include $(top_srcdir)/lint/Makefile.lint
