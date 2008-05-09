/*
-------------------------------------------------------------------------
OBJECT NAME:	specPS.c

FULL NAME:	Produce PostScript File of Spectra Plot

ENTRY POINTS:	specPostScript()

STATIC FNS:	printSemiLog()
		printLogLog()
		doWaveLengthPS()

DESCRIPTION:	This does the PostScript printing.

REFERENCES:	ps.c

REFERENCED BY:	spec Print Button

AUTHOR:		websterc@ncar.ucar.edu

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"
#include "ps.h"

static float	numXpix, numYpix;

static void	printSemiLog(FILE *fp, PLOT_INFO *plot, double *dataP);
static void	printLogLog(FILE *fp, PLOT_INFO *plot, double *dataP);
static void	doWaveLengthPS(FILE *fp, PLOT_INFO *plot);


/* -------------------------------------------------------------------- */
static void ResizePSspecPlot()
{
  /* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
   * (0,0) is in the lower left corner of the paper, held at portrait.
   */
  printerSetup.shape = PORTRAIT;
  SetPlotRatios(&specPlot);

  specPlot.ps.titleOffset      = (int)(2600 * printerSetup.heightRatio);
  specPlot.ps.subTitleOffset   = (int)(2500 * printerSetup.heightRatio);

  specPlot.ps.LV = (int)(500 * printerSetup.widthRatio);
  specPlot.ps.RV = (int)(2000 * printerSetup.widthRatio);
  specPlot.ps.BH = (int)(750 * printerSetup.heightRatio);
  specPlot.ps.TH = (int)(2250 * printerSetup.heightRatio);

  specPlot.ps.HD = specPlot.ps.RV - specPlot.ps.LV;
  specPlot.ps.VD = specPlot.ps.TH - specPlot.ps.BH;

  specPlot.ps.ticLength		= (int)(25 * printerSetup.fontRatio);
  specPlot.ps.xLabelOffset	= (int)(-110 * printerSetup.fontRatio);
  specPlot.ps.yLabelOffset	= (int)(-250 * printerSetup.fontRatio);
  specPlot.ps.yTicLabelOffset	= (int)(-15 * printerSetup.fontRatio);
  specPlot.ps.xTicLabelOffset	= (int)(-45 * printerSetup.fontRatio);

  specPlot.ps.xLegendText	= (int)(-300 * printerSetup.widthRatio);
}

/* -------------------------------------------------------------------- */
void specPostScript(Widget w, XtPointer client, XtPointer call)
{
  FILE	*fp;
  int	save = printerSetup.shape;

  if (call)
    CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

  ResizePSspecPlot();

  if ((fp = openPSfile(outFile)) == NULL)
    return;

  PSheader(fp, &specPlot);

  bool warning = false;
  if (dataFile[dataSet[0].fileIndex].ShowPrelimDataWarning ||
        (psd[0].display != SPECTRA &&
        dataFile[dataSet[1].fileIndex].ShowPrelimDataWarning))
    warning = true;

  PStitles(fp, &specPlot, warning);
  PSbox(fp, &specPlot);

  numXpix = (float)specPlot.ps.HD / specPlot.Xaxis.nMajorTics;
  numYpix = (float)specPlot.ps.VD / specPlot.Yaxis[0].nMajorTics;

  /* Then move the origin.
   */
  fprintf(fp, "%d %d translate\n", specPlot.ps.LV, specPlot.ps.BH);

  PSlabels(fp, &specPlot);
  fprintf(fp, "1 setlinewidth\n");

  PSyTics(fp, &specPlot, 0, True);
  PSxTics(fp, &specPlot, True);

printf("specPS.c:  No multiple spec print, when added, make sure psd[0] -> psd[set]\n");

  if (psd[0].display == SPECTRA || psd[0].display == COSPECTRA)
    PlotVariancePS(&specPlot, fp);

  fprintf(fp, "stroke 0 0 moveto\n");

  fprintf(fp, "newpath\n");
  fprintf(fp, "[] 0 setdash\n");

  switch (psd[0].display)
    {
    case SPECTRA:
      printLogLog(fp, &specPlot, psd[0].Pxx);
      break;

    case COSPECTRA:
      printSemiLog(fp, &specPlot, psd[0].Pxx);
      break;

    case QUADRATURE:
      printSemiLog(fp, &specPlot, psd[0].Qxx);
      break;

    case COHERENCE:
    case PHASE:
    case RATIO:
      printSemiLog(fp, &specPlot, psd[0].Special);
      break;
    }

  closePSfile(fp);
  printerSetup.shape = save;

}	/* END SPECPOSTSCRIPT */

