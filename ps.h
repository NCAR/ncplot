/*
-------------------------------------------------------------------------
OBJECT NAME:    ps.h
 
FULL NAME:      Printing/PostScript header include file.
 
DESCRIPTION:
-------------------------------------------------------------------------
*/

#ifndef PS_H
#define PS_H

enum	layout { LANDSCAPE, PORTRAIT };

typedef struct
	{
	char	lpCommand[64];
	float	width, height;	/* Plot Size (!frame size)	*/
	float	widthRatio, heightRatio, fontRatio;
	int	shape;
	int	dpi;		/* Dots Per Inch		*/
	bool	color;
	} PRINTER;

extern PRINTER  printerSetup;

extern const char *show, *rightShow, *lineto, *moveto;

FILE	*openPSfile(char *outFile);

void	PSheader(FILE *, PLOT_INFO *),
	PStitles(FILE *, PLOT_INFO *),
	PSlabels(FILE *, PLOT_INFO *),
	PSbox(FILE *, PLOT_INFO *),
	PSxTics(FILE *, PLOT_INFO *, bool),
	PSyTics(FILE *, PLOT_INFO *, int, bool),
	PSxTicsLog(FILE *, PLOT_INFO *),
	PSstatsLegend(FILE *, PLOT_INFO *, char *title, int ylegend, DATASET_INFO *),
	PSstatsTitle(FILE *, PLOT_INFO *, int ylegend),
	closePSfile(FILE *),
	SetPlotRatios(PLOT_INFO *);

#endif
