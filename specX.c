/*
-------------------------------------------------------------------------
OBJECT NAME:	specX.c

FULL NAME:	Plot Graph in X window

ENTRY POINTS:	PlotSpectrum()
		ResizeSpecWindow()
		doWaveLengthX()
		doFiveThirdsX()

STATIC FNS:	plotVariance()
		plotLogLog()
		plotSemiLog()

DESCRIPTION:	This is the Expose event procedure to regenerate the
		whole image.

AUTHOR:		cjw@ucar.edu

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2005
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

static double	xScale, yScale;
static int	fontOffset;

void	ComputeELIA(), doWaveLengthX(PLOT_INFO *), doFiveThirdsX(PLOT_INFO *);
static void	plotLogLog(PLOT_INFO *), plotSemiLog(PLOT_INFO *, double *);


/* -------------------------------------------------------------------- */
void ResizeSpecWindow(Widget w, XtPointer client, XtPointer call)
{
  int	n, depth;
  Arg	args[5];

  n = 0;
  XtSetArg(args[n], XmNwidth, &specPlot.x.windowWidth); ++n;
  XtSetArg(args[n], XmNheight, &specPlot.x.windowHeight); ++n;
  XtSetArg(args[n], XmNdepth, &depth); ++n;
  XtGetValues(specPlot.canvas, args, n);

  NewPixmap(&specPlot, specPlot.x.windowWidth, specPlot.x.windowHeight, depth);
 
  specPlot.x.HD = (int)(specPlot.x.windowWidth * 0.7);
  specPlot.x.VD = (int)(specPlot.x.windowHeight * 0.7);
  specPlot.x.LV = (int)(specPlot.x.windowWidth * 0.2);
  specPlot.x.TH = (int)(specPlot.x.windowHeight * 0.15);

  specPlot.x.titleOffset    = 20;
  specPlot.x.subTitleOffset = 40;

  specPlot.x.yLabelOffset   = 10;

  specPlot.x.RV = specPlot.x.LV + specPlot.x.HD;
  specPlot.x.BH = specPlot.x.TH + specPlot.x.VD;

  specPlot.x.xLabelOffset = specPlot.x.BH + 40;

  specPlot.x.ticLength = specPlot.x.HD > 300 ? 10 : 5;
  specPlot.x.yTicLabelOffset = 5;
  specPlot.x.xTicLabelOffset = 15;

  fontOffset = 1;

}	/* END RESIZESPECWINDOW */

/* -------------------------------------------------------------------- */
void PlotSpectrum(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *call)
{
  XFontStruct	*fontInfo;

  /* If there are more Expose Events on the queue, then ignore this
   * one.
   */
  if (specPlot.windowOpen == False ||
      (call && call->event && call->reason == XmCR_EXPOSE && ((XExposeEvent *)call->event)->count > 0))
    return;

  XSetClipMask(specPlot.dpy, specPlot.gc, None);

  ClearPixmap(&specPlot);

  XDrawRectangle(specPlot.dpy, specPlot.win, specPlot.gc,
                specPlot.x.LV, specPlot.x.TH, specPlot.x.HD, specPlot.x.VD);

  plotTitlesX(&specPlot, fontOffset, false);
  plotLabelsX(&specPlot, fontOffset);

  if (psd[0].display == SPECTRA)
    specPlot.Yaxis[0].logScale = True;
  else
    specPlot.Yaxis[0].logScale = False;

  AutoScaleSpectralWindow();

  fontInfo = specPlot.fontInfo[3+fontOffset];
  XSetFont(specPlot.dpy, specPlot.gc, fontInfo->fid);

  yTicsLabelsX(&specPlot, fontInfo, LEFT_SIDE, True);
  xTicsLabelsX(&specPlot, fontInfo, True);

  if (NumberDataFiles > 0 && NumberDataSets >= 1)
    {
    if (equalLogInterval())
      ComputeELIA();

    switch (psd[0].display)
      {
      case SPECTRA:
        plotLogLog(&specPlot);

        PlotVarianceX(&specPlot, fontInfo);
        break;

      case COSPECTRA:
        plotSemiLog(&specPlot, psd[0].Pxx);
        PlotVarianceX(&specPlot, fontInfo);
        break;

      case QUADRATURE:
        plotSemiLog(&specPlot, psd[0].Qxx);
        break;

      case COHERENCE:
      case PHASE:
      case RATIO:
        plotSemiLog(&specPlot, psd[0].Special);
        break;
      }
    }

  XCopyArea(specPlot.dpy, specPlot.win, XtWindow(specPlot.canvas), specPlot.gc,
        0, 0, specPlot.x.windowWidth, specPlot.x.windowHeight, 0, 0);
 
}	/* END PLOTSPECTRUM */

