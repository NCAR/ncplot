/*
-------------------------------------------------------------------------
OBJECT NAME:	xyPS.c

FULL NAME:	Produce PostScript File of XY Plot

ENTRY POINTS:	PrintXY()

STATIC FNS:	ResizePSxyyPlot()
		PSplotXY()
		PSplotRegression()

DESCRIPTION:	This does PostScript printing.

REFERENCES:	ps.c

REFERENCED BY:	PrintPostScript()

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

static int      currentPanel;

static void ResizePSxyyPlot();
static void PSplotXY(FILE *fp, PLOT_INFO *plot);
static void PSplotRegression(FILE *fp, PLOT_INFO *plot,
				DATASET_INFO *x, DATASET_INFO *y);
static void doLegendLineItemPS(FILE *fp, PLOT_INFO *plot, DATASET_INFO *set,
				int ylegend, int color);

extern float	*NextColorRGB_PS(), *GetColorRGB_PS(int);


/* -------------------------------------------------------------------- */
void PrintXY()
{
  bool	label;
  int	i;
  FILE	*fp;

  ResizePSxyyPlot();

  if ((fp = openPSfile(outFile)) == NULL)
    return;

  PSheader(fp, &xyyPlot[0]);
  PStitles(fp, &xyyPlot[0]);

  for (currentPanel = 0; currentPanel < NumberOfXYpanels; ++currentPanel)
    {
    PSbox(fp, &xyyPlot[currentPanel]);

    /* Then move the origin
     */
    fprintf(fp, "%d %d translate\n",
		xyyPlot[currentPanel].ps.LV, xyyPlot[currentPanel].ps.BH);

    PSlabels(fp, &xyyPlot[currentPanel]);
    fprintf(fp, "stroke 1 setlinewidth\n");
    PSxTics(fp, &xyyPlot[currentPanel], True);

    if (allLabels || currentPanel == 0)
      label = True;
    else
      label = False;

    PSyTics(fp, &xyyPlot[currentPanel], 0, label);

    for (i = 0; i < NumberXYYsets; ++i)
      if (xyYset[i].scaleLocation == RIGHT_SIDE)
        PSyTics(fp, &xyyPlot[currentPanel], 1, label);

    fprintf(fp, "stroke 0 0 moveto\n");

    PSplotXY(fp, &xyyPlot[currentPanel]);

    if (WindBarbs)
      PlotWindBarbs(&xyyPlot[currentPanel], fp);

    DrawGeoPolMapXY(&xyyPlot[currentPanel], fp);
    PlotLandMarksXY(&xyyPlot[currentPanel], fp);

    fprintf(fp, "%d %d translate\n",
		-xyyPlot[currentPanel].ps.LV, -xyyPlot[currentPanel].ps.BH);
    }

  UpdateAnnotationsPS(&xyyPlot[0], fp);
  closePSfile(fp);

}	/* END PRINTXY */

/* -------------------------------------------------------------------- */
static void ResizePSxyyPlot()
{
  int	i, totalHD;

  /* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
   * (0,0) is in the lower left corner of the paper, held at portrait.
   */
  SetPlotRatios(&xyyPlot[0]);

  if (NumberOfXYpanels == 1)
    {
    xyyPlot[0].ps.titleOffset    = (int)(xyyPlot[0].ps.windowHeight * 0.82);
    xyyPlot[0].ps.subTitleOffset = (int)(xyyPlot[0].ps.windowHeight * 0.78);
    }

  totalHD = xyyPlot[0].ps.windowWidth - (int)(200 * printerSetup.widthRatio);
 
  for (i = 0; i < NumberOfXYpanels; ++i)
    {
    if (NumberOfXYpanels == 1)
      {
      xyyPlot[i].ps.LV = (int)(xyyPlot[0].ps.windowWidth * 0.2);
      xyyPlot[i].ps.HD = (int)(xyyPlot[0].ps.windowWidth * 0.6);
 
      xyyPlot[i].ps.TH = (int)(xyyPlot[0].ps.windowHeight * 0.71);
      xyyPlot[i].ps.VD = (int)(xyyPlot[0].ps.windowHeight * 0.4735);

      xyyPlot[i].ps.xLabelOffset = (int)(-160 * printerSetup.fontRatio);
      xyyPlot[i].ps.yLabelOffset = (int)(-200 * printerSetup.fontRatio);
      xyyPlot[i].ps.xLegendText  = (int)(-100 * printerSetup.widthRatio);
      }
    else
      {
      xyyPlot[i].ps.LV = (int)(totalHD / NumberOfXYpanels * i +
					(300 * printerSetup.heightRatio));
      xyyPlot[i].ps.HD = (int)(totalHD / NumberOfXYpanels -
					(300 * printerSetup.heightRatio));
 
      xyyPlot[i].ps.TH = (int)(xyyPlot[0].ps.windowHeight * 0.8);
      xyyPlot[i].ps.VD = (int)(xyyPlot[0].ps.windowHeight * 0.6);

      xyyPlot[i].ps.xLabelOffset = (int)(-140 * printerSetup.fontRatio);
      xyyPlot[i].ps.yLabelOffset = (int)(-180 * printerSetup.fontRatio);
      xyyPlot[i].ps.xLegendText  = (int)((xyyPlot[i].x.HD >> 1) -
			(100 * printerSetup.widthRatio));
      }
 
    xyyPlot[i].ps.RV = xyyPlot[i].ps.LV + xyyPlot[i].ps.HD;
    xyyPlot[i].ps.BH = xyyPlot[i].ps.TH - xyyPlot[i].ps.VD;

    xyyPlot[i].ps.ticLength		= (int)(25 * printerSetup.fontRatio);
    xyyPlot[i].ps.yTicLabelOffset	= (int)(-15 * printerSetup.fontRatio);
    xyyPlot[i].ps.xTicLabelOffset	= (int)(-45 * printerSetup.fontRatio);
    }

}	/* END RESIZEPSXYPLOT */

