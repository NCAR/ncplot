/*
-------------------------------------------------------------------------
OBJECT NAME:	variance.c

FULL NAME:	Spectral Variance routines.

ENTRY POINTS:	ComputeBandLimitedVariance()
		PlotVarianceX()
		PlotVariancePS()

STATIC FNS:	makeTotalVarLabel()
		makeBandLimitedVarLabel()

DESCRIPTION:	

REFERENCES:	spctrm.c

REFERENCED BY:	Callback

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"
#include "ps.h"

static void	makeTotalVarLabel(),
		makeBandLimitedVarLabel(size_t, size_t, double, double);


/* -------------------------------------------------------------------- */
void ComputeBandLimitedVariance(Widget w, XtPointer client, XtPointer call)
{
  size_t i, startBin, endBin;

  psd[0].bandVariance = 0.0;

  if ((startBin = (size_t)(startFreq() / psd[0].freqPerBin)) <= 0)
    startBin = 1;

  if ((endBin = (size_t)(endFreq() / psd[0].freqPerBin)) > psd[0].M)
    endBin = psd[0].M;

  for (i = startBin; i <= endBin; ++i)
    psd[0].bandVariance += psd[0].Pxx[i];

}	/* END COMPUTEBANDLIMITEDVARIANCE */

/* -------------------------------------------------------------------- */
void PlotVarianceX(PLOT_INFO *plot, XFontStruct *fontInfo)
{
  size_t startBin, endBin;

  XSetFont(plot->dpy, plot->gc, fontInfo->fid);

  if ((startBin = (size_t)(startFreq() / psd[0].freqPerBin)) <= 0)
    startBin = 1;

  if ((endBin = (size_t)(endFreq() / psd[0].freqPerBin)) > psd[0].M)
    endBin = psd[0].M;


  makeTotalVarLabel();
  XDrawString(plot->dpy, plot->win, plot->gc, plot->x.LV,
                plot->x.windowHeight - 25, buffer, strlen(buffer));

  makeBandLimitedVarLabel(startBin, endBin,
                          psd[0].freqPerBin * startBin, psd[0].freqPerBin * endBin);
  XDrawString(plot->dpy, plot->win, plot->gc, plot->x.LV,
                plot->x.windowHeight - 15, buffer, strlen(buffer));

  sprintf(buffer, "K = %d, M = %d, nPoints = %ld\n",
                (int)psd[0].K, (int)psd[0].M, (long)dataSet[0].nPoints);
  XDrawString(plot->dpy, plot->win, plot->gc, plot->x.LV,
                plot->x.windowHeight - 5, buffer, strlen(buffer));

}       /* END PLOTVARIANCEX */

/* -------------------------------------------------------------------- */
void PlotVariancePS(PLOT_INFO *plot, FILE *fp)
{
  size_t startBin, endBin;

  if ((startBin = (size_t)(startFreq() / psd[0].freqPerBin)) <= 0)
    startBin = 1;

  if ((endBin = (size_t)(endFreq() / psd[0].freqPerBin)) > psd[0].M)
    endBin = psd[0].M;


  makeTotalVarLabel();
  fprintf(fp, moveto, 0, plot->ps.xLabelOffset - 75);
  fprintf(fp, show, buffer);

  makeBandLimitedVarLabel(startBin, endBin,
	psd[0].freqPerBin * startBin, psd[0].freqPerBin * endBin);
  fprintf(fp, moveto, 0, plot->ps.xLabelOffset - 120);
  fprintf(fp, show, buffer);

  sprintf(buffer, "K = %d, M = %d, nPoints = %ld\n",
          (int)psd[0].K, (int)psd[0].M, (long)dataSet[0].nPoints);
  fprintf(fp, moveto, 0, plot->ps.xLabelOffset - 165);
  fprintf(fp, show, buffer);

}	/* END PLOTVARIANCEPS */

/* -------------------------------------------------------------------- */
static void makeTotalVarLabel()
{
  sprintf(buffer, "Total %svariance = %g",
		psd[0].display == SPECTRA ? "" : "co-", psd[0].totalVariance);
}

/* -------------------------------------------------------------------- */
static void makeBandLimitedVarLabel(size_t startBin, size_t endBin, double sf, double ef)
{
  if (startBin == 1 && endBin == psd[0].M)
    sprintf(buffer, "%sVariance (w/o DC component) = %g",
      psd[0].display == SPECTRA ? "" : "Co-", psd[0].bandVariance);
  else
    sprintf(buffer, "Band limited %svariance = %g (%gHz - %gHz)",
      psd[0].display == SPECTRA ? "" : "co-", psd[0].bandVariance, sf, ef);
}

/* END VARIANCE.C */
