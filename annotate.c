/*
-------------------------------------------------------------------------
OBJECT NAME:	annotate.c

FULL NAME:	Annotation Text

ENTRY POINTS:	ClearAnnotations()
		SetCursorXY()
		ProcessText()
		UpdateAnnotationsX()
		UpdateAnnotationsPS()

STATIC FNS:		

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

static struct _annotations
	{
	int	whichPlot;	/* TIME_SERIES | XY_PLOT, etc	*/
	float	x, y;		/* Position, ratio of pixel&windowWidth	*/
	char	text[48];
	} anot[8];

static int	currentAnot = 0;
static short	plotWidth, plotHeight;
static bool	pointSet = false;

/* -------------------------------------------------------------------- */
void ClearAnnotations()
{
  if (currentAnot > 0)
    printf("Annotations have been cleared.\n");

  currentAnot = 0;

}	/* END CLEARANNOTATIONS */

/* -------------------------------------------------------------------- */
void SetCursorXY(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
  XKeyEvent	*xk = (XKeyEvent *)evt->event;

  if (PlotType == XY_PLOT)
    {
    plotWidth = xyyPlot[0].x.windowWidth;
    plotHeight = xyyPlot[0].x.windowHeight;
    }
  else
  if (PlotType == XYZ_PLOT)
    {
    plotWidth = xyzPlot.x.windowWidth;
    plotHeight = xyzPlot.x.windowHeight;
    }
  else
    {
    plotWidth = mainPlot[0].x.windowWidth;
    plotHeight = mainPlot[0].x.windowHeight;
    }

  anot[currentAnot].whichPlot = PlotType;	/* Currently unused	*/
  anot[currentAnot].text[0] = '\0';
  anot[currentAnot].x = (float)xk->x / plotWidth;
  anot[currentAnot].y = (float)xk->y / plotHeight;

  pointSet = true;

}	/* END SETCURSORXY */

/* -------------------------------------------------------------------- */
void ProcessText(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
  XKeyEvent	*xk = (XKeyEvent *)evt->event;

  if (pointSet == false)
    return;

  buffer[XLookupString(xk, buffer, 80, NULL, NULL)] = '\0';

  if (buffer[0] == 8)		/* Backspace key	*/
    {
    anot[currentAnot].text[strlen(anot[currentAnot].text)-1] = '\0';
    }
  else
  if (buffer[0] == '\r')	/* Carriage return	*/
    {
    if (strlen(anot[currentAnot].text))
      ++currentAnot;

    pointSet = false;
    PointerCursor(mainPlot[0].canvas);
    DrawMainWindow();
    }
  else
    strcat(anot[currentAnot].text, buffer);

  XDrawString(mainPlot[0].dpy, XtWindow(mainPlot[0].canvas), mainPlot[0].gc,
       (int)(anot[currentAnot].x * plotWidth), (int)(anot[currentAnot].y * plotHeight),
       anot[currentAnot].text, strlen(anot[currentAnot].text));

}	/* END PROCESSTEXT */

/* -------------------------------------------------------------------- */
void UpdateAnnotationsX(PLOT_INFO *plot)
{
  int	i;

  for (i = 0; i < currentAnot; ++i)
    if (PlotType == anot[i].whichPlot)
      {
      XDrawString(plot->dpy, plot->win, plot->gc,
                 (int)(anot[i].x * plot->x.windowWidth),
                 (int)(anot[i].y * plot->x.windowHeight),
                 anot[i].text, strlen(anot[i].text));
      }

}	/* UPDATEANNOTATIONS */

/* -------------------------------------------------------------------- */
void UpdateAnnotationsPS(PLOT_INFO *plot, FILE *fp)
{
  int	i;

  for (i = 0; i < currentAnot; ++i)
    if (PlotType == anot[i].whichPlot)
      {
      fprintf(fp, moveto, (int)(anot[i].x * plot->ps.windowWidth),
            plot->ps.windowHeight - (int)(anot[i].y * plot->ps.windowHeight));

      fprintf(fp, show, anot[i].text);
      }

}	/* END UPDATEANNOTATIONSPS */

/* END ANNOTATE.C */
