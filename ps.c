/*
-------------------------------------------------------------------------
OBJECT NAME:	ps.c

FULL NAME:	PostScript building blocks.

ENTRY POINTS:	openPSfile()
		PSheader()
		PStitles()
		PSbox()
		PSlabels()
		PSxTics()
		PSyTics()
		PSstatsLegend()
		PSstatsTitle()
		PSclip()
		PSclearClip()
		yLegendPS()
		closePSfile()

STATIC FNS:	none

DESCRIPTION:	Generic Post Script routines for Titles, labels, tic marks,
		etc.

REFERENCES:	none

REFERENCED BY:	plotPS.c, specPS.c, diffPS.c, trackPS.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1995-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>

const char	*show   = "(%s) s\n",
		*rightShow  = "(%s) rs\n",
		*lineto = "%d %d l\n",
		*moveto = "%d %d m\n";

/* -------------------------------------------------------------------- */
FILE *openPSfile(char *outFile)
{
  FILE	*fp;

  if (outFile)
    {
    if ((fp = fopen(outFile, "w")) == NULL)
      {
      sprintf(buffer, "Can't open output file %s", outFile);
      HandleError(buffer, Interactive, IRET);
      return(NULL);
      }
    }
  else
    if ((fp = popen(printerSetup.lpCommand, "w")) == NULL)
      {
      HandleError("ps: can't open pipe to 'lp'", Interactive, IRET);
      return(NULL);
      }

  return(fp);

}	/* END OPENPSFILE */

/* -------------------------------------------------------------------- */
void PSheader(FILE *fp, PLOT_INFO *plot)
{
  char		*user, *date;
  time_t	print_time = time((time_t *)NULL);

  user = (getpwuid(getuid()))->pw_gecos;
  date = ctime(&print_time);

  /* Print standard header info, and quadruple number of pixels
   */
  fprintf(fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(fp, "%%%%Creator: ncplot\n");
  fprintf(fp, "%%%%For: %s\n", user);
  fprintf(fp, "%%%%Title: %s\n", plot->title);
  fprintf(fp, "%%%%CreationDate: %s", date);
  fprintf(fp, "%%%%Pages: 1\n");
  fprintf(fp, "%%%%BoundingBox: 0 0 %d %d\n",
                     (int)(612 * printerSetup.widthRatio),
                     (int)(792 * printerSetup.heightRatio));
  fprintf(fp, "%%%%Orientation: %s\n", printerSetup.shape == PORTRAIT ?
                                         "Portrait" : "Landscape");
  fprintf(fp, "%%%%DocumentNeededResources: font Times-Roman\n");
  fprintf(fp, "%%%%EndComments\n");

  fprintf(fp, "%%%%BeginDefaults\n");
  fprintf(fp, "%%%%PageResources: font Times-Roman\n");
  fprintf(fp, "%%%%EndDefaults\n");

  fprintf(fp, "%%%%BeginProlog\n");
  fprintf(fp, "/m /moveto load def\n");
  fprintf(fp, "/l /lineto load def\n");
  fprintf(fp, "/s /show load def\n");
  fprintf(fp, "/rs {dup stringwidth pop 120 exch sub 0 rmoveto show");
  fprintf(fp, "} bind def\n");
  fprintf(fp, "%%%%EndProlog\n");

  fprintf(fp, "%%%%BeginSetup\n");
  fprintf(fp, "%%%%IncludeResource: font Times-Roman\n");
  fprintf(fp, "save\n");
  fprintf(fp, "%g %g scale\n", 0.25, 0.25);
  fprintf(fp, "%%%%EndSetup\n");

  fprintf(fp, "%%%%Page: 1 1\n");

  if (printerSetup.shape == LANDSCAPE)
    {
    fprintf(fp, "%d %d translate\n", 0, 3168);
    fprintf(fp, "-90 rotate\n");

    fprintf(fp, moveto, 2750, 2350);
    }
  else
    {
    fprintf(fp, moveto, 2000, 3050);
    }

  fprintf(fp, "/Times-Roman findfont 18 scalefont setfont\n");
  fprintf(fp, "(%s, %s) show\n", user, date);

}	/* END PSHEADER */

/* -------------------------------------------------------------------- */
void PStitles(FILE *fp, PLOT_INFO *plot)
{
  int	x = plot->ps.windowWidth >> 1;

  /* Print Titles
   */
  if (strlen(plot->title))
    {
    fprintf(fp, "/Times-Roman findfont %d scalefont setfont\n",
		(int)(printerSetup.fontRatio * 80));
    fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
		x, plot->title, plot->ps.titleOffset);

    fprintf(fp, show, plot->title);
    }

  if (strlen(plot->subTitle))
    {
    fprintf(fp, "/Times-Roman findfont %d scalefont setfont\n",
		(int)(printerSetup.fontRatio * 60));
    fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
		x, plot->subTitle, plot->ps.subTitleOffset);

    fprintf(fp, show, plot->subTitle);
    }

}	/* END PSTITLES */

