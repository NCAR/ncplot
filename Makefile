#
# Makefile for ncplot
#
WWW	= /net/www/docs/raf/Software

CC	= g++
LEX	= flex
YACC	= bison
YFLAGS	= -d


# ${LOCAL}/include needs to locate netcdf.h

# Linux
#
DEFINES	= -DPNG
INCLUDES= ${JLOCAL}/include -I/usr/X11R6/include
LIB_DIRS= -L/usr/X11R6/lib -L${JLOCAL}/lib
LIBS    = -lXm -lXp -lXt -lXext -lX11 -lnetcdf -lfl -lm -lpng -lz -lpthread
BIN	= /home/local/bin
IHOST	= syrah
ANONFTP	= /net/ftp/pub/archive/RAF-src/bin.Redhat9.0

# Mac OS X
#
#DEFINES	= -DPNG
#INCLUDES= -I/usr/include -I/usr/X11R6/include -I/sw/include
#LIB_DIRS= -L/sw/lib -L/usr/X11R6/lib -L/usr/lib
#LIBS    = -lXm -lXt -lXext -lX11 -lnetcdf -lfl -lm -lpng -lz -lpthread
#LIBS    = -lXm -lXt -lXext -lX11 -lnetcdf -lfl -lm -lpthread
#BIN	= /sw/bin

# Solaris
#
#DEFINES	= -DSVR4 -DPNG
#INCLUDES= ${LOCAL}/include -I/usr/openwin/include -I/usr/dt/include
#LIBS	= -Wl,-Bstatic -lpng -lz -Wl,-Bdynamic -R /usr/dt/lib -lXm -lXt -lX11 -lnetcdf -ll -lm -lpthread
#LIB_DIRS= -L/net/lcal_sol/lib
#BIN	= /net/local_sol/bin
#IHOST	= mead
#ANONFTP	= /net/ftp/pub/archive/RAF-src/bin.Solaris2.9

# HP
#
#DEFINES	= -DSVR4 -DPNG
#INCLUDES= ${LOCAL}/include -I/usr/include/X11R6 -I/usr/include/Motif1.2
#LIB_DIRS= -L${LOCAL}/lib
#LIBS    = -lXm -lXt -lX11 -lnetcdf -ll -lm -L/usr/lib/Motif1.2 -L/usr/lib/X11R6 -L/lib/pa1.1

# Irix
#
#INCLUDES= ${LOCAL}/include
#LIBS	= -lXm -lXt -lX11 -lnetcdf -lm

CFLAGS	= -g -O2 -I${INCLUDES} ${DEFINES}

PROG	= ncplot
HDRS	= define.h extern.h

SRCS=	ncplot.c global.c init.c X.c Xwin.c annotate.c arrows.c ascii.c\
	autoscale.c barbs.c ccb.c color.c control.c cospec.c crosshair.c\
	cursor.c dataIO.c detrend.c diff.c diffPS.c diffX.c ed_diff.c\
	ed_parms.c ed_plot.c ed_print.c ed_spec.c ed_stats.c ed_xy.c\
	ed_xyz.c elia.c equal.c error.c expar.c exp.tab.c lex.yy.c fft.c\
	geopolmap.c getmem.c header.c labels.c landmarks.c logtics.c page.c\
	panel.c plotPS.c plotX.c png.c preferences.c print.c ps.c regret.c\
	regret1.c rt.c search.c sort.c spctrm.c spec.c specPS.c specX.c\
	stats.c template.c timestamps.c titles.c track.c validate.c\
	variance.c window.c xyPS.c xyX.c xyzPS.c xyzX.c zoom.c Xquery.c\
	Xerror.c Xfile.c Xwarn.c

OBJS=	ncplot.o global.o init.o X.o Xwin.o annotate.o arrows.o ascii.o\
	autoscale.o barbs.o ccb.o color.o control.o cospec.o crosshair.o\
	cursor.o dataIO.o detrend.o diff.o diffPS.o diffX.o ed_diff.o\
	ed_parms.o ed_plot.o ed_print.o ed_spec.o ed_stats.o ed_xy.o\
	ed_xyz.o elia.o equal.o error.o expar.o exp.tab.o lex.yy.o fft.o\
	geopolmap.o getmem.o header.o labels.o landmarks.o logtics.o page.o\
	panel.o plotPS.o plotX.o png.o preferences.o print.o ps.o regret.o\
	regret1.o rt.o search.o sort.o spctrm.o spec.o specPS.o specX.o\
	stats.o template.o timestamps.o titles.o track.o validate.o\
	variance.o window.o xyPS.o xyX.o xyzPS.o xyzX.o zoom.o Xquery.o\
	Xerror.o Xfile.o Xwarn.o

SPECOBJ=cospec.o detrend.o ed_spec.o global.o init.o spctrm.o spec.o\
	specPS.o specX.o variance.o window.o dataIO.o ascii.o

PSOBJ=	annotate.o arrows.o ascii.o barbs.o diffPS.o ed_print.o global.o\
	header.o init.o plotPS.o print.o ps.o specPS.o stats.o variance.o\
	xyPS.o xyzPS.o


.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${LIB_DIRS} ${OBJS} ${LIBS} -o $@

exp.tab.c exp.tab.h: exp.y
	${YACC} ${YFLAGS} exp.y
lex.yy.c: exp.l
	${LEX} exp.l

install: ${PROG}
	cp ${PROG} $(BIN)
	cp ${PROG} $(ANONFTP)
	cp ${PROG}.html $(WWW)
	scp ${PROG} ${IHOST}:$(BIN)

clean:
	rm -f core* ${OBJS} ${PROG}

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	${HDRS}
${SPECOBJ}:	spec.h
${PSOBJ}:	ps.h

ncplot.o:	fbr.h
dataIO.o rt.o stats.o:	${LOCAL}/include/netcdf.h
lex.yy.o:	lex.yy.c exp.tab.h