/* -------------------------------------------------------------------- */
static void printLogLog(FILE *fp, PLOT_INFO *plot, double *dataP)
{
  char		*p;
  size_t	i, nPts;
  float		x, y;
  double	freq, waveNumber = 0, *plotData, datumY, xMin, yMin, xScale,
		yScale;

  PSclip(fp, plot);

  if (equalLogInterval())
    {
    nPts = psd[0].ELIAcnt - 1;
    plotData = psd[0].ELIAy;
    }
  else
    {
    nPts = psd[0].M;
    plotData = dataP;
    }

  if (plotWaveNumber())
    waveNumber = 2.0 * M_PI / tas.stats.mean;

  if (plot->Xaxis.logScale)
  {
    xMin = log10(plot->Xaxis.min);
    xScale = (float)plot->ps.HD / (log10(plot->Xaxis.max) - xMin);
  }
  else
  {
    xMin = plot->Xaxis.min;
    xScale = (float)plot->ps.HD / (plot->Xaxis.max - xMin);
  }

  if (plot->Yaxis[0].logScale)
  {
    yMin = log10(plot->Yaxis[0].min);
    yScale = (float)plot->ps.VD / (log10(plot->Yaxis[0].max) - yMin);
  }
  else
  {
    yMin = plot->Yaxis[0].min;
    yScale = (float)plot->ps.VD / (plot->Yaxis[0].max - yMin);
  }


  fprintf(fp, "%ld setlinewidth\n", LineThickness<<1);
  for (i = 1; i <= nPts; ++i)
    {
    if (equalLogInterval())
      freq = psd[0].ELIAx[i];
    else
      freq = psd[0].freqPerBin * i;

    if (plotWaveNumber())
      freq *= waveNumber;

    if (plot->Xaxis.logScale)
      freq = log10(freq);

    if (plot->Yaxis[0].logScale)
      datumY = log10(plotData[i]);
    else
      datumY = plotData[i];

    if (i == 1)
      p = (char *)moveto;
    else
      p = (char *)lineto;

    x = xScale * (freq - xMin);
    y = yScale * (datumY - yMin);

    fprintf(fp, p, (int)x, (int)y);

    if (!(i % 128))
      {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, (int)x, (int)y);
      }
    }

  fprintf(fp, "1 setlinewidth\n");


  /* Drop band limited variance lines.
   */
  for (i = 1; i <= psd[0].M; ++i)
    {
    if ((size_t)(startFreq() / psd[0].freqPerBin) == i ||
        (size_t)(endFreq() / psd[0].freqPerBin) == i)
      {
      freq = psd[0].freqPerBin * i;

      if (plotWaveNumber())
        freq *= waveNumber;

      if (plot->Xaxis.logScale)
	freq = log10(freq);

      if (plot->Yaxis[0].logScale)
	datumY = log10(dataP[i]);
      else
	datumY = dataP[i];

      x = numXpix * (freq - xMin);
      y = numYpix * (datumY - yMin);

      fprintf(fp, moveto, (int)x, 0);
      fprintf(fp, lineto, (int)x, (int)y);
      }
    }


  /* Draw -5/3 slope.
   */
  if (plotFiveThirds())
    {
    fprintf(fp, moveto, (int)xScale * 3, 0);

    if (multiplyByFreq())
      fprintf(fp, lineto, 0, (int)yScale * 2);
    else
      fprintf(fp, lineto, 0, (int)yScale * 5);
    }

  PSclearClip(fp);


  if (plotWaveLength())
    doWaveLengthPS(fp, plot);

}	/* END PRINTLOGLOG */

