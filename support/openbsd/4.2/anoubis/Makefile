# $OpenBSD$

COMMENT-main=	the Anoubis Security Suite
COMMENT-daemon=	the anoubisd of the Anoubis Security Suite
V=		%%VERSION%%
DISTNAME=	anoubis-$V
PKGNAME-main=	anoubis-gui-$V
PKGNAME-daemon=	anoubis-daemon-$V

CATEGORIES=	security

MAINTAINER=	Hans-Joerg Hoexer <hshoexer@genua.de>

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

USE_X11=	YES

MULTI_PACKAGES=	-main -daemon

RUN_DEPENDS-main=	:notification-daemon:sysutils/notification-daemon
LIB_DEPENDS-main=	notify::devel/libnotify\
			wx_base_xml,wx_gtk2_aui,wx_gtk2_qa,wx_base_odbc,wx_gtk2_html,wx_base,wx_gtk2_adv,wx_gtk2_xrc,wx_gtk2_core,wx_base_net,wx_gtk2_dbgrid:wxWidgets-gtk2->=2.8.6:x11/wxWidgets

WANTLIB-main+=		c m pthread stdc++ event glib-2.0 \
			X11 Xau Xcursor Xdmcp Xext Xfixes Xi Xinerama Xrandr \
			Xrender atk-1.0 cairo dbus-1 dbus-glib-1 expat \
			fontconfig freetype gdk-x11-2.0 gdk_pixbuf-2.0 \
			glitz gmodule-2.0 gobject-2.0 gtk-x11-2.0 iconv \
			intl pango-1.0 pangocairo-1.0 pangoft2-1.0 png z \
			crypto ssl

WANTLIB-daemon+=	c
CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--enable-static-anoubisd

REGRESS_DEPENDS=	::devel/check

.include <bsd.port.mk>
