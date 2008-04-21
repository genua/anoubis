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
	-I$(top_builddir)/src \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view  \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	$(WX_CPPFLAGS) $(GTK_CFLAGS) $(GLIB_CFLAGS) $(NOTIFY_CFLAGS) \
	$(AM_CPPFLAGS)

xanoubis_CXXFLAGS = \
	-fno-strict-aliasing $(AM_CXXFLAGS) $(WX_CXXFLAGS)

xanoubis_LDADD = \
	@WX_LIBS@ \
	@GTK_LIBS@ \
	@GLIB_LIBS@ \
	@NOTIFY_LIBS@ \
	$(libanoubischat) \
	$(libanoubisprotocol)

if NEEDBSDCOMPAT
xanoubis_LDADD += $(libbsdcompat)
xanoubis_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
endif

xanoubis_DEPENDENCIES = $(top_srcdir)/$(xanoubisdir)/catalogs/de/xanoubis.mo

catalogdir = $(datadir)/$(bin_PROGRAMS)/catalogs/de
utildir = $(datadir)/$(bin_PROGRAMS)/utils
icondir = $(datadir)/$(bin_PROGRAMS)/icons

dist_catalog_DATA = catalogs/de/xanoubis.mo
dist_util_DATA = utils/xanoubis_guessApp.sh
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

# XXX by ch: the target for po files is experimental and
# will not been done automatically
%.po:
	( \
	  cd $(top_srcdir)/$(xanoubisdir); \
	  rm -f $@; \
	  xgettext -L C++ -k_ -o $@ $(xanoubis_SOURCES); \
	)

%.mo: %.po
	msgfmt -o $@ $<

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