/* -------------------------------------------------------------------- */
static void plotLogLog(PLOT_INFO *plot)
{
  size_t	i, nPts, set, nSets;
  XPoint	*pts;
  double	*plotData, datumY, freq, waveNumber = 0;
  double	xMin, yMin;

  if (plot->Xaxis.logScale)
    {
    xMin = log10(plot->Xaxis.min);
    xScale = (float)plot->x.HD / (log10(plot->Xaxis.max) - xMin);
    }
  else
    {
    xMin = plot->Xaxis.min;
    xScale = (float)plot->x.HD / (plot->Xaxis.max - xMin);
    }

  if (plot->Yaxis[0].logScale)
    {
    yMin = log10(plot->Yaxis[0].min);
    yScale = (float)plot->x.VD / (log10(plot->Yaxis[0].max) - yMin);
    }
  else
    {
    yMin = plot->Yaxis[0].min;
    yScale = (float)plot->x.VD / (plot->Yaxis[0].max - yMin);
    }

  if (plotWaveNumber())
    waveNumber = 2.0 * M_PI / tas.stats.mean;

  nSets = std::min(NumberDataSets, MAX_PSD);

  pts = new XPoint[psd[0].M+1];
  setClippingX(plot);
  XSetLineAttributes(specPlot.dpy, specPlot.gc, LineThickness, LineSolid,CapButt,JoinMiter);

  /* Display data lines.
   */
  if (Color)
    {
    ResetColors();
    XSetForeground(plot->dpy, plot->gc, NextColor());
    }


  for (set = 0; set < nSets; ++set)
    {
    if (equalLogInterval())
      {
      nPts = psd[set].ELIAcnt - 1;
      plotData = psd[set].ELIAy;
      }
    else
      {
      nPts = psd[0].M;
      plotData = psd[set].Pxx;
      }

    /* Calculate points for lines
     */
    for (i = 1; i <= nPts; ++i)
      {
      if (equalLogInterval())
        freq = psd[set].ELIAx[i];
      else
        freq = psd[set].freqPerBin * i;

      if (plotWaveNumber())
        freq *= waveNumber;

      if (plot->Xaxis.logScale)
        freq = log10(freq);

      if (plot->Yaxis[0].logScale)
        datumY = log10(plotData[i]);
      else
        datumY = plotData[i];

      pts[i-1].x = plot->x.LV + (int)(xScale * (freq - xMin) + 0.5);
      pts[i-1].y = plot->x.BH - (int)(yScale * (datumY - yMin) + 0.5);
      }


    /* Drop band limited variance lines.
     */
    PushColor();
    XSetForeground(plot->dpy, plot->gc, GetColor(0));
    for (i = 1; i <= psd[0].M; ++i)
      {
      if ((size_t)rint(startFreq() / psd[set].freqPerBin) == i ||
          (size_t)rint(endFreq() / psd[set].freqPerBin) == i)
        {
        int x, y;

        freq = psd[set].freqPerBin * i;

        if (plotWaveNumber())
          freq *= waveNumber;

        if (plot->Xaxis.logScale)
          freq = log10(freq);

        if (plot->Yaxis[0].logScale)
          datumY = log10(psd[set].Pxx[i]);
        else
          datumY = psd[set].Pxx[i];

        x = plot->x.LV + (int)(xScale * (freq - xMin) + 0.5);
        y = plot->x.BH - (int)(yScale * (datumY - yMin) + 0.5);

        XDrawLine(plot->dpy, plot->win, plot->gc, x, y, x, plot->x.BH);
        }
      }

    PopColor();
    XSetForeground(plot->dpy, plot->gc, CurrentColor());

    XDrawLines(plot->dpy, plot->win, plot->gc, pts, nPts, CoordModeOrigin);
    XSetForeground(plot->dpy, plot->gc, NextColor());
    }

  /* Draw -5/3 slope line.
   */
  XSetForeground(plot->dpy, plot->gc, GetColor(0));
  if (plotFiveThirds())
    doFiveThirdsX(plot);

  XSetClipMask(plot->dpy, plot->gc, None);


  if (plotWaveLength())
    doWaveLengthX(plot);

  delete [] pts;

}	/* END PLOTLOGLOG */

