/*
-------------------------------------------------------------------------
OBJECT NAME:	global.c

FULL NAME:	Global Variable Definitions

DESCRIPTION:	extern.h should look just like this.
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"
#include "spec.h"

bool	Interactive;	/* Interactive or batch mode			*/
bool	RealTime;	/* Onboard realtime reading of netCDF		*/
bool	Freeze;		/* Freeze updates while in RealTime		*/
bool	Color;
bool	DataChanged;	/* New data was read from file.			*/
bool	AsciiWinOpen;
bool	StatsWinOpen;
bool	Statistics;	/* Display statistics on main window.		*/
bool	UTCseconds;	/* Seconds since midnight vs. HH:MM:SS		*/

bool	allLabels;	/* Labels on all panels for multi-panel		*/
bool	ScatterPlot;	/* Scatter instead of line on XY		*/
bool	ProjectToXY;	/* Project XYZ  to ground			*/
bool	ProjectToBack;	/* Project XYZ to back & side planes.		*/
bool	WindBarbs;	/* Show wind barbs on XY			*/
bool	LandMarks;	/* Show hl proj/###/landmarks on XY or XYZ	*/
int	ShowRegression;	/* Show regression on XY			*/

char	buffer[BUFFSIZE], *parmsFile, *outFile, DataPath[PATH_LEN],
	*timeSeg, tasVarName[40];

DATAFILE_INFO	dataFile[MAX_DATAFILES];

DATASET_INFO	dataSet[MAX_DATASETS],	/* Time Series data sets	*/
		xyXset[MAX_DATASETS],	/* XY X data sets		*/
		xyYset[MAX_DATASETS],	/* XY Y data sets		*/
		xyzSet[3],		/* XYZ data sets.		*/
		diffSet,		/* Difference data set		*/
		ui, vi;			/* Wind barbs data sets		*/

PLOT_INFO	mainPlot[MAX_PANELS], xyyPlot[MAX_PANELS], xyzPlot,
		specPlot, diffPlot;

PRINTER		printerSetup;

int	CurrentDataFile, CurrentDataSet, CurrentPanel, PlotType,
	NumberDataFiles, NumberDataSets, NumberXYYsets, NumberXYXsets,
	NumberOfPanels, NumberOfXYpanels;

int	NumberSeconds, nDirectionArrows, nTimeStamps, LineThickness;


/* Regression coeffs	*/
double	regretCo[5];

/* Spectral globals (see spec.h for externs).	*/
PSD_INFO	psd[MAX_PSD];
DATASET_INFO	tas;

const char *statsTitle =
	"                                     mean      sigma        min        max";


/* Time stuff */
int	UserStartTime[4], UserEndTime[4];	/* HH:MM:SS, 4th is seconds*/
int	MinStartTime[4], MaxEndTime[4];		/* since midnight.	*/

/* Parameter File Variables	*/
char	asciiFormat[10];

int	nASCIIpoints;

/* X vars	*/
Widget	varList;

instanceRec	iv;

/* END GLOBAL.C */
