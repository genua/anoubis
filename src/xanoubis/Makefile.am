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

xanoubisdir    = src/xanoubis
anoubischatdir = src/libanoubischat
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.a

xanoubis_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view  \
	-I$(top_srcdir)/$(xanoubisdir)/control

xanoubis_CXXFLAGS = -fno-strict-aliasing

xanoubis_LDADD = \
	@WX_LIBS@ \
	$(libanoubischat)

icondir = $(datadir)/$(bin_PROGRAMS)/icons
dist_icon_DATA = \
	icons/ModALF.png \
	icons/ModAnoubis.png \
	icons/ModOverview.png \
	icons/tray.gif \
	icons/tray_msgByHand.png \
	icons/tray_normal.png

xanoubis_SOURCES= \
	control/AnShortcuts.cpp \
	control/AnShortcuts.h \
	control/Communicator.cpp \
	control/CommunicatorException.cpp \
	control/CommunicatorException.h \
	control/Communicator.h \
	view/AnStatusBar.cpp \
	view/AnStatusBar.h \
	view/DlgRuleEditor.cpp \
	view/DlgRuleEditor.h \
	view/DlgRuleEditorBase.cpp \
	view/DlgRuleEditorBase.h \
	view/MainFrameBase.cpp \
	view/MainFrameBase.h \
	view/MainFrame.cpp \
	view/MainFrame.h \
	view/ModAlf.cpp \
	view/ModAlf.h \
	view/ModAlfMainPanelImpl.cpp \
	view/ModAlfMainPanelImpl.h \
	view/ModAlfOverviewPanelImpl.cpp \
	view/ModAlfOverviewPanelImpl.h \
	view/ModAlfPanelsBase.cpp \
	view/ModAlfPanelsBase.h \
	view/ModAnoubis.cpp \
	view/ModAnoubis.h \
	view/ModAnoubisMainPanelImpl.cpp \
	view/ModAnoubisMainPanelImpl.h \
	view/ModAnoubisOverviewPanelImpl.cpp \
	view/ModAnoubisOverviewPanelImpl.h \
	view/ModAnoubisPanelsBase.cpp \
	view/ModAnoubisPanelsBase.h \
	view/ModOverview.cpp \
	view/ModOverview.h \
	view/ModOverviewMainPanelImpl.cpp \
	view/ModOverviewMainPanelImpl.h \
	view/ModOverviewPanelsBase.cpp \
	view/ModOverviewPanelsBase.h \
	view/Module.cpp \
	view/Module.h \
	view/TrayIcon.cpp \
	view/TrayIcon.h \
	ctassert.h \
	main.cpp \
	main.h

xanoubis_SOURCES_flawfinder= \
	control/AnShortcuts.cpp_flawfinder \
	control/AnShortcuts.h_flawfinder \
	control/Communicator.cpp_flawfinder \
	control/CommunicatorException.cpp_flawfinder \
	control/CommunicatorException.h_flawfinder \
	control/Communicator.h_flawfinder \
	view/AnStatusBar.cpp_flawfinder \
	view/AnStatusBar.h_flawfinder \
	view/DlgRuleEditor.cpp_flawfinder \
	view/DlgRuleEditor.h_flawfinder \
	view/DlgRuleEditorBase.cpp_flawfinder \
	view/DlgRuleEditorBase.h_flawfinder \
	view/MainFrameBase.cpp_flawfinder \
	view/MainFrameBase.h_flawfinder \
	view/MainFrame.cpp_flawfinder \
	view/MainFrame.h_flawfinder \
	view/ModAlf.cpp_flawfinder \
	view/ModAlf.h_flawfinder \
	view/ModAlfMainPanelImpl.cpp_flawfinder \
	view/ModAlfMainPanelImpl.h_flawfinder \
	view/ModAlfOverviewPanelImpl.cpp_flawfinder \
	view/ModAlfOverviewPanelImpl.h_flawfinder \
	view/ModAlfPanelsBase.cpp_flawfinder \
	view/ModAlfPanelsBase.h_flawfinder \
	view/ModAnoubis.cpp_flawfinder \
	view/ModAnoubis.h_flawfinder \
	view/ModAnoubisMainPanelImpl.cpp_flawfinder \
	view/ModAnoubisMainPanelImpl.h_flawfinder \
	view/ModAnoubisOverviewPanelImpl.cpp_flawfinder \
	view/ModAnoubisOverviewPanelImpl.h_flawfinder \
	view/ModAnoubisPanelsBase.cpp_flawfinder \
	view/ModAnoubisPanelsBase.h_flawfinder \
	view/ModOverview.cpp_flawfinder \
	view/ModOverview.h_flawfinder \
	view/ModOverviewMainPanelImpl.cpp_flawfinder \
	view/ModOverviewMainPanelImpl.h_flawfinder \
	view/ModOverviewPanelsBase.cpp_flawfinder \
	view/ModOverviewPanelsBase.h_flawfinder \
	view/Module.cpp_flawfinder \
	view/Module.h_flawfinder \
	view/TrayIcon.cpp_flawfinder \
	view/TrayIcon.h_flawfinder \
	ctassert.h_flawfinder \
	main.cpp_flawfinder \
	main.h_flawfinder

