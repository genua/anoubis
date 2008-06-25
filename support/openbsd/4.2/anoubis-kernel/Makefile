# $OpenBSD$

ONLY_FOR_ARCHS= i386
COMMENT=        anoubis Kernel
# MS6 0.3
# MS7 0.4
V=              0.4

DISTNAME=       anoubis-kernel-${V}
PKGNAME=        ${DISTNAME}p0
CATEGORIES=     devel
MASTER_SITES=   http://srv/~trahm/

HOMEPAGE=       http://www.genua.de/
MAINTAINER=     Hans-Joerg Hoexer <hshoexer@genua.de>

# BSD
PERMIT_PACKAGE_CDROM=   Yes
PERMIT_PACKAGE_FTP=     Yes
PERMIT_DISTFILES_CDROM= Yes
PERMIT_DISTFILES_FTP=   Yes

NO_BUILD=               Yes
NO_REGRESS=             Yes
PKG_ARCH=               i386

do-install:
@cp ${WRKSRC}/bsd.anoubis ${PREFIX}/bin/bsd.anoubis

.include <bsd.port.mk>
