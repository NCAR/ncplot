/*
-------------------------------------------------------------------------
OBJECT NAME:	crosshair.c

FULL NAME:	CrossHair Tracking

ENTRY POINTS:	CanvasMotion()

STATIC FNS:	TrackTimeSeries()
		TrackXY()

DESCRIPTION:	Cross hair tracking.

REFERENCES:	none

REFERENCED BY:	XtMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1999-2001
-------------------------------------------------------------------------
*/

#include "define.h"

static int	saveX = 0, saveY = 0;
static Display	*dpy;
static Window	win;
static GC	gc;
static char	tStr[3][20];

static void	TrackTimeSeries(int, int), TrackXY(int, int);


/* -------------------------------------------------------------------- */
void CanvasMotion(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
  XMotionEvent  *xm = (XMotionEvent *)evt;

  dpy	= mainPlot[0].dpy;
  win	= XtWindow(mainPlot[0].canvas);
  gc	= mainPlot[0].gc;

/* The color that works ok on Solaris doesn't work so good under Linux.
 * This should eventually be put in some defaults file along with colors.
 * Similar code is in zoom.c.
 */
#ifdef SVR4
  XSetForeground(dpy, gc, GetColor(2));
#else
  XSetForeground(dpy, gc, GetColor(3));
#endif
  XSetFont(dpy, gc, mainPlot[0].fontInfo[4]->fid);
  XSetFunction(dpy, gc, GXxor);

  switch (PlotType)
    {
    case TIME_SERIES:
      TrackTimeSeries(xm->x, xm->y);
      break;

    case XY_PLOT:
      TrackXY(xm->x, xm->y);
      break;
    }

  XSetFunction(dpy, gc, GXcopy);

}	/* END CANVASMOTION */

/* -------------------------------------------------------------------- */
static void TrackTimeSeries(int x, int y)
{
  size_t	i;
  PLOT_INFO	*plot = NULL;
  float		yDiff;
  struct axisInfo	*axis;


  if (saveX)	/* Erase previous lines.	*/
    {
    XDrawLine(dpy, win, gc, mainPlot[0].x.LV, saveY, mainPlot[0].x.RV, saveY);

    for (i = 0; i < NumberOfPanels; ++i)
      XDrawLine(dpy, win, gc, saveX, mainPlot[i].x.TH, saveX, mainPlot[i].x.BH);

    XDrawString(dpy, win, gc, saveX+10, saveY+30, tStr[0], strlen(tStr[0]));
    XDrawString(dpy, win, gc, saveX-50, saveY-10, tStr[1], strlen(tStr[1]));
    }


  /* Locate which panel the cursor is in.
   */
  if (x >= mainPlot[0].x.LV && x <= mainPlot[0].x.RV)
    for (i = 0; i < NumberOfPanels; ++i)
      if (y >= mainPlot[i].x.TH && y <= mainPlot[i].x.BH)
        plot = &mainPlot[i];


  /* Bail out if we're outside all panels.
   */
  if (!plot)
    {
    saveX = saveY = 0;
    tStr[0][0] = tStr[1][0] = tStr[2][0] = '\0';

    PointerCursor(mainPlot[0].canvas);
    return;
    }

  CrossHairCursor(mainPlot[0].canvas);

  /* Calculate engineering units.
   */
  MakeTimeTicLabel(tStr[0], x - mainPlot[0].x.LV, mainPlot[0].x.HD);
  XDrawString(dpy, win, gc, x+10, y+30, tStr[0], strlen(tStr[0]));

  axis = &plot->Yaxis[whichSide()];
  if (axis->logScale)
    {
    float yScale = plot->x.VD /
	(log10(axis->max) - log10(axis->min));

    if (axis->invertAxis)
      yDiff = ((y - plot->x.TH) / yScale) + log10(axis->min);
    else
      yDiff = -(((y - plot->x.BH) / yScale) - log10(axis->min));

    sprintf(tStr[1], "%g", pow(10.0, yDiff));
    }
  else
    {
    yDiff = axis->max - axis->min;

    if (axis->invertAxis)
      yDiff *= (float)(y - plot->x.TH) / plot->x.VD;
    else
      yDiff *= (float)(plot->x.BH - y) / plot->x.VD;

    sprintf(tStr[1], "%g", axis->min + yDiff);
    }

  XDrawString(dpy, win, gc, x-50, y-10, tStr[1], strlen(tStr[1]));


  /* Draw new lines and text.
   */
  XDrawLine(dpy, win, gc, plot->x.LV, y, plot->x.RV, y);

  for (i = 0; i < NumberOfPanels; ++i)
    XDrawLine(dpy, win, gc, x, mainPlot[i].x.TH, x, mainPlot[i].x.BH);

  saveX = x;
  saveY = y;

}	/* END TRACKTIMESERIES */

