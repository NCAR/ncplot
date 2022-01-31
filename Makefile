#
# Makefile for ncplot
#

LEX	= flex
YACC	= bison
YFLAGS	= -d

# Linux Redhat / CentOS / Fedora
#
# yum install flex-devel netcdf-devel motif-devel gsl-devel \
#    xorg-x11-fonts-ISO8859-1-75dpi xorg-x11-fonts-ISO8859-1-100dpi
#
DEFINES	= -DPNG
INCLUDES= 
LIB_DIRS= 
NCH_DEP	= /usr/include/netcdf.h
LIBS    = -lXm -lXt -lX11 -lnetcdf -lhdf5 -lhdf5_hl -lfl -lgsl -lgslcblas -lpng -lpthread
BIN	= ${JLOCAL}/bin
WWW	= /net/www/docs/raf/Software


# Linux Ubuntu
#
# apt-get install bison flex libgsl-dev libnetcdf-dev libxt-dev libmotif-dev
#
#DEFINES	= -DPNG -DUBUNTU
#LIBS    = -lXm -lXt -lX11 -lnetcdf -lfl -lgsl -lgslcblas -lpng -lpthread


# Mac OS X (ncplot is availble from macports as of 2021).
#
# Requires XCode from Apple App Store.  Don't forget to run Xcode after it is
# installed to accept license.
#
# Need to get XQuartz download for X11, it is no longer bundled
# with MacOS.  https://www.xquartz.org/
#
# Using Homebrew instead of MacPorts.  https://brew.sh/
#  brew install openmotif netcdf gsl
#  brew install gmt  ; for geo-politcal maps.
#  brew install whatever else
#
#DEFINES	= -DPNG
#LIBS    = -lXm -lXt -lX11 -lnetcdf -ll -lgsl -lpng -lpthread
#BIN	= /usr/local/bin

# Homebrew
#INCLUDES=
#LIB_DIRS=
#NCH_DEP	= /usr/local/include/netcdf.h

# MacPorts
#INCLUDES= -I/opt/local/include
#LIB_DIRS= -L/opt/local/lib
#NCH_DEP	= /opt/local/include/netcdf.h


CXXFLAGS	= -Wall -g -O2 ${INCLUDES} ${DEFINES} -Wno-write-strings -Wno-overflow

PROG	= ncplot
HDRS	= define.h extern.h

SRCS=	ncplot.c global.c init.c X.c Xwin.c annotate.c arrows.c ascii.c\
	autoscale.c barbs.c ccb.c color.c control.c cospec.c crosshair.c\
	cursor.c dataIO.c detrend.c diff.c diffPS.c diffX.c ed_diff.c\
	ed_parms.c ed_plot.c ed_print.c ed_spec.c ed_stats.c ed_xy.c\
	ed_xyz.c elia.c equal.c error.c expar.c exp.tab.c lex.yy.c\
	geopolmap.c header.c labels.c landmarks.c logtics.c page.c\
	panel.c plotPS.c plotX.c png.c preferences.c print.c ps.c regret.c\
	regret1.c rt.c search.c spctrm.c spec.c specPS.c specX.c\
	stats.c template.c timestamps.c titles.c track.c validate.c\
	variance.c window.c xyPS.c xyX.c xyzPS.c xyzX.c zoom.c Xquery.c\
	Xerror.c Xfile.c Xwarn.c sanity.c opener.c

OBJS=	ncplot.o global.o init.o X.o Xwin.o annotate.o arrows.o ascii.o\
	autoscale.o barbs.o ccb.o color.o control.o cospec.o crosshair.o\
	cursor.o dataIO.o detrend.o diff.o diffPS.o diffX.o ed_diff.o\
	ed_parms.o ed_plot.o ed_print.o ed_spec.o ed_stats.o ed_xy.o\
	ed_xyz.o elia.o equal.o error.o expar.o exp.tab.o lex.yy.o\
	geopolmap.o header.o labels.o landmarks.o logtics.o page.o\
	panel.o plotPS.o plotX.o png.o preferences.o print.o ps.o regret.o\
	regret1.o rt.o search.o spctrm.o spec.o specPS.o specX.o\
	stats.o template.o timestamps.o titles.o track.o validate.o\
	variance.o window.o xyPS.o xyX.o xyzPS.o xyzX.o zoom.o Xquery.o\
	Xerror.o Xfile.o Xwarn.o sanity.o opener.o

SPECOBJ=cospec.o detrend.o ed_spec.o global.o init.o spctrm.o spec.o\
	specPS.o specX.o variance.o window.o dataIO.o ascii.o

PSOBJ=	annotate.o arrows.o ascii.o barbs.o diffPS.o ed_print.o global.o\
	header.o init.o plotPS.o print.o ps.o specPS.o stats.o variance.o\
	xyPS.o xyzPS.o


.c.o:
	${CXX} ${CXXFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CXX} ${CXXFLAGS} ${LIB_DIRS} ${OBJS} ${LIBS} -o $@

exp.tab.c exp.tab.h: exp.y
	${YACC} ${YFLAGS} exp.y
lex.yy.c: exp.l
	${LEX} exp.l

install: ${PROG}
	test -d $(BIN) || mkdir $(BIN)
	cp ${PROG} $(BIN)

publish: $(PROG)
	cp ${PROG} $(ANONFTP)
	cp ${PROG}.html $(WWW)

clean:
	rm -f core* ${OBJS} ${PROG} exp.tab.h exp.tab.c lex.yy.c

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	${HDRS}
${SPECOBJ}:	spec.h
${PSOBJ}:	ps.h

ncplot.o:	fbr.h
dataIO.o rt.o stats.o:	${NCH_DEP}
lex.yy.o:	lex.yy.c exp.tab.h
