/*
-------------------------------------------------------------------------
OBJECT NAME:	plotPS.c

FULL NAME:	Produce PostScript File

ENTRY POINTS:	PrintTimeSeries()

STATIC FNS:	doLineGraph()
		ResizePSmainPlot()

DESCRIPTION:	Generates native PostScript of the Time Series window.

REFERENCES:	ps.c

REFERENCED BY:	PrintPostScript()

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

static int	currentPanel;

static void	doLineGraph(FILE *fp, PLOT_INFO *plot), ResizePSmainPlot();

/* -------------------------------------------------------------------- */
static void ResizePSmainPlot()
{
  int	i, totalVD;

  /* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
   * (0,0) is in the lower left corner of the paper, held at portrait.
   */
  SetPlotRatios(&mainPlot[0]);

  totalVD = (int)(mainPlot[0].ps.windowHeight - (400 * printerSetup.heightRatio));

  for (i = 0; i < NumberOfPanels; ++i)
    {
    mainPlot[i].ps.LV = (int)(mainPlot[0].ps.windowWidth * 0.1364);
    mainPlot[i].ps.HD = (int)(mainPlot[0].ps.windowWidth * 0.7273);	/* 8" */

    if (NumberOfPanels == 1)
      {
      mainPlot[i].ps.TH = (int)(mainPlot[0].ps.windowHeight * 0.2089);
      mainPlot[i].ps.VD = (int)(mainPlot[0].ps.windowHeight * 0.5822); /* 5" */
      mainPlot[i].ps.TH += mainPlot[i].ps.VD;
      }
    else
      {
      mainPlot[i].ps.TH = (int)(totalVD - totalVD / NumberOfPanels * i) + 50;
      mainPlot[i].ps.VD = (int)(totalVD / NumberOfPanels -
                                    (160 * printerSetup.heightRatio));
      }

    mainPlot[i].ps.RV = mainPlot[i].ps.LV + mainPlot[i].ps.HD;
    mainPlot[i].ps.BH = mainPlot[i].ps.TH - mainPlot[i].ps.VD;

    if (NumberOfPanels == 1)
      mainPlot[i].ps.xLabelOffset = (int)(-100 * printerSetup.fontRatio);
    else
      mainPlot[i].ps.xLabelOffset = (int)(-45 * printerSetup.fontRatio);

    mainPlot[i].ps.yLabelOffset = (int)(-200 * printerSetup.fontRatio);

    mainPlot[i].ps.ticLength		= (int)(25 * printerSetup.fontRatio);
    mainPlot[i].ps.yTicLabelOffset	= (int)(-15 * printerSetup.fontRatio);
    mainPlot[i].ps.xTicLabelOffset	= (int)(-45 * printerSetup.fontRatio);

    mainPlot[i].ps.xLegendText		= (int)(180 * printerSetup.widthRatio);
    }

}	/* END RESIZEPSMAINPLOT */

/* -------------------------------------------------------------------- */
void PrintTimeSeries()
{
  int	i;
  FILE	*fp;

  ResizePSmainPlot();

  if ((fp = openPSfile(outFile)) == NULL)
    return;

  PSheader(fp, &mainPlot[0]);
  PStitles(fp, &mainPlot[0]);

  for (currentPanel = 0; currentPanel < NumberOfPanels; ++currentPanel)
    {
    PSbox(fp, &mainPlot[currentPanel]);

    /* Move origin to (0,0) of plot.
     */
    fprintf(fp, "%d %d translate\n",
            mainPlot[currentPanel].ps.LV,
            mainPlot[currentPanel].ps.BH);

    PSlabels(fp, &mainPlot[currentPanel]);
    fprintf(fp, "1 setlinewidth\n");

    if (allLabels || currentPanel == NumberOfPanels-1)
      PSxTics(fp, &mainPlot[currentPanel], True);
    else
      PSxTics(fp, &mainPlot[currentPanel], False);

    PSyTics(fp, &mainPlot[currentPanel], 0, True);

    for (i = 0; i < NumberDataSets; ++i)
      if (dataSet[i].scaleLocation == RIGHT_SIDE &&
          dataSet[i].panelIndex == currentPanel)
        PSyTics(fp, &mainPlot[currentPanel], 1, True);

    fprintf(fp, "stroke 0 0 moveto\n");

    doLineGraph(fp, &mainPlot[currentPanel]);

    fprintf(fp, "%d %d translate\n",
            -mainPlot[currentPanel].ps.LV,
            -mainPlot[currentPanel].ps.BH);
    }

  UpdateAnnotationsPS(&mainPlot[0], fp);
  closePSfile(fp);

}	/* END PRINTPOSTSCRIPT */