/* -------------------------------------------------------------------- */
static void plotSemiLog(PLOT_INFO *plot, double *dataP)
{
  size_t	i, nPts;
  XPoint	*pts;
  double	*plotData, freq, yScale, waveNumber = 0;
  double	xMin;

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

  pts = new XPoint[psd[0].M+1];

  if (plot->Xaxis.logScale)
  {
    xMin = log10(plot->Xaxis.min);
    xScale = (NR_TYPE)plot->x.HD / (log10(plot->Xaxis.max) - xMin);
  }
  else
  {
    xMin = plot->Xaxis.min;
    xScale = (NR_TYPE)plot->x.HD / (plot->Xaxis.max - xMin);
  }

  yScale = (NR_TYPE)plot->x.VD / (plot->Yaxis[0].max - plot->Yaxis[0].min);

  if (plotWaveNumber())
    waveNumber = 2.0 * M_PI / tas.stats.mean;

  /* Calculate points for lines		*/
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

    pts[i-1].x = plot->x.LV + (int)(xScale * (freq - xMin));
    pts[i-1].y = plot->x.BH - (int)(yScale * (plotData[i] -plot->Yaxis[0].min));
    }


  /* Drop band limited variance lines.
   */
  for (i = 1; i <= psd[0].M; ++i)
    {
    if ((size_t)(startFreq() / psd[0].freqPerBin) == i ||
        (size_t)(endFreq() / psd[0].freqPerBin) == i)
      {
      int x, y;

      freq = psd[0].freqPerBin * i;

      if (plotWaveNumber())
        freq *= waveNumber;

      if (plot->Xaxis.logScale)
        freq = log10(freq);

      x = plot->x.LV + (int)(xScale * (freq - xMin));
      y = plot->x.BH - (int)(yScale * (dataP[i] - plot->Yaxis[0].min));

      XDrawLine(plot->dpy, plot->win, plot->gc, x, y, x, plot->x.BH);
      }
    }


  if (plotWaveLength())
    doWaveLengthX(plot);


  /* Display lines
   */
  if (Color)
    {
    ResetColors();
    XSetForeground(plot->dpy, plot->gc, NextColor());
    }

  setClippingX(plot);
  XSetLineAttributes(specPlot.dpy, specPlot.gc, LineThickness, LineSolid,CapButt,JoinMiter);
  XDrawLines(plot->dpy, plot->win, plot->gc, pts, nPts, CoordModeOrigin);
  XSetClipMask(plot->dpy, plot->gc, None);
  XSetForeground(plot->dpy, plot->gc, GetColor(0));

  delete [] pts;

}	/* END PLOTSEMILOG */

/* -------------------------------------------------------------------- */
void doWaveLengthX(PLOT_INFO *plot)
{
  int		i, x, xDecade, nXdecades;
  double	xMantissa;


  /* Plot Wave Length Scale.
   */
  xMantissa = log10(tas.stats.mean);
  xDecade = (int)xMantissa;
  xMantissa -= xDecade;
  nXdecades = (int)(xDecade - log10(plot->Xaxis.min)) + 1;

  x = plot->x.LV + (int)(xMantissa * xScale);

  for (i = 0; i < plot->Xaxis.nMajorTics; ++i)
    {
    XDrawLine(plot->dpy,plot->win,plot->gc, x, plot->x.TH,
              x, plot->x.TH - plot->x.ticLength);

    MakeLogTicLabel(buffer, nXdecades);
    XDrawString(plot->dpy, plot->win, plot->gc,
             x - 18, plot->x.TH - 12, buffer, strlen(buffer));

    x += (int)xScale;
    --nXdecades;
    }

  XDrawString(plot->dpy, plot->win, plot->gc,
              plot->x.LV + (plot->x.HD >> 1) - 50,
              plot->x.TH - 25,
              "Wave length (M)", 15);;

}	/* END DOWAVELENGTH */

/* -------------------------------------------------------------------- */
void doFiveThirdsX(PLOT_INFO *plot)
{
  int	yscl;

  if (multiplyByFreq())
    yscl = (int)(yScale * 2);
  else
    yscl = (int)(yScale * 5);

  XDrawLine(plot->dpy, plot->win, plot->gc, plot->x.LV,
        plot->x.BH - yscl, plot->x.LV + (int)(xScale * 3), plot->x.BH);

}	/* END DOFIVETHIRDS */

/* END SPECX.C */
