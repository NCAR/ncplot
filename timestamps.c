/*
-------------------------------------------------------------------------
OBJECT NAME:	timestamps.c

FULL NAME:	Time Stamps for XY & XYZ plots

ENTRY POINTS:	PlotTimeStamps()
		ToggleTimeStamps()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <Xm/TextF.h>

/* -------------------------------------------------------------------- */
void ToggleTimeStamps(Widget w, XtPointer client, XtPointer call)
{
  char  *p = XmTextFieldGetString(w);
 
  nTimeStamps = atoi(p);
  XtFree(p);

  if (nTimeStamps > 0)
    --nTimeStamps;

  DrawMainWindow();
 
}	/* END TOGGLETIMESTAMPS */

/* -------------------------------------------------------------------- */
void PlotTimeStamps(PLOT_INFO *plot, int x, int y, int cnt, FILE *fp)
{
  if (nTimeStamps <= 0)
    return;

  MakeTimeTicLabel(buffer, cnt, nTimeStamps);

  if (fp)	/* PostScript */
    {
    fprintf(fp, "gsave\n");

    if (printerSetup.color)
      fprintf(fp, "stroke 0 0 0 setrgbcolor\n");
    else
      fprintf(fp, "stroke [] 0 setdash\n");

    fprintf(fp, moveto, x, y);
    fprintf(fp, lineto, x+20, y);
    fprintf(fp, moveto, x+25, y-10);
    fprintf(fp, show, buffer);

    fprintf(fp, "stroke grestore\n");
    }
  else
    {
    XSetForeground(plot->dpy, plot->gc, GetColor(0));
 
    XDrawLine(plot->dpy, plot->win, plot->gc, x, y, x+8, y);

    XDrawString(plot->dpy, plot->win, plot->gc, x+10, y+4,
                buffer, strlen(buffer));
 
    XSetForeground(plot->dpy, plot->gc, CurrentColor());
    }

}	/* END PLOTTIMESTAMPS */

/* END TIMESTAMPS.C */
