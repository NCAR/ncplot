/*
-------------------------------------------------------------------------
OBJECT NAME:	regret1.c

FULL NAME:	Linear Detrend Regression.

ENTRY POINTS:	LinearLeastSquare()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	spctrm.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"


/* -------------------------------------------------------------------- */
void LinearLeastSquare(DATASET_INFO *set, float inp[], float out[])
{
  size_t	i;
  double	Ymean, Xmean, SXX, SXY, B0, B1, im;


  /* Compute x bar and y bar.
   */
  Xmean = (double)set->nPoints / 2.0;
  Ymean = set->stats.mean;

  for (i = 0, SXX = SXY = 0.0; i < set->nPoints; ++i)
    {
    im = (i - Xmean);
    SXX += im * im;
    SXY += im * (inp[i] - Ymean);
    }

  B1 = SXY / SXX;
  B0 = Ymean - B1 * Xmean;


  /* y[i] = B0 + B1 * x[i]
   */
  for (i = 0; i < set->nPoints; ++i)
    out[i] = B0 + B1 * i;
/*
fprintf(stderr,"Ybar=%f, Xbar=%f, Slope=%f, Yinter=%f\n", Ymean, Xmean, B1, B0);
*/
}	/* END LINEARLEASTSQUARE */

/* END REGRET1.C */