/* -------------------------------------------------------------------- */
static void TrackXY(int x, int y)
{
  size_t	i;
  PLOT_INFO	*plot = NULL;
  float		xDiff, yDiff;
  struct axisInfo	*axis;

  if (saveX)	/* Erase previous lines.	*/
    {
    XDrawLine(dpy, win, gc, saveX, xyyPlot[0].x.TH, saveX, xyyPlot[0].x.BH);

    for (i = 0; i < NumberOfXYpanels; ++i)
      XDrawLine(dpy, win, gc, xyyPlot[i].x.LV, saveY, xyyPlot[i].x.RV, saveY);

    XDrawString(dpy, win, gc, saveX+10, saveY+30, tStr[0], strlen(tStr[0]));
    XDrawString(dpy, win, gc, saveX-50, saveY-10, tStr[1], strlen(tStr[1]));
/*    XDrawString(dpy, win, gc, saveX+10, saveY-10, tStr[2], strlen(tStr[2]));*/
    }


  /* Locate which panel the cursor is in.
   */
  if (y >= xyyPlot[0].x.TH && y <= xyyPlot[0].x.BH)
    for (i = 0; i < NumberOfXYpanels; ++i)
      if (x >= xyyPlot[i].x.LV && x <= xyyPlot[i].x.RV)
        plot = &xyyPlot[i];


  /* Bail out if we're outside all panels.
   */
  if (!plot)
    {
    saveX = saveY = 0;
    tStr[0][0] = tStr[1][0] = tStr[2][0] = '\0';

    PointerCursor(xyyPlot[0].canvas);
    return;
    }

  CrossHairCursor(xyyPlot[0].canvas);

  /* Calculate engineering units.
   */
  axis = &plot->Xaxis;
  if (axis->logScale)
    {
    float xScale = plot->x.HD / (log10(axis->max) - log10(axis->min));
    
    if (axis->invertAxis)
      xDiff = -(((x - plot->x.RV) / xScale) - log10(axis->min));
    else
      xDiff = ((x - plot->x.LV) / xScale) + log10(axis->min);

    sprintf(tStr[0], "%g", pow(10.0, xDiff));
    }
  else
    {
    xDiff = axis->max - axis->min;

    if (axis->invertAxis)
      xDiff *= (float)(x + plot->x.RV) / plot->x.HD;
    else
      xDiff *= (float)(x - plot->x.LV) / plot->x.HD;

    sprintf(tStr[0], "%g", axis->min + xDiff);
    }

  XDrawString(dpy, win, gc, x+10, y+30, tStr[0], strlen(tStr[0]));


  axis = &plot->Yaxis[whichSide()];
  if (axis->logScale)
    {
    float yScale = plot->x.VD / (log10(axis->max) - log10(axis->min));
    
    if (axis->invertAxis)
      yDiff = ((y - plot->x.TH) / yScale) + log10(axis->min);
    else
      yDiff = -(((y - plot->x.BH) / yScale) - log10(axis->min));

    sprintf(tStr[1], "%g", pow(10.0, yDiff));
    }
  else
    {
    yDiff = axis->max - axis->min;

    if (axis->invertAxis)
      yDiff *= (float)(y - plot->x.TH) / plot->x.VD;
    else
      yDiff *= (float)(plot->x.BH - y) / plot->x.VD;

    sprintf(tStr[1], "%g", axis->min + yDiff);
    }

  XDrawString(dpy, win, gc, x-50, y-10, tStr[1], strlen(tStr[1]));

  /* Draw new lines and text.
   */
  XDrawLine(dpy, win, gc, x, plot->x.TH, x, plot->x.BH);

  for (i = 0; i < NumberOfXYpanels; ++i)
    XDrawLine(dpy, win, gc, xyyPlot[i].x.LV, y, xyyPlot[i].x.RV, y);

  saveX = x;
  saveY = y;

}	/* END TRACKXY */

/* END CROSSHAIR.C */
