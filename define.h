/*
-------------------------------------------------------------------------
OBJECT NAME:	define.h

FULL NAME:	Include File to Include the Include Files

DESCRIPTION:	
-------------------------------------------------------------------------
*/

#ifndef DEFINE_H
#define DEFINE_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>

#ifndef ERR
#define OK		(0)
#define ERR		(-1)
#endif
 
#define COMMENT	'#' /* Comment character for textfiles  */
 
#ifndef MAX
#define MAX(x, y)	(x > y ? x : y)
#endif
 
#ifndef MIN
#define MIN(x, y)	(x > y ? y : x)
#endif

#define SecondsSinceMidnite(t)	(t[0] * 3600 + t[1] * 60 + t[2])


#define BUFFSIZE	2048
#define PATH_LEN	256
#define NAMELEN		20

#define MAX_DATAFILES	4
#define MAX_DATASETS	16
#define MAX_PANELS	4
#define MAX_VARIABLES	800

#define TITLESIZE	100
#define UNITS_LEN	20

#define ALL_SETS	(-1)

#define MISSING_VALUE	(-32767.0)
#define COMPUTED	(-2)


/* Plot type	*/
enum { TIME_SERIES, XY_PLOT, XYZ_PLOT };
enum { X_AXIS=0x01, Y_AXIS=0x02, Z_AXIS=0x04 };

/* Which side of graph: these are used as array indicies	*/
enum { LEFT_SIDE, RIGHT_SIDE };

/* Values for CursorMode		*/
enum { POINTER, WAIT, ANNOTATE, GRAB, CROSSHAIR };

/* Values for specDisplay		*/
enum { SPECTRA=1, COSPECTRA, QUADRATURE, COHERENCE, PHASE, RATIO };

/* Values for "HandleError"		*/
enum { RETURN, EXIT, IRET };

/* Currently for save PNG, but can be used for others also. */
enum { MAIN_CANVAS, SPEC_CANVAS, DIFF_CANVAS };


/* Values for reading Font Resources	*/
#define XtNfont24	"font24"
#define XtCFont24	"Font24"
#define XtNfont18	"font18"
#define XtCFont18	"Font18"
#define XtNfont14	"font14"
#define XtCFont14	"Font14"
#define XtNfont12	"font12"
#define XtCFont12	"Font12"
#define XtNfont10	"font10"
#define XtCFont10	"Font10"

#define MAX_FONTS	5


typedef float	NR_TYPE;


/* Struct for reading font resources from resource file	*/
typedef struct _insRec
	{
	char	*font24;
	char	*font18;
	char	*font14;
	char	*font12;
	char	*font10;
	} instanceRec;

typedef struct
	{
	char	name[NAMELEN];
	int	inVarID;        /* netCDF variable ID       */
	int	OutputRate;
	char	*expression;
	} VARTBL;

typedef struct
	{
	long	nPoints;
	float	min;
	float	max;
	float	mean;
	float	sigma;
	float	variance;
	float	outlierMin;
	float	outlierMax;
	char	units[UNITS_LEN];
	} STAT;

typedef struct
	{
	char	fileName[PATH_LEN];
	int	ncid;
	int	nVariables;
	VARTBL	*Variable[MAX_VARIABLES];
	char	*ProjectNumber;
	char	*ProjectName;
	char	*FlightNumber;
	char	*FlightDate;
	int	FileStartTime[4], FileEndTime[4];
	int	baseDataRate;	/* Most files are 1, this is to handle ncav */

	char	*catName[32];	/* Category list		*/
	} DATAFILE_INFO;

typedef struct
	{
	int	fileIndex;
	VARTBL	*varInfo;	/* Pointer into VARTBL			*/
	int	panelIndex;	/* Which panel, time series only	*/
	int	scaleLocation;	/* LEFT_SIDE or RIGHT_SIDE		*/

	long	nPoints;	/* Total # data points in array		*/
	NR_TYPE	*data;
	NR_TYPE	missingValue;
	int	head;		/* Realtime only, index into data	*/
	STAT	stats;
	} DATASET_INFO;

struct plot_offset
	{
	short	windowHeight, windowWidth;	/* in Pixels		*/
	int	HD, VD;		/* Horizontal & Vertical Distance, in Pixels*/
	int	LV, RV;		/* Left & Right Vertical, in Pixels	*/
	int	TH, BH;		/* Top & Bottom Horizontal, in Pixels	*/
	int	ticLength;	/* in Pixels				*/

	int	titleOffset, subTitleOffset, xLabelOffset, yLabelOffset,
		yTicLabelOffset, xTicLabelOffset, xLegendText;
	};

struct axisInfo
	{
	char	label[TITLESIZE];
	bool	logScale;
	bool	invertAxis;
	int	nMajorTics, nMinorTics;
	float	smallestValue, biggestValue;
	float	min, max;
	};

typedef struct
	{
	Widget		canvas;
	Display		*dpy;
	Window		win;
	GC		gc;
	XGCValues	values;
	XFontStruct	*fontInfo[MAX_FONTS];

	int	plotType;	/* TIME_SERIES, XY_PLOT or XYZ_PLOT	*/
	bool	windowOpen;
	bool	grid;
	bool	autoScale;
	bool	autoTics;

	char	title[TITLESIZE], subTitle[TITLESIZE];

	struct plot_offset	ps;	/* PostScript info		*/
	struct plot_offset	x;	/* Xwindow info			*/

	struct axisInfo		Xaxis, Yaxis[2], Zaxis;
	} PLOT_INFO;


#include "extern.h"

#endif

/* END DEFINE.H */
