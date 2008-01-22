# $OpenBSD$

COMMENT=		GUI for the Anoubis Security Suite
V=			%%VERSION%%
DISTNAME=		xanoubis-${V}
PKGNAME=		${DISTNAME}
CATEGORIES=		x11

MAINTAINER=		Christian Hiesl <hiesl@genua.de>

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

NO_REGRESS=		Yes

LIB_DEPENDS=		wx_gtk2_aui,wx_base_xml,wx_gtk2_qa,wx_base_odbc,wx_gtk2_html,wx_base,wx_gtk2_adv,wx_gtk2_xrc,wx_gtk2_core,wx_base_net,wx_gtk2_dbgrid:wxWidgets-gtk2->=2.8.6:x11/wxWidgets

BUILD_DEPENDS+=		::libanoubischat

WANTLIB+=		c m pthread stdc++
CONFIGURE_STYLE=	gnu
CONFIGURE_ENV+=		PKG_CONFIG_PATH="${LOCALBASE}/lib/pkgconfig"
CONFIGURE_ARGS+=	--enable-static

PORTSDIR_PATH?=		${PORTSDIR}:${PORTSDIR}/mystuff:%%TMP_BUILD_DIR%%

.include <bsd.port.mk>