xanoubis_SOURCES_lint= \
	control/AnShortcuts.cpp_lint \
	control/AnShortcuts.h_lint \
	control/Communicator.cpp_lint \
	control/CommunicatorException.cpp_lint \
	control/CommunicatorException.h_lint \
	control/Communicator.h_lint \
	view/AnStatusBar.cpp_lint \
	view/AnStatusBar.h_lint \
	view/DlgRuleEditor.cpp_lint \
	view/DlgRuleEditor.h_lint \
	view/DlgRuleEditorBase.cpp_lint \
	view/DlgRuleEditorBase.h_lint \
	view/MainFrameBase.cpp_lint \
	view/MainFrameBase.h_lint \
	view/MainFrame.cpp_lint \
	view/MainFrame.h_lint \
	view/ModAlf.cpp_lint \
	view/ModAlf.h_lint \
	view/ModAlfMainPanelImpl.cpp_lint \
	view/ModAlfMainPanelImpl.h_lint \
	view/ModAlfOverviewPanelImpl.cpp_lint \
	view/ModAlfOverviewPanelImpl.h_lint \
	view/ModAlfPanelsBase.cpp_lint \
	view/ModAlfPanelsBase.h_lint \
	view/ModAnoubis.cpp_lint \
	view/ModAnoubis.h_lint \
	view/ModAnoubisMainPanelImpl.cpp_lint \
	view/ModAnoubisMainPanelImpl.h_lint \
	view/ModAnoubisOverviewPanelImpl.cpp_lint \
	view/ModAnoubisOverviewPanelImpl.h_lint \
	view/ModAnoubisPanelsBase.cpp_lint \
	view/ModAnoubisPanelsBase.h_lint \
	view/ModOverview.cpp_lint \
	view/ModOverview.h_lint \
	view/ModOverviewMainPanelImpl.cpp_lint \
	view/ModOverviewMainPanelImpl.h_lint \
	view/ModOverviewPanelsBase.cpp_lint \
	view/ModOverviewPanelsBase.h_lint \
	view/Module.cpp_lint \
	view/Module.h_lint \
	view/TrayIcon.cpp_lint \
	view/TrayIcon.h_lint \
	ctassert.h_lint \
	main.cpp_lint \
	main.h_lint

xanoubis_SOURCES_splint= \
	control/AnShortcuts.cpp_splint \
	control/AnShortcuts.h_splint \
	control/Communicator.cpp_splint \
	control/CommunicatorException.cpp_splint \
	control/CommunicatorException.h_splint \
	control/Communicator.h_splint \
	view/AnStatusBar.cpp_splint \
	view/AnStatusBar.h_splint \
	view/DlgRuleEditor.cpp_splint \
	view/DlgRuleEditor.h_splint \
	view/DlgRuleEditorBase.cpp_splint \
	view/DlgRuleEditorBase.h_splint \
	view/MainFrameBase.cpp_splint \
	view/MainFrameBase.h_splint \
	view/MainFrame.cpp_splint \
	view/MainFrame.h_splint \
	view/ModAlf.cpp_splint \
	view/ModAlf.h_splint \
	view/ModAlfMainPanelImpl.cpp_splint \
	view/ModAlfMainPanelImpl.h_splint \
	view/ModAlfOverviewPanelImpl.cpp_splint \
	view/ModAlfOverviewPanelImpl.h_splint \
	view/ModAlfPanelsBase.cpp_splint \
	view/ModAlfPanelsBase.h_splint \
	view/ModAnoubis.cpp_splint \
	view/ModAnoubis.h_splint \
	view/ModAnoubisMainPanelImpl.cpp_splint \
	view/ModAnoubisMainPanelImpl.h_splint \
	view/ModAnoubisOverviewPanelImpl.cpp_splint \
	view/ModAnoubisOverviewPanelImpl.h_splint \
	view/ModAnoubisPanelsBase.cpp_splint \
	view/ModAnoubisPanelsBase.h_splint \
	view/ModOverview.cpp_splint \
	view/ModOverview.h_splint \
	view/ModOverviewMainPanelImpl.cpp_splint \
	view/ModOverviewMainPanelImpl.h_splint \
	view/ModOverviewPanelsBase.cpp_splint \
	view/ModOverviewPanelsBase.h_splint \
	view/Module.cpp_splint \
	view/Module.h_splint \
	view/TrayIcon.cpp_splint \
	view/TrayIcon.h_splint \
	ctassert.h_splint \
	main.cpp_splint \
	main.h_splint

lint: $(xanoubis_SOURCES_lint) $(xanoubis_SOURCES_splint) \
	$(xanoubis_SOURCES_flawfinder)

flawfinder_FLAGS=--minlevel=4 --allowlink --context
splint_FLAGS=+posixlib @WX_CPPFLAGS@
lint_FLAGS=""

flawfinder:	$(xanoubis_SOURCES_flawfinder)
splint:		$(xanoubis_SOURCES_splint)
lint:		$(xanoubis_SOURCES_lint)

.cpp.cpp_flawfinder .h.h_flawfinder:
	flawfinder $(flawfinder_FLAGS) $< > $@

.SUFFIXES= .lint

.cpp.cpp_lint .h.h_lint:
	@if expr $(build_os) : "openbsd" >/dev/null; then \
		lint $< > $@; \
		echo [lint] $<; \
	else \
		touch $@; \
	fi

# splint returns an bad exitcode on every source code hit. We need to
# ignore these exits for a successful build
.cpp.cpp_splint .h.h_splint:
	@echo splint stub
	-splint $(splint_FLAGS) $< > $@


clean-local:
	@rm -f $(xanoubis_SOURCES_lint)
	@rm -f $(xanoubis_SOURCES_splint)
	@rm -f $(xanoubis_SOURCES_flawfinder)
