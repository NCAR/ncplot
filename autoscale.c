/*
-------------------------------------------------------------------------
OBJECT NAME:	autoscale.c

FULL NAME:	Auto Scale Axies

ENTRY POINTS:	AutoScaleXY()
		AutoScaleXYZ()
		AutoScaleSpec()
		AutoScaleDiff()
		AutoScale()

static FNS:	scaleAxis()
		scaleZaxis()

DESCRIPTION:	These procedures are responsible for auto scaling the
		X and Y axis.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"

bool equalScaling();

static void	scaleAxis(struct axisInfo *), scaleZaxis(struct axisInfo *);
static void	DoEqualScale(struct axisInfo *, struct axisInfo *);

static NR_TYPE	RoundBig(NR_TYPE x, int round), expt(NR_TYPE a, int n);

/* -------------------------------------------------------------------- */
void AutoScaleXY()
{
  int	p;

  for (p = 0; p < NumberOfXYpanels; ++p)
    if (xyyPlot[p].autoScale)
      {
      scaleAxis(&xyyPlot[p].Xaxis);
      scaleAxis(&xyyPlot[p].Yaxis[LEFT_SIDE]);
      scaleAxis(&xyyPlot[p].Yaxis[RIGHT_SIDE]);
      }

  if (equalScaling())
    DoEqualScale(&xyyPlot[0].Xaxis, &xyyPlot[0].Yaxis[LEFT_SIDE]);

  SetXYDefaults();

}

/* -------------------------------------------------------------------- */
void AutoScaleDiff()
{
  if (diffPlot.autoScale)
    {
    diffPlot.Yaxis[LEFT_SIDE].smallestValue = diffSet.stats.min;
    diffPlot.Yaxis[LEFT_SIDE].biggestValue = diffSet.stats.max;
    scaleAxis(&diffPlot.Yaxis[LEFT_SIDE]);
    SetDiffDefaults();
    }

}

/* -------------------------------------------------------------------- */
void AutoScaleSpec()
{
  if (specPlot.autoScale)
    {
    scaleAxis(&specPlot.Yaxis[LEFT_SIDE]);
    SetSpecDefaults();
    }
}

/* -------------------------------------------------------------------- */
void AutoScaleXYZ()
{
  if (!xyzPlot.autoScale)
    return;

  xyzPlot.Xaxis.smallestValue	= xyzSet[0].stats.min;
  xyzPlot.Xaxis.biggestValue 	= xyzSet[0].stats.max;
  xyzPlot.Yaxis[0].smallestValue= xyzSet[1].stats.min;
  xyzPlot.Yaxis[0].biggestValue = xyzSet[1].stats.max;
  xyzPlot.Zaxis.smallestValue	= xyzSet[2].stats.min;
  xyzPlot.Zaxis.biggestValue 	= xyzSet[2].stats.max;

  scaleAxis(&xyzPlot.Xaxis);
  scaleAxis(&xyzPlot.Yaxis[LEFT_SIDE]);
  scaleZaxis(&xyzPlot.Zaxis);

  if (equalScaling())
    DoEqualScale(&xyzPlot.Xaxis, &xyzPlot.Zaxis);

  SetTrackDefaults();
}

/* -------------------------------------------------------------------- */
static void DoEqualScale(struct axisInfo *xAxis, struct axisInfo *yAxis)
{
  double	deltaLat = yAxis->max - yAxis->min,
		deltaLon = xAxis->max - xAxis->min,
		meanLat = (yAxis->max + yAxis->min) / 2,
		meanLon = (xAxis->max + xAxis->min) / 2, d;

  deltaLon *= cos(meanLat * M_PI / 180.0);

  if (deltaLat > deltaLon)
    {
    d = deltaLat / cos(meanLat * M_PI / 180.0) / 2;

    xAxis->max = meanLon + d;
    xAxis->min = meanLon - d;
    }
  else
  if (deltaLat < deltaLon)
    {
    d = deltaLon / 2;

    yAxis->max = meanLat + d;
    yAxis->min = meanLat - d;
    }

}