/* -------------------------------------------------------------------- */
static void printSemiLog(FILE *fp, PLOT_INFO *plot, double *dataP)
{
  char		*p;
  size_t	i, nPts;
  float		x, y;
  double	freq, waveNumber = 0, *plotData, yScale, xScale, xMin;

  if (equalLogInterval())
    {
    nPts = psd[0].ELIAcnt - 1;
    plotData = psd[0].ELIAy;
    }
  else
    {
    nPts = psd[0].M;
    plotData = dataP;
    }

  /* Set the scale factor to number of pixels divided by the
   * number of divisions
   */
  if (plot->Xaxis.logScale)
  {
    xMin = log10(plot->Xaxis.min);
    xScale = (NR_TYPE)plot->ps.HD / (log10(plot->Xaxis.max) - xMin);
  }
  else
  {
    xMin = plot->Xaxis.min;
    xScale = (NR_TYPE)plot->ps.HD / (plot->Xaxis.max - xMin);
  }

  yScale = (NR_TYPE)plot->ps.VD / (plot->Yaxis[0].max - plot->Yaxis[0].min);

  if (plotWaveNumber())
    waveNumber = 2.0 * M_PI / tas.stats.mean;

  PSclip(fp, plot);
  fprintf(fp, "%ld setlinewidth\n", LineThickness<<1);

  for (i = 1; i <= nPts; ++i)
    {
    if (equalLogInterval())
      freq = psd[0].ELIAx[i];
    else
      freq = psd[0].freqPerBin * i;

    if (plotWaveNumber())
      freq *= waveNumber;

    if (plot->Xaxis.logScale)
      freq = log10(freq);

    if (i == 1)
      p = (char *)moveto;
    else
      p = (char *)lineto;

    x = xScale * (freq - xMin);
    y = yScale * (plotData[i] - plot->Yaxis[0].min);

    fprintf(fp, p, (int)x, (int)y);

    if (!(i % 128))
      {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, (int)x, (int)y);
      }
    }

  fprintf(fp, "1 setlinewidth\n");


  /* Drop band limited variance lines.
   */
  for (i = 1; i <= psd[0].M; ++i)
    {
    if ((size_t)(startFreq() / psd[0].freqPerBin) == i ||
        (size_t)(endFreq() / psd[0].freqPerBin) == i)
      {
      freq = psd[0].freqPerBin * i;

      if (plotWaveNumber())
        freq *= waveNumber;

      if (plot->Xaxis.logScale)
        freq = log10(freq);

      x = xScale * (freq - xMin);
      y = yScale * (dataP[i] - plot->Yaxis[0].min);

      fprintf(fp, moveto, (int)x, 0);
      fprintf(fp, lineto, (int)x, (int)y);
      }
    }


  PSclearClip(fp);

  if (plotWaveLength())
    doWaveLengthPS(fp, plot);

}	/* END PRINTSEMILOG */

/* -------------------------------------------------------------------- */
static void doWaveLengthPS(FILE *fp, PLOT_INFO *plot)
{
  int           i, x, xDecade, nXdecades;
  double        xMantissa;
 
 
  /* Plot Wave Length Scale.
   */
  xMantissa = log10(tas.stats.mean);
  xDecade = (int)xMantissa;
  xMantissa -= xDecade;
  nXdecades = (int)(xDecade - log10(plot->Xaxis.min)) + 1;
 
  x = (int)(xMantissa * numXpix);
 
  for (i = 0; i < plot->Xaxis.nMajorTics; ++i)
    {
    fprintf(fp, moveto, x, plot->ps.VD);
    fprintf(fp, lineto, x, plot->ps.VD + plot->ps.ticLength);
 
    MakeLogTicLabel(buffer, nXdecades);
    fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
            x, buffer, plot->ps.VD + plot->ps.ticLength+5);
    fprintf(fp, show, buffer);

    x += (int)numXpix;
    --nXdecades;
    }

  strcpy(buffer, "Wave length (M)");
  fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
        plot->ps.HD >> 1, buffer,
        plot->ps.VD + plot->ps.ticLength + (int)(50 * printerSetup.fontRatio));
  fprintf(fp, show, buffer);

}	/* END DOWAVELENGTH */

/* END SPECPS.C */
