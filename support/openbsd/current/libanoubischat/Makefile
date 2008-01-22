# $Id: d88e7c,v 1.4 2007/11/23 16:05:21 ehrhardt Exp $

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

WANTLIB+=		c

SEPARATE_BUILD=		concurrent
CONFIGURE_STYLE=	gnu

.include <bsd.port.mk>