/* -------------------------------------------------------------------- */
static void doLineGraph(FILE *fp, PLOT_INFO *plot)
{
  char		*p;
  int		i;
  float		halfSecond, *rgb, *NextColorRGB_PS(), yMin, yMax;
  int		x, y, prevX, prevY, pCnt, lCnt, rCnt;
  double	xScale, yScale, datumY;
  VARTBL	*vp;
  DATASET_INFO	*set;
  struct axisInfo *yAxis;

  ResetColors();
  fprintf(fp, "stroke 5 setlinewidth\n");

  /* Print legend.
   */
  lCnt = rCnt = 0;

  for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
    {
    if (dataSet[CurrentDataSet].panelIndex != currentPanel)
      continue;

    set = &dataSet[CurrentDataSet];
    vp = set->varInfo;
    y = yLegendPS(plot, CurrentDataSet) + 10;

    if (printerSetup.color)
      {
      rgb = NextColorRGB_PS();
      fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0],rgb[1],rgb[2]);
      }


    if (Statistics)
      {
      if (set->scaleLocation == RIGHT_SIDE)
        x = plot->ps.xLegendText + (int)(1950 * printerSetup.widthRatio);
      else
        x = plot->ps.xLegendText;

      fprintf(fp, moveto, (int)(x - (180 * printerSetup.widthRatio)), y);
      fprintf(fp, lineto, (int)(x - (20 * printerSetup.widthRatio)), y);

      sprintf(buffer, "%s (%s), %d s/sec",
		vp->name, set->stats.units, vp->OutputRate);
      PSstatsLegend(fp, plot, buffer, CurrentDataSet, set);
      }
    else
      {
      if (set->scaleLocation == LEFT_SIDE)
        x = lCnt++ * 400 + 130;
      else
        x = plot->ps.HD - ++rCnt * 400 + 150;

      fprintf(fp, moveto, (int)(x - (130 * printerSetup.widthRatio)), y+10);
      fprintf(fp, lineto, (int)(x - (20 * printerSetup.widthRatio)), y+10);

      if (printerSetup.color)
        fprintf(fp, "stroke\n0 0 0 setrgbcolor\n");

      fprintf(fp, moveto, x, y);
      fprintf(fp, show, vp->name);
      }


    if (!printerSetup.color)
      fprintf(fp, "stroke [%d] 0 setdash\n", (CurrentDataSet+1) << 3);
    }

  fprintf(fp, "stroke 0 0 0 setrgbcolor\n");

  if (Statistics)
    PSstatsTitle(fp, plot, CurrentDataSet+1);


  fprintf(fp, "%d setlinewidth\n", LineThickness<<1);
  fprintf(fp, "[] 0 setdash\n");
  PSclip(fp, plot);

  ResetColors();

  for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
    {
    set = &dataSet[CurrentDataSet];
    yAxis = &plot->Yaxis[set->scaleLocation];

    if (set->panelIndex != currentPanel)
      continue;

    /* Set the scale factor to number of pixels divided by the
    * number of divisions
    */
    xScale = (NR_TYPE)plot->ps.HD / set->nPoints;

    if (yAxis->logScale)
      {
      yMin = log10(yAxis->min);
      yMax = log10(yAxis->max);
      yScale = (float)plot->ps.VD / (log10(yAxis->max) - yMin);
      }
    else
      {
      yMin = yAxis->min;
      yMax = yAxis->max;
      yScale = (float)plot->ps.VD / (yAxis->max - yMin);
      }

    if (set->nPoints == NumberSeconds)
      halfSecond = plot->ps.HD / NumberSeconds / 2;
    else
      if (set->nPoints < NumberSeconds)
        halfSecond = (plot->ps.HD / NumberSeconds) *
                     (dataFile[set->fileIndex].baseDataRate / 2);
      else
        halfSecond = 0.0;

    if (printerSetup.color)
      {
      rgb = NextColorRGB_PS();
      fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0], rgb[1], rgb[2]);
      }

    pCnt = 0;

    for (i = 0; i < set->nPoints; ++i)
      {
      if (set->data[(set->head + i) % set->nPoints] == set->missingValue || i == 0)
        {
        while (set->data[(set->head + i) % set->nPoints] ==
                set->missingValue && i < set->nPoints)
          ++i;

        if (i > 0 && p == moveto)
          {
          fprintf(fp, moveto, x-4, y);
          fprintf(fp, lineto, x+4, y);
          fprintf(fp, moveto, x, y-4);
          fprintf(fp, lineto, x, y+4);
          }

        p = (char *)moveto;
        }
      else
        p = (char *)lineto;

      x = (int)(xScale * i + halfSecond);
      datumY = set->data[(set->head + i) % set->nPoints];

      if (yAxis->logScale)
        {
        if (datumY <= 0.0)
          datumY = yMin;
        else
          datumY = log10(datumY);
        }

      if (yAxis->invertAxis)
        y = (int)(yScale * (yMax - datumY));
      else
        y = (int)(yScale * (datumY - yMin));

      if (!(x == prevX && y == prevY))
        {
        fprintf(fp, p, x, y);
        prevX = x;
        prevY = y;
        ++pCnt;
        }

      if (!(pCnt % 1024))
        {
        fprintf(fp, "stroke\n");
        fprintf(fp, moveto, x, y);
        ++pCnt;
        }
      }

    if (!printerSetup.color)
      fprintf(fp, "stroke [%d] 0 setdash\n", (CurrentDataSet + 1) << 3);
    }

  PSclearClip(fp);
  fprintf(fp, "1 setlinewidth\n");

  if (printerSetup.color)
    fprintf(fp, "0 0 0 setrgbcolor\n");
  else
    fprintf(fp, "[] 0 setdash\n");

}	/* END DOLINEGRAPH */

/* END PLOTPS.C */