/* -------------------------------------------------------------------- */
void AutoScale()
{
  int	i;

  if (NumberDataSets == 0)
    return;

  for (i = 0; i < NumberOfPanels; ++i)
    if (mainPlot[i].autoScale)
      {
      scaleAxis(&mainPlot[i].Yaxis[LEFT_SIDE]);
      scaleAxis(&mainPlot[i].Yaxis[RIGHT_SIDE]);
      }

  SetMainDefaults();

}	/* END AUTOSCALE */

/* -------------------------------------------------------------------- */
static void scaleAxis(struct axisInfo *axis)
{
  NR_TYPE	tmpmin, tmpmax, range, d;

  if (axis->smallestValue == FLT_MAX || axis->biggestValue == -FLT_MAX)
    {
    axis->min = 0;
    axis->max = 1;
    }
  else
    {
    axis->min = axis->smallestValue;
    axis->max = axis->biggestValue;
    }


  if (axis->logScale)
    {
    if (axis->min <= 0.0)
      axis->min = 1;

    if (axis->max <= 0.0)
      axis->max = 10;

    axis->min = pow(10.0, floor(log10(axis->min)));
    axis->max = pow(10.0, ceil(log10(axis->max)));
    }
  else
    {
    /* min better not equal max */
    if (axis->min == axis->max)
      axis->max += 1.0;

    range = RoundBig(axis->max - axis->min, False);
    d = RoundBig(range / axis->nMajorTics, True);  /* tic mark spacing	*/

    tmpmin = floor(axis->min / d) * d;
    tmpmax = ceil(axis->max / d) * d;

    axis->min = tmpmin;
    axis->max = tmpmax;
    }

}	/* END SCALEAXIS */

/* -------------------------------------------------------------------- */
static void scaleZaxis(struct axisInfo *Zaxis)
{
  NR_TYPE	tmpmin, tmpmax, range, d;

  if (Zaxis->smallestValue == FLT_MAX || Zaxis->biggestValue == -FLT_MAX)
    {
    Zaxis->min = 0;
    Zaxis->max = 1;
    }
  else
    {
    Zaxis->min = Zaxis->smallestValue;
    Zaxis->max = Zaxis->biggestValue;
    }

  /* min better not equal max */
  if (Zaxis->min == Zaxis->max)
    Zaxis->max += 1.0;

  range = RoundBig(Zaxis->max - Zaxis->min, False);
  d = RoundBig(range / 8, True);	/* tic mark spacing	*/

  tmpmin = floor(Zaxis->min / d) * d;
  tmpmax = ceil(Zaxis->max / d) * d;

  Zaxis->min = tmpmin;
  Zaxis->max = tmpmax;

}	/* END SCALEZAXIS */

/* -------------------------------------------------------------------- */
static NR_TYPE RoundBig(NR_TYPE x, int round)
{
  int	exp;
  NR_TYPE	f, y;

  exp = (int)floor(log10(x));
  f = x / expt(10.0, exp);     /* fraction between 1 and 10 */

  if (round)
    if (f < 1.5)
      y = 1.0;
    else
      if (f < 3.0)
        y = 2.0;
      else
        if (f < 7.0)
          y = 5.0;
        else
          y = 10.0;
   else
     if (f <= 1.0)
       y = 1.0;
     else
       if (f <= 2.0)
         y = 2.0;
       else
         if (f <= 5.0)
           y = 5.0;
         else
           y = 10.0;

  return(y * expt(10.0, exp));

}	/* END ROUNDBIG */

/* -------------------------------------------------------------------- */
static NR_TYPE expt(NR_TYPE a, int n)
{
  NR_TYPE	x;

  x = 1.0;

  if (n > 0)
    for (; n > 0; n--)
      x *= a;
  else
    for (; n < 0; n++)
      x /= a;

  return(x);

}	/* END EXPT */

/* END AUTOSCALE.C */
