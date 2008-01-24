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

NO_REGRESS=	YES

MULTI_PACKAGES=	-main -daemon

LIB_DEPENDS-main=	wx_base_xml,wx_gtk2_aui,wx_gtk2_qa,wx_base_odbc,wx_gtk2_html,wx_base,wx_gtk2_adv,wx_gtk2_xrc,wx_gtk2_core,wx_base_net,wx_gtk2_dbgrid:wxWidgets-gtk2->=2.6.3:x11/wxWidgets

WANTLIB-main+=		c m pthread stdc++
WANTLIB-daemon+=	c
CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--enable-static-anoubisd

.include <bsd.port.mk>