/* -------------------------------------------------------------------- */
void PSlabels(FILE *fp, PLOT_INFO *plot)
{
  fprintf(fp, "/Times-Roman findfont %d scalefont setfont\n",
		(int)(printerSetup.fontRatio * 60));

  /* Print Labels
   */
  if (strlen(plot->Yaxis[0].label))
    {
    fprintf(fp, "%d %d (%s) stringwidth pop 2 div sub moveto\n",
		plot->ps.yLabelOffset,
		plot->ps.VD >> 1, plot->Yaxis[0].label);

    fprintf(fp, "90 rotate\n");
    fprintf(fp, show, plot->Yaxis[0].label);
    fprintf(fp, "-90 rotate\n");
    }

  if (strlen(plot->Yaxis[1].label))
    {
    fprintf(fp, "%d %d (%s) stringwidth pop 2 div sub moveto\n",
		plot->ps.HD - plot->ps.yLabelOffset,
		plot->ps.VD >> 1, plot->Yaxis[1].label);

    fprintf(fp, "90 rotate\n");
    fprintf(fp, show, plot->Yaxis[1].label);
    fprintf(fp, "-90 rotate\n");
    }


  if (strlen(plot->Zaxis.label))
    {
    fprintf(fp, "%d %d (%s) stringwidth pop 2 div sub moveto\n",
		plot->ps.RV + 200, plot->ps.HD >> 3, plot->Zaxis.label);

    fprintf(fp, "30 rotate\n");
    fprintf(fp, show, plot->Zaxis.label);
    fprintf(fp, "-30 rotate\n");
    }

  if (strlen(plot->Xaxis.label))
    {
    if (PlotType == TIME_SERIES && NumberOfPanels > 1)
    fprintf(fp, "/Times-Roman findfont %d scalefont setfont\n",
		(int)(printerSetup.fontRatio * 50));

    fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
		plot->ps.HD >> 1, plot->Xaxis.label, plot->ps.xLabelOffset);

    fprintf(fp, show, plot->Xaxis.label);
    }

}	/* END PSLABELS */

/* -------------------------------------------------------------------- */
void PSbox(FILE *fp, PLOT_INFO *plot)
{
  /* Draw the bounding box for graph.
   */
  fprintf(fp, "3 setlinewidth\n");
  fprintf(fp, "stroke\n");
  fprintf(fp, moveto, plot->ps.LV, plot->ps.BH);
  fprintf(fp, lineto, plot->ps.LV, plot->ps.TH);
  fprintf(fp, lineto, plot->ps.RV, plot->ps.TH);
  fprintf(fp, lineto, plot->ps.RV, plot->ps.BH);
  fprintf(fp, lineto, plot->ps.LV, plot->ps.BH);
  fprintf(fp, "stroke\n");

}	/* END PSBOXPORTRAIT */

