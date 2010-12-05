/*
-------------------------------------------------------------------------
OBJECT NAME:	zoom.c

FULL NAME:	Drag Zoom & Undo.

ENTRY POINTS:	Zoom()
		UnZoom()

STATIC FNS:	DoTheBox()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <stack>

struct zoom_info
{
  int	saveStartTime[4], saveEndTime[4];
  float	xMin, xMax, yMin[2], yMax[2];

};

// Corner points from drawing the box.
static int startX = 0, startY = 0;
static int endX = 0, endY = 0;

static std::stack<struct zoom_info> zoom_stack;
struct zoom_info this_zoom;


/* -------------------------------------------------------------------- */
static void DoTheBox(Widget w, XtPointer client, XMotionEvent *evt, Boolean cont2disp)
{
  if (evt == NULL)
    return;

/* The color that works ok on Solaris doesn't work so good under Linux.
 * This should eventually be put in some defaults file along with colors.
 * Similar code is in crosshair.c.
 */
#ifdef SVR4
  XSetForeground(mainPlot[0].dpy, mainPlot[0].gc, GetColor(2));
#else
  XSetForeground(mainPlot[0].dpy, mainPlot[0].gc, GetColor(3));
#endif

  XSetLineAttributes(mainPlot[0].dpy, mainPlot[0].gc, 1,
                     LineSolid, CapButt, JoinMiter);

  if (endX != 0 && endY != 0)
    XDrawRectangle(mainPlot[0].dpy, XtWindow(mainPlot[0].canvas),mainPlot[0].gc,
                   startX, startY, endX - startX, endY - startY);

  endX = evt->x;
  endY = evt->y;

  XDrawRectangle(mainPlot[0].dpy, XtWindow(mainPlot[0].canvas), mainPlot[0].gc,
                 startX, startY, endX - startX, endY - startY);

}	/* END DOTHEBOX */

