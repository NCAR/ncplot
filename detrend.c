/*
-------------------------------------------------------------------------
OBJECT NAME:	detrend.c

FULL NAME:	Detrending Functions

ENTRY POINTS:	DetrendNone()
		DetrendMean()
		DetrendLinear()

STATIC FNS:	none

DESCRIPTION:	Perform detrending function for spectral routines.  All
		MISSING_VALUES are converted to zero.

REFERENCES:	regret.c (Linear only).

REFERENCED BY:	spec.c, cospec.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"


/* -------------------------------------------------------------------- */
void CleanAndCopyData(DATASET_INFO *set, float out[])
{
  int	i;
  float	datum;

  for (i = 0; i < set->nPoints; ++i)
    {
    datum = set->data[(set->head + i) % set->nPoints];

    if (datum != set->missingValue)
      out[i] = datum;
    else
      {		/* Linear interp missing values. */
      int	e;
      float	d1;

      for (e = i; e < set->nPoints; ++e)
        if ((d1 = set->data[(set->head + e) % set->nPoints]) != set->missingValue)
          break;

      for (; i < e; ++i)
        {
        out[i] = (d1 - out[i-1]) / (e - i + 1);
        }

      --i;
      }
    }

}	/* END CLEANANDCOPYDATA */

/* -------------------------------------------------------------------- */
void DiffPreFilter(DATASET_INFO *set, float out[])
{
  int	i;

  for (i = set->nPoints-1; i > 0; --i)
    if (isMissingValue(out[i], set->missingValue) || isMissingValue(out[i-1], set->missingValue))
      out[i] = set->missingValue;
    else
      out[i] = out[i] - out[i-1];

  out[0] = 0.0;

}	/* END DIFFPREFILTER */

/* -------------------------------------------------------------------- */
void DetrendNone(DATASET_INFO *set, float out[])
{

}	/* END DETRENDNONE */

/* -------------------------------------------------------------------- */
void DetrendMean(DATASET_INFO *set, float out[])
{
  int		i;
  float		datum;

  for (i = 0; i < set->nPoints; ++i)
    {
    datum = out[i];

    if (isMissingValue(datum, set->missingValue))
      out[i] = out[i-1];
    else
      out[i] = datum - set->stats.mean;
    }

}	/* END DETRENDMEAN */

/* -------------------------------------------------------------------- */
void DetrendLinear(DATASET_INFO *set, float out[])
{
  int	i;
  float datum, *out1;

  out1 = (float *)GetMemory(set->nPoints * sizeof(float));

  LinearLeastSquare(set, out, out1);

  for (i = 0; i < set->nPoints; ++i)
    {
    datum = out[i];

    if (isMissingValue(datum, set->missingValue))
      out[i] = out[i-1];
    else
      out[i] = datum - out1[i];
    }

  free(out1);

}	/* END DETRENDLINEAR */

/* END DETREND.C */