/* -------------------------------------------------------------------- */
static void PSplotXY(FILE *fp, PLOT_INFO *plot)
{
  char		*p;
  DATASET_INFO	*xSet, *ySet;
  bool		xChanged, yChanged;
  struct axisInfo *yAxis, *xAxis;
  int		i, nPts, inX, inY, x, y, prevX, prevY, pCnt, xset, yset,
		plotNum, n;
  float		ratioX, ratioY, *rgb, xMin, xMax, yMin, yMax;
  double	xScale, yScale, datumY, datumX;


  /* Do Statistics first (clipping).
   */
  fprintf(fp, "stroke 5 setlinewidth\n");

  xset = yset = -1;
  n = MAX(NumberXYXsets, NumberXYYsets);

  for (CurrentDataSet = 0, plotNum = 0; plotNum < n; ++plotNum)
    {
    xChanged = yChanged = False;
 
    for (i = xset+1; i < NumberXYXsets; ++i)
      if (xyXset[i].panelIndex == currentPanel)
        {
        xChanged = True;
        xset = i;
        break;
        }
 
    for (i = yset+1; i < NumberXYYsets; ++i)
      if (xyYset[i].panelIndex == currentPanel)
        {
        yChanged = True;
        yset = i;
        break;
        }

    if (xset == -1 || yset == -1 || (!xChanged && !yChanged))
      break;

    xSet = &xyXset[xset];
    ySet = &xyYset[yset];

    doLegendLineItemPS(fp, plot, xSet, yLegendPS(plot,CurrentDataSet+2), False);
    ++CurrentDataSet;

    if (printerSetup.color)
      {
      rgb = GetColorRGB_PS(plotNum+1);
      fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0], rgb[1], rgb[2]);
      }

    doLegendLineItemPS(fp, plot, ySet, yLegendPS(plot,CurrentDataSet+2), True);
    ++CurrentDataSet;

    if (!printerSetup.color)
        fprintf(fp, "stroke [%d] 0 setdash\n", (plotNum+1) << 3);
    }

  fprintf(fp, "stroke 0 0 0 setrgbcolor\n");

  if (Statistics)
    PSstatsTitle(fp, plot, CurrentDataSet+3);

  fprintf(fp, "1 setlinewidth\n");
  fprintf(fp, "[] 0 setdash\n");
  PSclip(fp, plot);

  if (plot->Xaxis.logScale)
    {
    xMin = log10(plot->Xaxis.min);
    xMax = log10(plot->Xaxis.max);
    }
  else
    {
    xMin = plot->Xaxis.min;
    xMax = plot->Xaxis.max;
    }

  if (plot->Yaxis[0].logScale)
    {
    yMin = log10(plot->Yaxis[0].min);
    yMax = log10(plot->Yaxis[0].max);
    }
  else
    {
    yMin = plot->Yaxis[0].min;
    yMax = plot->Yaxis[0].max;
    }

  ResetColors();
  fprintf(fp, "%d setlinewidth\n", LineThickness<<1);

  xScale = plot->ps.HD / (xMax - xMin);

  xset = yset = -1;

  for (plotNum = 0; plotNum < n; ++plotNum)
    {
    xChanged = yChanged = False;
 
    for (i = xset+1; i < NumberXYXsets; ++i)
      if (xyXset[i].panelIndex == currentPanel)
        {
        xChanged = True;
        xset = i;
        break;
        }
 
    for (i = yset+1; i < NumberXYYsets; ++i)
      if (xyYset[i].panelIndex == currentPanel)
        {
        yChanged = True;
        yset = i;
        break;
        }
 
    if (xset == -1 || yset == -1 || (!xChanged && !yChanged))
      break;

    xSet = &xyXset[xset];
    ySet = &xyYset[yset];
    xAxis = &plot->Xaxis;
    yAxis = &plot->Yaxis[ySet->scaleLocation];

    ratioX = ratioY = 1.0;
    nPts = MAX(xSet->nPoints, ySet->nPoints);
    yScale = plot->ps.VD / (yMax - yMin);

    if (xSet->nPoints > ySet->nPoints)
      ratioY = (float)ySet->nPoints / xSet->nPoints;

    if (xSet->nPoints < ySet->nPoints)
      ratioX = (float)xSet->nPoints / ySet->nPoints;

    if (printerSetup.color)
      {
      rgb = NextColorRGB_PS();

      fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0], rgb[1], rgb[2]);
      }


    pCnt = 0;

    for (i = 0; i < nPts; ++i)
      {
      if (isMissingValue(xSet->data[(xSet->head + i) % xSet->nPoints],
						xSet->missingValue) || i == 0)
        {
        while (isMissingValue(xSet->data[(xSet->head + i) % xSet->nPoints],
						xSet->missingValue) && i < nPts)
          ++i;

        p = (char *)moveto;
        }
      else
        p = (char *)lineto;

      inX = (int)(xSet->head + (i * ratioX)) % xSet->nPoints;
      inY = (int)(ySet->head + (i * ratioY)) % ySet->nPoints;

      datumX = xSet->data[inX];
      datumY = ySet->data[inY];

      if (xAxis->logScale)
        {
        if (datumX <= 0.0)
          datumX = xMin;
        else
          datumX = log10(datumX);
        }

      if (yAxis->logScale)
        {
        if (datumY <= 0.0)
          datumY = yMin;
        else
          datumY = log10(datumY);
        }

      if (xAxis->invertAxis)
        x = (int)(xScale * (xMax - datumX));
      else
        x = (int)(xScale * (datumX - xMin));

      if (yAxis->invertAxis)
        y = (int)(yScale * (yMax - datumY));
      else
        y = (int)(yScale * (datumY - yMin));

      /* Don't print duplicate points. */
      if (!(x == prevX && y == prevY))
        {
        if (ScatterPlot)
          {
          fprintf(fp, moveto, x-1, y-1);
          fprintf(fp, lineto, x+1, y-1);
          fprintf(fp, lineto, x+1, y);
          fprintf(fp, lineto, x-1, y);
          fprintf(fp, lineto, x-1, y+1);
          fprintf(fp, lineto, x+1, y+1);
          }
        else
          fprintf(fp, p, x, y);

        prevX = x;
        prevY = y;
        ++pCnt;
        }

      if (nDirectionArrows && (i+1) % (nPts / nDirectionArrows) == 0)
        PlotDirectionArrow(plot, x, y,
            (int)(xScale * (xSet->data[inX-4] - xAxis->min)),
            (int)(yScale * (ySet->data[inY-4] - yAxis->min)),
            fp);

      if (nTimeStamps && (i == 0 || (i+1) % (nPts / nTimeStamps) == 0))
        PlotTimeStamps(plot, x, y, (i+1) / (nPts / nTimeStamps), fp);


      if (!(pCnt % 1024)) /* Some printers can't handle infinite lineto's */
        {
        fprintf(fp, "stroke\n");
        fprintf(fp, moveto, x, y);
        ++pCnt;
        }
      }


    if (ShowRegression)
      PSplotRegression(fp, plot, xSet, ySet);

    if (!printerSetup.color)
      fprintf(fp, "stroke [%d] 0 setdash\n", (plotNum+1) << 3);
    }


  fprintf(fp, "1 setlinewidth\n");

  if (printerSetup.color)
    fprintf(fp, "stroke 0 0 0 setrgbcolor\n");
  else
    fprintf(fp, "stroke [] 0 setdash\n");

  PSclearClip(fp);

}	/* END PSPLOTXY */

