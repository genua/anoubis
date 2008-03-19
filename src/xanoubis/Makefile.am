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

bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
xanoubisdir    = src/xanoubis
anoubischatdir = src/libanoubischat
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
anoubisprotocoldir = src/libanoubisprotocol
libanoubisprotocol = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.a

xanoubis_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view  \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	$(WX_CPPFLAGS) $(AM_CPPFLAGS)

xanoubis_CXXFLAGS = -fno-strict-aliasing $(AM_CXXFLAGS) $(WX_CXXFLAGS)

xanoubis_LDADD = \
	@WX_LIBS@ \
	$(libanoubischat) \
	$(libanoubisprotocol)

if NEEDBSDCOMPAT
xanoubis_LDADD += $(libbsdcompat)
endif

icondir = $(datadir)/$(bin_PROGRAMS)/icons
dist_icon_DATA = \
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
	icons/ModSfs_question_48.png

xanoubis_SOURCES= \
	model/AlertNotify.cpp \
	model/AlertNotify.h \
	model/EscalationNotify.cpp \
	model/EscalationNotify.h \
	model/LogNotify.cpp \
	model/LogNotify.h \
	model/ModAlf.cpp \
	model/ModAlf.h \
	model/ModAnoubis.cpp \
	model/ModAnoubis.h \
	model/ModOverview.cpp \
	model/ModOverview.h \
	model/ModSfs.cpp \
	model/ModSfs.h \
	model/Module.cpp \
	model/Module.h \
	model/Notification.cpp \
	model/Notification.h \
	model/NotifyAnswer.cpp \
	model/NotifyAnswer.h \
	view/AnStatusBar.cpp \
	view/AnStatusBar.h \
	view/AnFader.cpp \
	view/AnFader.h \
	view/DlgLogViewerBase.cpp \
	view/DlgLogViewerBase.h \
	view/DlgLogViewer.cpp \
	view/DlgLogViewer.h \
	view/DlgRuleEditorBase.cpp \
	view/DlgRuleEditorBase.h \
	view/DlgRuleEditor.cpp \
	view/DlgRuleEditor.h \
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
	view/ModSfsMainPanelImpl.cpp \
	view/ModSfsMainPanelImpl.h \
	view/ModSfsOverviewPanelImpl.cpp \
	view/ModSfsOverviewPanelImpl.h \
	view/ModSfsPanelsBase.cpp \
	view/ModSfsPanelsBase.h \
	view/TrayIcon.cpp \
	view/TrayIcon.h \
	control/AnShortcuts.cpp \
	control/AnShortcuts.h \
	control/AnEvents.cpp \
	control/AnEvents.h \
	control/CommunicatorCtrl.cpp \
	control/CommunicatorCtrl.h \
	control/Communicator.cpp \
	control/Communicator.h \
	ctassert.h \
	main.cpp \
	main.h

xanoubis_SOURCES_flawfinder= \
	model/AlertNotify.cpp_flawfinder \
	model/AlertNotify.h_flawfinder \
	model/EscalationNotify.cpp_flawfinder \
	model/EscalationNotify.h_flawfinder \
	model/LogNotify.cpp_flawfinder \
	model/LogNotify.h_flawfinder \
	model/ModAlf.cpp_flawfinder \
	model/ModAlf.h_flawfinder \
	model/ModAnoubis.cpp_flawfinder \
	model/ModAnoubis.h_flawfinder \
	model/ModOverview.cpp_flawfinder \
	model/ModOverview.h_flawfinder \
	model/ModSfs.cpp_flawfinder \
	model/ModSfs.h_flawfinder \
	model/Module.cpp_flawfinder \
	model/Module.h_flawfinder \
	model/Notification.cpp_flawfinder \
	model/Notification.h_flawfinder \
	model/NotifyAnswer.cpp_flawfinder \
	model/NotifyAnswer.h_flawfinder \
	view/AnStatusBar.cpp_flawfinder \
	view/AnStatusBar.h_flawfinder \
	view/DlgLogViewerBase.cpp_flawfinder \
	view/DlgLogViewerBase.h_flawfinder \
	view/DlgLogViewer.cpp_flawfinder \
	view/DlgLogViewer.h_flawfinder \
	view/DlgRuleEditorBase.cpp_flawfinder \
	view/DlgRuleEditorBase.h_flawfinder \
	view/DlgRuleEditor.cpp_flawfinder \
	view/DlgRuleEditor.h_flawfinder \
	view/MainFrameBase.cpp_flawfinder \
	view/MainFrameBase.h_flawfinder \
	view/MainFrame.cpp_flawfinder \
	view/MainFrame.h_flawfinder \
	view/ModAlfMainPanelImpl.cpp_flawfinder \
	view/ModAlfMainPanelImpl.h_flawfinder \
	view/ModAlfOverviewPanelImpl.cpp_flawfinder \
	view/ModAlfOverviewPanelImpl.h_flawfinder \
	view/ModAlfPanelsBase.cpp_flawfinder \
	view/ModAlfPanelsBase.h_flawfinder \
	view/ModAnoubisMainPanelImpl.cpp_flawfinder \
	view/ModAnoubisMainPanelImpl.h_flawfinder \
	view/ModAnoubisOverviewPanelImpl.cpp_flawfinder \
	view/ModAnoubisOverviewPanelImpl.h_flawfinder \
	view/ModAnoubisPanelsBase.cpp_flawfinder \
	view/ModAnoubisPanelsBase.h_flawfinder \
	view/ModOverviewMainPanelImpl.cpp_flawfinder \
	view/ModOverviewMainPanelImpl.h_flawfinder \
	view/ModOverviewPanelsBase.cpp_flawfinder \
	view/ModOverviewPanelsBase.h_flawfinder \
	view/ModSfsMainPanelImpl.cpp_flawfinder \
	view/ModSfsMainPanelImpl.h_flawfinder \
	view/ModSfsOverviewPanelImpl.cpp_flawfinder \
	view/ModSfsOverviewPanelImpl.h_flawfinder \
	view/ModSfsPanelsBase.cpp_flawfinder \
	view/ModSfsPanelsBase.h_flawfinder \
	view/TrayIcon.cpp_flawfinder \
	view/TrayIcon.h_flawfinder \
	control/AnShortcuts.cpp_flawfinder \
	control/AnShortcuts.h_flawfinder \
	control/AnEvents.cpp_flawfinder \
	control/AnEvents.h_flawfinder \
	control/CommunicatorCtrl.cpp_flawfinder \
	control/CommunicatorCtrl.h_flawfinder \
	control/Communicator.cpp_flawfinder \
	control/Communicator.h_flawfinder \
	ctassert.h_flawfinder \
	main.cpp_flawfinder \
	main.h_flawfinder

