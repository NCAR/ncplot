/*
-------------------------------------------------------------------------
OBJECT NAME:	diffX.c

FULL NAME:	Plot Graph in X window

ENTRY POINTS:	PlotDifference()

STATIC FNS:	plotLines()

DESCRIPTION:	This is the Expose event procedure to regenerate the
		whole image.

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

static int	fontOffset;

static void	plotDiffLines(PLOT_INFO *plot);


/* -------------------------------------------------------------------- */
void ResizeDiffWindow(Widget w, XtPointer client, XtPointer call)
{
  int	n, depth;
  Arg	args[5];

  n = 0;
  XtSetArg(args[n], XmNwidth, &diffPlot.x.windowWidth); ++n;
  XtSetArg(args[n], XmNheight, &diffPlot.x.windowHeight); ++n;
  XtSetArg(args[n], XmNdepth, &depth); ++n;
  XtGetValues(diffPlot.canvas, args, n);

  NewPixmap(&diffPlot, diffPlot.x.windowWidth, diffPlot.x.windowHeight, depth);

  diffPlot.x.HD = (int)(diffPlot.x.windowWidth * 0.83);
  diffPlot.x.VD = (int)(diffPlot.x.windowHeight * 0.691);
  diffPlot.x.LV = (int)(diffPlot.x.windowWidth * 0.104);
  diffPlot.x.TH = (int)(diffPlot.x.windowHeight * 0.09);

  diffPlot.x.RV = diffPlot.x.LV + diffPlot.x.HD;
  diffPlot.x.BH = diffPlot.x.TH + diffPlot.x.VD;

  diffPlot.x.ticLength		= diffPlot.x.HD > 400 ? 10 : 5;
  diffPlot.x.yLabelOffset	= 10;
  diffPlot.x.yTicLabelOffset	= 5;
  diffPlot.x.xTicLabelOffset	= 20;
  diffPlot.x.xLegendText	= 55;

  fontOffset = (diffPlot.x.windowWidth < 400) ? 1 : 0;


}	/* END RESIZEDIFFWINDOW */

/* -------------------------------------------------------------------- */
void PlotDifference(Widget w, XtPointer client,
			XmDrawingAreaCallbackStruct *call)
{
  XFontStruct	*fontInfo;

  /* If there are more Expose Events on the queue, then ignore this
   * one.
   */
  if (diffPlot.windowOpen == false ||
     (call && call->event && call->reason == XmCR_EXPOSE && ((XExposeEvent *)call->event)->count > 0))
    return;

  if (DataChanged)
    ComputeDiff();

  XSetClipMask(diffPlot.dpy, diffPlot.gc, None);

  ClearPixmap(&diffPlot);

  XDrawRectangle( diffPlot.dpy, diffPlot.win, diffPlot.gc,
                  diffPlot.x.LV, diffPlot.x.TH, diffPlot.x.HD, diffPlot.x.VD);

  if (dataFile[dataSet[0].fileIndex].ShowPrelimDataWarning ||
      dataFile[dataSet[1].fileIndex].ShowPrelimDataWarning)
    plotWarningX(&diffPlot, fontOffset);

  plotLabelsX(&diffPlot, fontOffset);

  fontInfo = diffPlot.fontInfo[3+fontOffset];
  XSetFont(diffPlot.dpy, diffPlot.gc, fontInfo->fid);

  if (NumberDataFiles > 0 && NumberDataSets >= 2)
    {
    yTicsLabelsX(&diffPlot, fontInfo, LEFT_SIDE, true);
    xTicsLabelsX(&diffPlot, fontInfo, true);
    plotDiffLines(&diffPlot);
    }

  XCopyArea(diffPlot.dpy, diffPlot.win, XtWindow(diffPlot.canvas), diffPlot.gc,
        0, 0, diffPlot.x.windowWidth, diffPlot.x.windowHeight, 0, 0);

}	/* END PLOTDIFFERENCE */

/* -------------------------------------------------------------------- */
static void plotDiffLines(PLOT_INFO *plot)
{
  if (Color)
    {
    ResetColors();
    XSetForeground(plot->dpy, plot->gc, NextColor());
    }

  XSetLineAttributes(plot->dpy, plot->gc, LineThickness, LineSolid, CapButt, JoinMiter);

  plotTimeSeries(plot, &diffSet);

  if (Color)
    XSetForeground(plot->dpy, plot->gc, GetColor(0));

  sprintf(buffer, "%10.2f%10.2f%10.2f%10.2f%10.2f",
          diffSet.stats.mean, diffSet.stats.sigma, diffSet.stats.variance,
          diffSet.stats.min, diffSet.stats.max);

  XDrawString(plot->dpy, plot->win, plot->gc, plot->x.xLegendText+100,
              yLegendX(plot, 0)+3, buffer, strlen(buffer));

  strcpy(buffer, "      mean     sigma       var       min       max");
  XDrawString(plot->dpy, plot->win, plot->gc, plot->x.xLegendText+100,
              yLegendX(plot, 2)+12, buffer, strlen(buffer));

}	/* END PLOTDIFFLINES */

/* END DIFFX.C */
