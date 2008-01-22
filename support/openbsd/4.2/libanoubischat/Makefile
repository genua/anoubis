# $OpenBSD$

COMMENT=	"lib providing transport functionality in anoubis"
V=		%%VERSION%%
DISTNAME=	libanoubischat-${V}
PKGNAME=	${DISTNAME}
CATEGORIES=	devel

MAINTAINER=	Christian Hiesl <hiesl@genua.de>

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

BUILD_DEPENDS+=		::devel/check

SEPARATE_BUILD=		concurrent
CONFIGURE_STYLE=	gnu

.include <bsd.port.mk>