/* -------------------------------------------------------------------- */
static void PSplotRegression(FILE *fp, PLOT_INFO *plot, DATASET_INFO *x, DATASET_INFO *y)
{
  int	i, j, y1, y2;
  float	xMin = plot->Xaxis.min;
  float	xMax = plot->Xaxis.max;
  float	yMin = plot->Yaxis[0].min;
  float	yScale = (float)plot->ps.VD / (plot->Yaxis[0].max - yMin);
  float	yInterMin, yInterMax;

  void fitcurve(DATASET_INFO *x, DATASET_INFO *y, int ideg);
 
  printf("X axis variable: %s from %s\n",
		x->varInfo->name, dataFile[x->fileIndex].fileName);
  printf("Y axis variable: %s from %s\n",
		y->varInfo->name, dataFile[y->fileIndex].fileName);

  fitcurve(x, y, ShowRegression);

  if (ShowRegression == 1)	/* Linear */
    {
    yInterMin = yScale * (regretCo[0] + (regretCo[1] * xMin) - yMin);
    yInterMax = yScale * (regretCo[0] + (regretCo[1] * xMax) - yMin);

    if (plot->Yaxis[0].invertAxis)
      {
      y1 = plot->ps.VD - (int)yInterMin;
      y2 = plot->ps.VD - (int)yInterMax;
      }
    else
      {
      y1 = (int)yInterMin;
      y2 = (int)yInterMax;
      }

    fprintf(fp, moveto, 0, (int)y1);
    fprintf(fp, lineto, (int)plot->ps.HD, (int)y2);
    }
  else
    {
    float	inc = (xMax - xMin) / plot->ps.HD * 4;

    xMax = xMin + inc;

    for (i = 0; i < plot->ps.HD; i += 4)
      {
      yInterMin = regretCo[0] + (regretCo[1] * xMin);
      yInterMax = regretCo[0] + (regretCo[1] * xMax);

      for (j = 2; j <= ShowRegression; ++j)
        {
        yInterMin += regretCo[j] * pow(xMin, (double)j);
        yInterMax += regretCo[j] * pow(xMax, (double)j);
        }

      yInterMin = yScale * (yInterMin - yMin);
      yInterMax = yScale * (yInterMax - yMin);

      if (plot->Yaxis[0].invertAxis)
        {
        y1 = plot->ps.VD - (int)yInterMin;
        y2 = plot->ps.VD - (int)yInterMax;
        }
      else
        {
        y1 = (int)yInterMin;
        y2 = (int)yInterMax;
        }

      fprintf(fp, moveto, i, (int)y1);
      fprintf(fp, lineto, i+4, (int)y2);

      xMin = xMax; xMax += inc;
      }
    }

}	/* END PSPLOTREGRESSION */