xanoubis_SOURCES_lint= \
	model/AlertNotify.cpp_lint \
	model/AlertNotify.h_lint \
	model/EscalationNotify.cpp_lint \
	model/EscalationNotify.h_lint \
	model/LogNotify.cpp_lint \
	model/LogNotify.h_lint \
	model/ModAlf.cpp_lint \
	model/ModAlf.h_lint \
	model/ModAnoubis.cpp_lint \
	model/ModAnoubis.h_lint \
	model/ModOverview.cpp_lint \
	model/ModOverview.h_lint \
	model/ModSfs.cpp_lint \
	model/ModSfs.h_lint \
	model/Module.cpp_lint \
	model/Module.h_lint \
	model/Notification.cpp_lint \
	model/Notification.h_lint \
	model/NotifyAnswer.cpp_lint \
	model/NotifyAnswer.h_lint \
	view/AnStatusBar.cpp_lint \
	view/AnStatusBar.h_lint \
	view/DlgLogViewerBase.cpp_lint \
	view/DlgLogViewerBase.h_lint \
	view/DlgLogViewer.cpp_lint \
	view/DlgLogViewer.h_lint \
	view/DlgRuleEditorBase.cpp_lint \
	view/DlgRuleEditorBase.h_lint \
	view/DlgRuleEditor.cpp_lint \
	view/DlgRuleEditor.h_lint \
	view/MainFrameBase.cpp_lint \
	view/MainFrameBase.h_lint \
	view/MainFrame.cpp_lint \
	view/MainFrame.h_lint \
	view/ModAlfMainPanelImpl.cpp_lint \
	view/ModAlfMainPanelImpl.h_lint \
	view/ModAlfOverviewPanelImpl.cpp_lint \
	view/ModAlfOverviewPanelImpl.h_lint \
	view/ModAlfPanelsBase.cpp_lint \
	view/ModAlfPanelsBase.h_lint \
	view/ModAnoubisMainPanelImpl.cpp_lint \
	view/ModAnoubisMainPanelImpl.h_lint \
	view/ModAnoubisOverviewPanelImpl.cpp_lint \
	view/ModAnoubisOverviewPanelImpl.h_lint \
	view/ModAnoubisPanelsBase.cpp_lint \
	view/ModAnoubisPanelsBase.h_lint \
	view/ModOverviewMainPanelImpl.cpp_lint \
	view/ModOverviewMainPanelImpl.h_lint \
	view/ModOverviewPanelsBase.cpp_lint \
	view/ModOverviewPanelsBase.h_lint \
	view/ModSfsMainPanelImpl.cpp_lint \
	view/ModSfsMainPanelImpl.h_lint \
	view/ModSfsOverviewPanelImpl.cpp_lint \
	view/ModSfsOverviewPanelImpl.h_lint \
	view/ModSfsPanelsBase.cpp_lint \
	view/ModSfsPanelsBase.h_lint \
	view/TrayIcon.cpp_lint \
	view/TrayIcon.h_lint \
	control/AnShortcuts.cpp_lint \
	control/AnShortcuts.h_lint \
	control/AnEvents.cpp_lint \
	control/AnEvents.h_lint \
	control/CommunicatorCtrl.cpp_lint \
	control/CommunicatorCtrl.h_lint \
	control/Communicator.cpp_lint \
	control/Communicator.h_lint \
	ctassert.h_lint \
	main.cpp_lint \
	main.h_lint

# splint does not support C++ at all.
xanoubis_SOURCES_splint=

flawfinder_FLAGS=
lint_FLAGS=$(xanoubis_CPPFLAGS)

mkdirs:
	@mkdir model view control

.PHONY: mkdirs

# XXX RT: C++ code needs extra change. See #257
flawfinder splint lint:
	@echo "linting temporarily disabled for xanoubis"

#flawfinder:	$(xanoubis_SOURCES_flawfinder)
#splint:		$(xanoubis_SOURCES_splint)
#lint:		$(xanoubis_SOURCES_lint)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(xanoubis_SOURCES_lint)
	@rm -f $(xanoubis_SOURCES_splint)
	@rm -f $(xanoubis_SOURCES_flawfinder)
