# $OpenBSD$

COMMENT-main =		gui of the Anoubis Security Suite
COMMENT-daemon =	anoubisd of the Anoubis Security Suite
V =			%%VERSION%%

DISTNAME =		anoubis-$V
PKGNAME-main =		anoubis-gui-$V
PKGNAME-daemon =	anoubis-daemon-$V

CATEGORIES =		security

HOMEPAGE =		http://www.anoubis.org/

MAINTAINER =		Sebastian Trahm <trahm@genua.de>

# BSD
PERMIT_PACKAGE_CDROM =	Yes
PERMIT_PACKAGE_FTP =	Yes
PERMIT_DISTFILES_CDROM =Yes
PERMIT_DISTFILES_FTP =	Yes

WANTLIB-daemon +=	c crypto event ssl iconv intl
WANTLIB-main =		c m pthread stdc++ event glib-2.0 pixman-1 \
			X11 Xau Xcursor Xdmcp Xext Xfixes Xi Xinerama \
			Xrandr Xcomposite Xdamage Xrender atk-1.0 cairo \
			Xrender atk-1.0 dbus-1 dbus-glib-1 expat \
			fontconfig freetype gdk-x11-2.0 gdk_pixbuf-2.0 \
			glitz gmodule-2.0 gio-2.0 gobject-2.0 gtk-x11-2.0 \
			iconv intl pango-1.0 pangocairo-1.0 pangoft2-1.0 png z \
			crypto ssl

MASTER_SITES =		${HOMEPAGE}downloads/files/

ANOUBISDIR =		/etc/anoubis
PROFILESDIR =		${ANOUBISDIR}/profiles
POLICYDIR =		${ANOUBISDIR}/policy
SUBST_VARS =		ANOUBISDIR PROFILESDIR POLICYDIR

MULTI_PACKAGES =	-main -daemon

MODULES=		devel/gettext
BUILD_DEPENDS =		::devel/check \
			::devel/bison
RUN_DEPENDS-main =	:notification-daemon->=0.3.7:sysutils/notification-daemon \
			:desktop-file-utils-*:devel/desktop-file-utils \
			:anoubis-daemon->=0.9.3.216:%%SEARCH_DIR%%/anoubis
LIB_DEPENDS-main =	notify::devel/libnotify \
			wx_base_xml,wx_gtk2_aui,wx_gtk2_qa,wx_base_odbc,wx_gtk2_html,wx_base,wx_gtk2_adv,wx_gtk2_xrc,wx_gtk2_core,wx_base_net,wx_gtk2_richtext,wx_gtk2_dbgrid:wxWidgets-gtk2->=2.8.6:x11/wxWidgets

# explicitly unset WRKOBJDIR to create it within port directory
WRKOBJDIR =

USE_X11 =		YES

CONFIGURE_STYLE =	gnu
CONFIGURE_ARGS +=	--enable-static-anoubisd

REGRESS_DEPENDS =	${BUILD_DEPENDS}

do-regress:
	cd ${WRKSRC}/test && ${MAKE} check

post-install:
	@rm ${WRKINST}/usr/local/share/xanoubis/profiles/admin
	@rm ${WRKINST}/usr/local/share/xanoubis/profiles/high
	@rm ${WRKINST}/usr/local/share/xanoubis/profiles/medium

	@ ${INSTALL_DATA_DIR} ${PREFIX}/share/anoubisd/policy_templates/admin/
	@ ${INSTALL_DATA_DIR} ${PREFIX}/share/anoubisd/policy_templates/user/
	@ ${INSTALL_DATA_DIR} ${PREFIX}/share/anoubisd/policy_templates/profiles/

	for a in admin user profiles ; do \
	    for f in ${FILESDIR}/policy.$$a.* ; do \
		cp $$f ${PREFIX}/share/anoubisd/policy_templates/$$a/$${f##*/policy.$$a.} ;\
	    done ;\
	done

	cp ${FILESDIR}/anoubisd.conf ${PREFIX}/share/anoubisd ;

	@ ${INSTALL_DATA_DIR} ${PREFIX}/share/applications/
	@ ${INSTALL_DATA} ${WRKSRC}/src/xanoubis/xanoubis.desktop \
		${PREFIX}/share/applications/

.include <bsd.port.mk>