/* -------------------------------------------------------------------- */
void Zoom(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
  XButtonEvent	*xb = (XButtonEvent *)evt->event;
  static bool	cancel = false;

  if (xb->button != Button1)
    return;

  if ((xb->state & Button1MotionMask) == 0)
    startX = startY = endX = endY = 0;

  if ((xb->state & Button1MotionMask) == 0x100)
    {
    XSetFunction(mainPlot[0].dpy, mainPlot[0].gc, GXcopy);
    XtRemoveEventHandler(mainPlot[0].canvas, Button1MotionMask, false,
                         (XtEventHandler)DoTheBox, NULL);
    }

  // Keep it bounds.
  if (xb->x < mainPlot[0].x.LV) xb->x = mainPlot[0].x.LV;
  if (xb->x > mainPlot[0].x.RV) xb->x = mainPlot[0].x.RV;
  if (xb->y < mainPlot[0].x.TH) xb->y = mainPlot[0].x.TH;
  if (xb->y > mainPlot[0].x.BH) xb->y = mainPlot[0].x.BH;

  if ((xb->state & Button1MotionMask) != 0)
    if (cancel || endX < startX || endY < startY)
      return;


  if ((xb->state & Button1MotionMask) == 0)
    {
    cancel = false;

    startX = xb->x;
    startY = xb->y;

    XSetFunction(mainPlot[0].dpy, mainPlot[0].gc, GXxor);
    XtAddEventHandler(mainPlot[0].canvas, Button1MotionMask, false,
                      (XtEventHandler)DoTheBox, NULL);
    }
  else if ((xb->state & Button1MotionMask) == 0x100)
    {
    if (abs(startX - xb->x) < 10 || abs(startY - xb->y) < 10)
      return;

    endX = xb->x;
    endY = xb->y;


    /* Save off current values for unzoom.
     */
    memcpy((void *)this_zoom.saveStartTime, (void *)UserStartTime, 4*sizeof(int));
    memcpy((void *)this_zoom.saveEndTime, (void *)UserEndTime, 4*sizeof(int));

    if (PlotType == TIME_SERIES)
      {
      int	stpe, etpe;
      float	pixPerSec;

      this_zoom.xMin = mainPlot[0].Xaxis.min;
      this_zoom.xMax = mainPlot[0].Xaxis.max;
      this_zoom.yMin[0] = mainPlot[0].Yaxis[0].min;
      this_zoom.yMax[0] = mainPlot[0].Yaxis[0].max;
      this_zoom.yMin[1] = mainPlot[0].Yaxis[1].min;
      this_zoom.yMax[1] = mainPlot[0].Yaxis[1].max;

      pixPerSec = (float)mainPlot[0].x.HD / NumberSeconds;
      stpe = (int)((startX - mainPlot[0].x.LV) / pixPerSec);
      etpe = (int)((mainPlot[0].x.RV - endX) / pixPerSec);

      if (NumberSeconds > 900)	/* 15 minutes.	*/
        {
        stpe /= 60; stpe *= 60;
        etpe += 60; etpe /= 60; etpe *= 60;
        }

      ReduceData(stpe, NumberSeconds - stpe - etpe);

      if (NumberOfPanels == 1)
        {
        mainPlot[0].Yaxis[0].min += ((this_zoom.yMax[0] - this_zoom.yMin[0]) *
                ((float)(mainPlot[0].x.BH - endY) / mainPlot[0].x.VD));

        mainPlot[0].Yaxis[0].max -= ((this_zoom.yMax[0] - this_zoom.yMin[0]) *
                ((float)(startY - mainPlot[0].x.TH) / mainPlot[0].x.VD));

        mainPlot[0].Yaxis[1].min += ((this_zoom.yMax[1] - this_zoom.yMin[1]) *
                ((float)(mainPlot[0].x.BH - endY) / mainPlot[0].x.VD));

        mainPlot[0].Yaxis[1].max -= ((this_zoom.yMax[1] - this_zoom.yMin[1]) *
                ((float)(startY - mainPlot[0].x.TH) / mainPlot[0].x.VD));
        }
      }
    else
      {
      this_zoom.xMin = xyyPlot[0].Xaxis.min;
      this_zoom.xMax = xyyPlot[0].Xaxis.max;
      this_zoom.yMin[0] = xyyPlot[0].Yaxis[0].min;
      this_zoom.yMax[0] = xyyPlot[0].Yaxis[0].max;
      this_zoom.yMin[1] = xyyPlot[0].Yaxis[1].min;
      this_zoom.yMax[1] = xyyPlot[0].Yaxis[1].max;

      xyyPlot[0].Xaxis.min += ((this_zoom.xMax - this_zoom.xMin) *
          ((float)(startX - xyyPlot[0].x.LV) / xyyPlot[0].x.HD));

      xyyPlot[0].Xaxis.max -= ((this_zoom.xMax - this_zoom.xMin) *
          ((float)(xyyPlot[0].x.RV - endX) / xyyPlot[0].x.HD));

      xyyPlot[0].Yaxis[0].min += ((this_zoom.yMax[0] - this_zoom.yMin[0]) *
          ((float)(xyyPlot[0].x.BH - endY) / xyyPlot[0].x.VD));

      xyyPlot[0].Yaxis[0].max -= ((this_zoom.yMax[0] - this_zoom.yMin[0]) *
              ((float)(startY - xyyPlot[0].x.TH) / xyyPlot[0].x.VD));

      xyyPlot[0].Yaxis[1].min += ((this_zoom.yMax[1] - this_zoom.yMin[1]) *
          ((float)(xyyPlot[0].x.BH - endY) / xyyPlot[0].x.VD));

      xyyPlot[0].Yaxis[1].max -= ((this_zoom.yMax[1] - this_zoom.yMin[1]) *
              ((float)(startY - xyyPlot[0].x.TH) / xyyPlot[0].x.VD));
      }

    zoom_stack.push(this_zoom);
    DrawMainWindow();
    }
  else
    printf("Zoom: unexpected state encountered = 0x%x\n", xb->state);

}	/* END ZOOM */

/* -------------------------------------------------------------------- */
void UnZoom(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
  if (zoom_stack.empty())
    return;

  struct zoom_info top = zoom_stack.top();

  mainPlot[0].Xaxis.min = top.xMin;
  mainPlot[0].Xaxis.max = top.xMax;
  mainPlot[0].Yaxis[0].min = top.yMin[0];
  mainPlot[0].Yaxis[0].max = top.yMax[0];
  mainPlot[0].Yaxis[1].min = top.yMin[1];
  mainPlot[0].Yaxis[1].max = top.yMax[1];

  if (memcmp((void *)top.saveStartTime, (void *)UserStartTime, 4*sizeof(int)) ||
    memcmp((void *)top.saveStartTime, (void *)UserStartTime, 4*sizeof(int)))
    {
    memcpy((void *)UserStartTime, (void *)top.saveStartTime, 4*sizeof(int));
    memcpy((void *)UserEndTime, (void *)top.saveEndTime, 4*sizeof(int));

    ReadData();
    }

  zoom_stack.pop();
  DrawMainWindow();

}	/* END UNZOOM */

/* END ZOOM.C */
