/*
-------------------------------------------------------------------------
OBJECT NAME:	equal.c

FULL NAME:	Equal Scaling

ENTRY POINTS:	ToggleEqualScaling()
		equalScaling()

STATIC FNS:	none

DESCRIPTION:	On Track plots, one would like lat and lon to cover the
		same distance on the plot.

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 2000
-------------------------------------------------------------------------
*/

#include "define.h"

static int	equalScales = 0;


/* -------------------------------------------------------------------- */
void ToggleEqualScaling(Widget w, XtPointer client, XtPointer call)
{
  equalScales = !equalScales;

  DrawMainWindow();

}       /* END TOGGLEEQUALSCALING */

/* -------------------------------------------------------------------- */
bool equalScaling()
{
  return(equalScales);

}  /* END EQUALSCALING */

/* END EQUAL.C */
