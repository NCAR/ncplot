/*
-------------------------------------------------------------------------
OBJECT NAME:	diffPS.c

FULL NAME:	Produce PostScript File of Difference Plot

ENTRY POINTS:	diffPostScript()

STATIC FNS:	doDiffGraph()

DESCRIPTION:	This does the PostScript printing.

REFERENCES:	ps.c

REFERENCED BY:	diff Print Button

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2005
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

static void	doDiffGraph(FILE *fp, PLOT_INFO *plot);


/* -------------------------------------------------------------------- */
void ResizePSdiffPlot(Widget w, XtPointer client, XtPointer call)
{
  int		save = printerSetup.shape;

  /* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
   * (0,0) is in the lower left corner of the paper, held at portrait.
   */
  printerSetup.shape = LANDSCAPE;
  SetPlotRatios(&diffPlot);
  printerSetup.shape = save;

  diffPlot.ps.titleOffset      = (int)(2100 * printerSetup.heightRatio);
  diffPlot.ps.subTitleOffset   = (int)(2000 * printerSetup.heightRatio);


  diffPlot.ps.LV = (int)(diffPlot.ps.windowWidth * 0.1364);
  diffPlot.ps.HD = (int)(diffPlot.ps.windowWidth * 0.7273);    /* 8" */
  diffPlot.ps.RV = diffPlot.ps.LV + diffPlot.ps.HD;

  diffPlot.ps.BH = (int)(600 * printerSetup.heightRatio);
  diffPlot.ps.TH = (int)(1800 * printerSetup.heightRatio);
  diffPlot.ps.VD = diffPlot.ps.TH - diffPlot.ps.BH;

  diffPlot.ps.ticLength		= (int)(25 * printerSetup.fontRatio);
  diffPlot.ps.xLabelOffset	= (int)(-100 * printerSetup.fontRatio);
  diffPlot.ps.yLabelOffset	= (int)(-200 * printerSetup.fontRatio);
  diffPlot.ps.yTicLabelOffset	= (int)(-15 * printerSetup.fontRatio);
  diffPlot.ps.xTicLabelOffset	= (int)(-45 * printerSetup.fontRatio);

  diffPlot.ps.xLegendText	= 0;

}	/* END RESIZEPSDIFFPLOT */

/* -------------------------------------------------------------------- */
void diffPostScript(Widget w, XtPointer client, XtPointer call)
{
  FILE	*fp;

  if (call)
    CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

  ResizePSdiffPlot(NULL, NULL, NULL);

  if ((fp = openPSfile(outFile)) == NULL)
    return;

  PSheader(fp, &diffPlot);
  PStitles(fp, &diffPlot, false);
  PSbox(fp, &diffPlot);

  /* Move origin to (0,0) of plot window.
   */
  fprintf(fp, "%d %d translate\n", diffPlot.ps.LV, diffPlot.ps.BH);

  PSlabels(fp, &diffPlot);
  fprintf(fp, "1 setlinewidth\n");
  PSyTics(fp, &diffPlot, 0, true);
  PSxTics(fp, &diffPlot, true);
  fprintf(fp, "stroke 0 0 moveto\n");

  doDiffGraph(fp, &diffPlot);

  closePSfile(fp);

}	/* END DIFFPOSTSCRIPT */

/* -------------------------------------------------------------------- */
static void doDiffGraph(FILE *fp, PLOT_INFO *plot)
{
  char		*p;
  int		x, y;
  NR_TYPE	xScale, yScale, halfSecond;


  /* Print legend.
   */
  sprintf(buffer, "(%s-%s), %d s/sec",
          dataSet[0].varInfo->name, dataSet[1].varInfo->name,
          dataSet[0].varInfo->OutputRate);

  PSstatsLegend(fp, plot, buffer, 0, &diffSet);
  PSstatsTitle(fp, plot, 2);


  /* Set the scale factor to number of pixels divided by the
   * number of divisions
   */
  xScale = (NR_TYPE)plot->ps.HD / dataSet[0].nPoints;
  yScale = (NR_TYPE)plot->ps.VD / (plot->Yaxis[0].max - plot->Yaxis[0].min);

  if (dataSet[0].nPoints == NumberSeconds)
    halfSecond = plot->ps.HD / NumberSeconds / 2;
  else
    if (dataSet[0].nPoints < NumberSeconds)
      halfSecond = (plot->ps.HD / NumberSeconds) *
                  (dataFile[dataSet[0].fileIndex].baseDataRate / 2);
    else
      halfSecond = 0.0;


  PSclip(fp, plot);
  fprintf(fp, "%d setlinewidth\n", LineThickness<<1);


  for (size_t i = 0; i < dataSet[0].nPoints; ++i)
    {
    if (isMissingValue(diffSet.data[i], diffSet.missingValue) || i == 0)
      {
      while (isMissingValue(diffSet.data[i], diffSet.missingValue))
        ++i;

      p = (char *)moveto;
      }
    else
      p = (char *)lineto;

    x = (int)(xScale * i + halfSecond);
    y = (int)(yScale * (diffSet.data[i] - plot->Yaxis[0].min));

    fprintf(fp, p, x, y);

    if (!(i % 128))
      {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, x, y);
      }
    }

  PSclearClip(fp);

}	/* END DODIFFGRAPH */

/* END DIFFPS.C */
