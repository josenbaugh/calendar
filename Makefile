PREFIX = /usr/local

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS}

LDFLAGS  = ${LIBS}

CC = cc

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f calendar ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/calendar

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/calendar\