/* -------------------------------------------------------------------- */
static void doLegendLineItemPS(FILE *fp, PLOT_INFO *plot, DATASET_INFO *set, int y, int showLine)
{
  int	x;

  if (Statistics)
    {
    if (set->scaleLocation == RIGHT_SIDE)
      x = plot->ps.xLegendText + (int)(1950 * printerSetup.widthRatio);
    else
      x = plot->ps.xLegendText;
 
    if (showLine)
      {
      fprintf(fp, moveto, (int)(x - (180 * printerSetup.widthRatio)), y+10);
      fprintf(fp, lineto, (int)(x - (20 * printerSetup.widthRatio)), y+10);
      }

    sprintf(buffer, "%s (%s), %d s/sec",
	set->varInfo->name, set->stats.units, set->varInfo->OutputRate);

    PSstatsLegend(fp, plot, buffer, CurrentDataSet+2, set);
    }
  else
    {
    sprintf(buffer, "%s (%s)", set->varInfo->name, set->stats.units);

    if (showLine)
      {
      if (set->scaleLocation == LEFT_SIDE)
        fprintf(fp, moveto, (int)(plot->ps.xLegendText -
		(180 * printerSetup.widthRatio)), y + 10);
      else
        fprintf(fp, "%d (%s) stringwidth pop add %d moveto\n",
		(int)(plot->ps.xLegendText+20), buffer, y + 10);
 
      fprintf(fp, "%d %d rlineto\n", (int)(160 * printerSetup.widthRatio), 0);
      }

    if (printerSetup.color)
      fprintf(fp, "stroke\n0 0 0 setrgbcolor\n");
 
    sprintf(buffer, "%s (%s)", set->varInfo->name, set->stats.units);

    fprintf(fp, moveto, plot->ps.xLegendText, y);
    fprintf(fp, show, buffer);
    }

}	/* END DOLEGENDLINEITEMPS */

/* END XYPS.C */