/* -------------------------------------------------------------------- */
void PSyTics(FILE *fp, PLOT_INFO *plot, int scale, bool labels)
{
  int		i, j, y, ticlen, nMajTics;
  float		nMajorYpix, nMinorYpix;
  double	value, yDiff;
  struct axisInfo *yAxis = &plot->Yaxis[scale];

  fprintf(fp, "/Times-Roman findfont %d scalefont setfont\n",
          (int)(printerSetup.fontRatio * 50));
  fprintf(fp, "1 setlinewidth\n");

  if (yAxis->logScale)
    {
    yLogTicsLabelsPS(fp, plot, scale, labels);
    return;
    }


  /* Draw Y-axis tic marks & corresponding values
   */
  yDiff		= yAxis->max - yAxis->min;
  nMajTics	= (int)(yAxis->logScale ? yDiff : yAxis->nMajorTics);
  ticlen	= plot->grid ? plot->ps.HD : plot->ps.ticLength;
  nMajorYpix	= (float)plot->ps.VD / nMajTics;
  nMinorYpix	= (float)nMajorYpix / yAxis->nMinorTics;

  for (i = 0; i <= nMajTics; ++i)
    {
    y = (int)(nMajorYpix * i + 0.5);

    fprintf(fp, moveto, 0, y);
    fprintf(fp, lineto, ticlen, y);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      {
      fprintf(fp, moveto, plot->ps.HD - ticlen, y);
      fprintf(fp, lineto, plot->ps.HD, y);
      }


    /* Label. */
    if (labels)
      {
      value = yAxis->min + (yDiff / nMajTics * i);

      if (yAxis->invertAxis)
        value = yAxis->max - (yDiff / nMajTics * i);
      else
        value = yAxis->min + (yDiff / nMajTics * i);
 
      MakeTicLabel(buffer, yDiff, nMajTics, value);

      if (scale == LEFT_SIDE)
        fprintf(fp, "%d (%s) stringwidth pop sub %d moveto (%s) show\n",
                plot->ps.yTicLabelOffset, buffer, y-10, buffer);
      else
        fprintf(fp, "%d %d moveto (%s) show\n",
                plot->ps.HD - plot->ps.yTicLabelOffset, y-10, buffer);
      }


    /* Minor Tic marks. */
    if (i != nMajTics)
      {
      for (j = 1; j < yAxis->nMinorTics; ++j)
        {
        y = (int)((nMajorYpix * i) + (nMinorYpix * j) + 0.5);

        fprintf(fp, moveto, 0, y);
        fprintf(fp, lineto, (int)(ticlen * 2 / 3), y);

        if (plot->plotType != XYZ_PLOT)
          {
          fprintf(fp, moveto, plot->ps.HD - ticlen * 2 / 3, y);
          fprintf(fp, lineto, plot->ps.HD, y);
          }
        }
      }
    }

  fprintf(fp, "stroke\n");

}	/* END PSYTICS */

/* -------------------------------------------------------------------- */
void PSxTics(FILE *fp, PLOT_INFO *plot, bool labels)
{
  int		i, j;
  int		x, ticlen, nMajTics;
  float		nMajorXpix, nMinorXpix;
  double	xDiff, value;
  struct axisInfo *xAxis = &plot->Xaxis;

  fprintf(fp, "/Times-Roman findfont %d scalefont setfont\n",
          (int)(printerSetup.fontRatio * 50));
  fprintf(fp, "1 setlinewidth\n");

  if (xAxis->logScale)
    {
    xLogTicsLabelsPS(fp, plot, labels);
    return;
    }

  xDiff		= xAxis->max - xAxis->min;
  nMajTics	= xAxis->nMajorTics;
  ticlen	= plot->grid ? plot->ps.VD : plot->ps.ticLength;
  nMajorXpix	= (float)plot->ps.HD / nMajTics;
  nMinorXpix	= (float)nMajorXpix / xAxis->nMinorTics;

  for (i = 0; i <= nMajTics; ++i)
    {
    x = (int)((nMajorXpix * i) + 0.5);

    if (xAxis->invertAxis)
      x = plot->ps.HD - x;

    fprintf(fp, moveto, x, 0);
    fprintf(fp, lineto, x, ticlen);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      {
      fprintf(fp, moveto, x, plot->ps.VD - ticlen);
      fprintf(fp, lineto, x, plot->ps.VD);
      }


    /* Label. */
    if (labels)
      {
      if (plot->plotType == TIME_SERIES)
        MakeTimeTicLabel(buffer, i, nMajTics);
      else
        {
        value = xAxis->min + (xDiff / nMajTics * i);
        MakeTicLabel(buffer, xDiff, nMajTics, value);
        }

      fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
					x, buffer, plot->ps.xTicLabelOffset);
      fprintf(fp, show, buffer);
      }


    /* Minor Tic marks. */
    if (i != nMajTics)
      {
      for (j = 1; j < xAxis->nMinorTics; ++j)
        {
        x = (int)((nMajorXpix * i) + (nMinorXpix * j) + 0.5);

        fprintf(fp, moveto, x, 0);
        fprintf(fp, lineto, x, (int)(ticlen * 2 / 3));

        if (plot->plotType != XYZ_PLOT)
          {
          fprintf(fp, moveto, x, plot->ps.VD - ticlen * 2 / 3);
          fprintf(fp, lineto, x, plot->ps.VD);
          }
        }
      }
    }

  fprintf(fp, "stroke\n");

}	/* END PSXTICS */

