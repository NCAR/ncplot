/*
-------------------------------------------------------------------------
OBJECT NAME:	arrows.c

FULL NAME:	Direction Arrows.

ENTRY POINTS:	PlotDirectionArrows()
		ToggleArrows()

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
void ToggleArrows(Widget w, XtPointer client, XtPointer call)
{
  char	*p = XmTextFieldGetString(w);
 
  nDirectionArrows = atoi(p);
  XtFree(p);

  DrawMainWindow();
 
}	/* END TOGGLEARROWS */

/* -------------------------------------------------------------------- */
void PlotDirectionArrow(PLOT_INFO *plot, int x, int y, int x2, int y2, FILE *fp)
{
  float theta;
  int	x3, y3, x4, y4;
 
  if (fp)	/* PostScript */
    {
    fprintf(fp, "gsave\n");

    if (printerSetup.color)
      fprintf(fp, "stroke 0 0 0 setrgbcolor\n");
    else
      fprintf(fp, "stroke [] 0 setdash\n");

    x2 = x2 - x;
    y2 = y2 - y;
 
    theta = atan2(y2, x2);
 
    x3 = 12; y3 = 12;
    x4 = (int)(x3 * cos(theta) - y3 * sin(theta));
    y4 = (int)(x3 * sin(theta) + y3 * cos(theta));

    fprintf(fp, moveto, x+x4, y+y4);
    fprintf(fp, lineto, x, y);
 
    y3 = -12;
    x4 = (int)(x3 * cos(theta) - y3 * sin(theta));
    y4 = (int)(x3 * sin(theta) + y3 * cos(theta));

    fprintf(fp, lineto, x+x4, y+y4);
    fprintf(fp, moveto, x, y);

    fprintf(fp, "stroke grestore\n");
    }
  else
    {
    XSetForeground(plot->dpy, plot->gc, GetColor(0));
 
    x2 = x2 - x;
    y2 = y - y2;
 
    theta = atan2(y2, x2);
 
    x3 = 6; y3 = 6;
    x4 = (int)(x3 * cos(theta) - y3 * sin(theta));
    y4 = (int)(x3 * sin(theta) + y3 * cos(theta));
    XDrawLine(plot->dpy, plot->win, plot->gc, x, y, x+x4, y-y4);
 
    y3 = -6;
    x4 = (int)(x3 * cos(theta) - y3 * sin(theta));
    y4 = (int)(x3 * sin(theta) + y3 * cos(theta));
    XDrawLine(plot->dpy, plot->win, plot->gc, x, y, x+x4, y-y4);
 
    XSetForeground(plot->dpy, plot->gc, CurrentColor());
    }

}	/* END PLOTDIRECTIONARROWS */

/* END ARROWS.C */