/* -------------------------------------------------------------------- */
void PSstatsLegend(FILE *fp, PLOT_INFO *plot, char *title, int cnt, DATASET_INFO *set)
{
  int	ylegend = yLegendPS(plot, cnt);

  if (printerSetup.color)
    fprintf(fp, "stroke\n0 0 0 setrgbcolor\n");

  fprintf(fp, moveto, plot->ps.xLegendText, ylegend);
  fprintf(fp, show, title);
 
  fprintf(fp, moveto,
	(int)(plot->ps.xLegendText + (700 * printerSetup.widthRatio)), ylegend);
  sprintf(buffer, "%11.2f", set->stats.mean);
  fprintf(fp, rightShow, buffer);
 
  fprintf(fp, moveto,
	(int)(plot->ps.xLegendText +(1000 * printerSetup.widthRatio)), ylegend);
  sprintf(buffer, "%11.2f", set->stats.sigma);
  fprintf(fp, rightShow, buffer);
 
  fprintf(fp, moveto,
	(int)(plot->ps.xLegendText +(1300 * printerSetup.widthRatio)), ylegend);
  sprintf(buffer, "%11.2f", set->stats.min);
  fprintf(fp, rightShow, buffer);
 
  fprintf(fp, moveto,
	(int)(plot->ps.xLegendText +(1600 * printerSetup.widthRatio)), ylegend);
  sprintf(buffer, "%11.2f", set->stats.max);
  fprintf(fp, rightShow, buffer);
 
}	/* END PSSTATSLEGEND */

/* -------------------------------------------------------------------- */
void PSstatsTitle(FILE *fp, PLOT_INFO *plot, int cnt)
{
  int	ylegend = yLegendPS(plot, cnt) - 30;

  fprintf(fp, moveto,
      (int)(plot->ps.xLegendText + (700 * printerSetup.widthRatio)), ylegend);
  fprintf(fp, rightShow, "mean");
 
  fprintf(fp, moveto,
      (int)(plot->ps.xLegendText + (1000 * printerSetup.widthRatio)), ylegend);
  fprintf(fp, rightShow, "sigma");
 
  fprintf(fp, moveto,
      (int)(plot->ps.xLegendText + (1300 * printerSetup.widthRatio)), ylegend);
  fprintf(fp, rightShow, "min");
 
  fprintf(fp, moveto,
      (int)(plot->ps.xLegendText + (1600 * printerSetup.widthRatio)), ylegend);
  fprintf(fp, rightShow, "max");
 
}	/* END PSSTATSTITLE */

/* -------------------------------------------------------------------- */
void closePSfile(FILE *fp)
{
  fprintf(fp, "stroke\n");
  fprintf(fp, "showpage\n");
  fprintf(fp, "%%%%Trailer\n");
  fprintf(fp, "restore\n");
  fprintf(fp, "%%EOF\n");

  if (outFile)
    fclose(fp);
  else
    pclose(fp);

}	/* END CLOSEPSFILE */

/* -------------------------------------------------------------------- */
int yLegendPS(PLOT_INFO *plot, int row)
{
  if (PlotType == TIME_SERIES && !Statistics)
    return(plot->ps.VD + 5);
  else
    return((int)(-plot->ps.BH + ((row + 2) * 40 * printerSetup.fontRatio)+5));

}	/* END YLEGENDPS */

/* -------------------------------------------------------------------- */
void PSclip(FILE *fp, PLOT_INFO *plot)
{
  fprintf(fp, "gsave\n");
  fprintf(fp, "0 0 moveto\n");
  fprintf(fp, "%d 0 rlineto\n", plot->ps.HD);
  fprintf(fp, "0 %d rlineto\n", plot->ps.VD);
  fprintf(fp, "%d neg 0 rlineto\n", plot->ps.HD);
  fprintf(fp, "closepath\n");
  fprintf(fp, "clip\n");
  fprintf(fp, "newpath\n");


}	/* END PSCLIP */

/* -------------------------------------------------------------------- */
void PSclearClip(FILE *fp)
{
  fprintf(fp, "stroke\n");
  fprintf(fp, "grestore\n");
  fprintf(fp, "newpath\n");

}	/* END PSCLIP */

/* END PS.C */
