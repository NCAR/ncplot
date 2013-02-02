/* $Id: regret.c,v 4.6 2002/12/10 22:14:24 chris Exp $
 *
 * This code was stolen from Paul Turners xmgr package, and tinkered with.
 *
 * curve fitting, and other numerical routines used in compose.
 *
 * Contents:
 *
 * void gauss() - simple gauss elimination for least squares poly fit
 * void stasum() - compute mean and variance
 * void leasqu() - entry to linear or polynoimial regression routines
 * void fitcurve() - compute coefficients for a polynomial fit of degree >1
 * int linear_regression() - linear regression
 */

#include "define.h"

static const int MAXFIT = 12;

static int linear_regression(DATASET_INFO *x, DATASET_INFO *y);
static void gauss(int n, double *a, int adim, double *b, double *x);
static void stasum(DATASET_INFO *x, double *xbar, double *sd, int flag);
static void leasqu(DATASET_INFO *x, DATASET_INFO *y, int degree, double *w, int wdim, double *r);

/* -------------------------------------------------------------------- */
static void gauss(int n, double *a, int adim, double *b, double *x)
{
  int		i, k, j;
  double	mult;

  for (k = 0; k < n - 1; k++)
    {
    for (i = k + 1; i < n; i++)
      {
      mult = a[adim * i + k] / a[adim * k + k];

      for (j = k + 1; j < n; j++)
        a[adim * i + j] -= mult * a[adim * k + j];

      b[i] -= mult * b[k];
      }
    }

  for (i = n - 1; i >= 0; i--)
    {
    x[i] = b[i];

    for (j = i + 1; j < n; j++)
      x[i] -= a[adim * i + j] * x[j];

    x[i] /= a[adim * i + i];
    }
}

/* -------------------------------------------------------------------- */
static void stasum(DATASET_INFO *set, double *xbar, double *sd, int flag)
{
  size_t i, cnt = 0;

  *xbar = 0;
  *sd = 0;

  if (set->nPoints < 1)
    return;

  for (i = 0; i < set->nPoints; i++)
    if (!isMissingValue(set->data[i], set->missingValue))
      {
      *xbar += set->data[i];
      ++cnt;
      }

  *xbar /= cnt;

  for (i = 0; i < set->nPoints; i++)
    if (!isMissingValue(set->data[i], set->missingValue))
      *sd += (set->data[i] - *xbar) * (set->data[i] - *xbar);

  if (cnt - flag)
    *sd = sqrt(*sd / (cnt - flag));
  else
    {
    fprintf(stderr, "compmean: (n-flag) == 0.0\n");
    *sd = 0;
    }
}

/* -------------------------------------------------------------------- */
static void leasqu(DATASET_INFO *x, DATASET_INFO *y, int degree, double *w, int wdim, double *r)
{
  size_t	i;
  int		j, k;
  double	b[11];
  double	sumy1, sumy2, ybar, ysdev, stemp, rsqu;
  double	xbar, xsdev;

  sumy1 = 0.0;
  sumy2 = 0.0;

  /* form the matrix with normal equations and RHS */
  for (k = 0; k <= degree; k++)
    {
    for (j = k; j <= degree; j++)
      {
      w[wdim * k + j] = 0.0;

      for (i = 0; i < x->nPoints; i++)
        if (x->data[i] != 0.0)
          w[wdim * k + j] += pow(x->data[i], (double)k) * pow(x->data[i], (double)j);

      if (k != j)
        w[wdim * j + k] = w[wdim * k + j];
      }
    }

  for (k = 0; k <= degree; k++)
    {
    b[k] = 0.0;

    for (i = 0; i < x->nPoints; i++)
      if (x->data[i] != 0.0)
        b[k] += pow(x->data[i], (double)k) * y->data[i];
    }

  gauss(degree + 1, w, wdim, b, r);	/* solve */
  stasum(y, &ybar, &ysdev, 1);	/* compute statistics on fit */
  stasum(x, &xbar, &xsdev, 1);

  for (i = 0; i < x->nPoints; i++)
    {
    stemp = 0.0;

    for (j = 1; j <= degree; j++)
      if (x->data[i] != 0.0)
        stemp += r[j] * pow(x->data[i], (double)j);

    sumy1 += (stemp + r[0] - y->data[i]) * (stemp + r[0] - y->data[i]);
    sumy2 += y->data[i] * y->data[i];
    }

  if ((rsqu = 1.0 - sumy1 / (sumy2 - x->nPoints * ybar * ybar)) < 0.0)
    rsqu = 0.0;


  printf("Number of data points = %zu\n", x->nPoints);
  printf("A[0] is the constant, A[i] is the coefficient for ith power of X\n");

  for (i = 0; i <= (size_t)degree; i++)
    printf("A[%zu] = %15.8f\n", i, r[i]);

  printf("R square = %15.8f\n", rsqu);
  printf("Avg Y    = %15.8f\n", ybar);
  printf("Sdev Y   = %15.8f\n", ysdev);
  printf("Avg X    = %15.8f\n", xbar);
  printf("Sdev X   = %15.8f\n\n", xsdev);
}

/* -------------------------------------------------------------------- */
void fitcurve(DATASET_INFO *x, DATASET_INFO *y, int ideg)
{
  int		ifail = 1;
  double	w[MAXFIT * MAXFIT];

  if (ideg > 1)
    {
    leasqu(x, y, ideg, w, MAXFIT, regretCo);
    }
  else
    {
    ifail = linear_regression(x, y);

    if (ifail == 1)
      fprintf(stderr, "Linear_regression entered with N <= 3.\n");
    else
    if (ifail == 2)
      fprintf(stderr,"Linear_regression, all values of x or y are the same.\n");
    }

}

/* -------------------------------------------------------------------- */
static int linear_regression(DATASET_INFO *x, DATASET_INFO *y)
{
  size_t	i, cnt = 0;
  double	xbar, ybar;	/* sample means			*/
  double	sdx, sdy;	/* sample standard deviations	*/
  double	sxy, rxy;	/* sample covariance and sample correlation */
  double	SXX, SYY, SXY;	/* sums of squares		*/
  double	RSS;		/* residual sum of squares	*/
  double	rms;		/* residual mean square		*/
  double	sereg;		/* standard error of regression */
  double	seslope, seintercept;
  double	slope, intercept;
  double	SSreg, F, R2;

  if (x->nPoints <= 3)
    return(1);

  xbar = ybar = 0.0;
  SXX = SYY = SXY = 0.0;

  for (i = 0; i < x->nPoints; i++)
    {
    if (!isMissingValue(x->data[i], x->missingValue) && !isMissingValue(y->data[i], y->missingValue))
      {
      xbar += x->data[i];
      ybar += y->data[i];
      ++cnt;
      }
    }

  xbar = xbar / x->nPoints;
  ybar = ybar / x->nPoints;

  for (i = 0; i < x->nPoints; i++)
    {
    if (!isMissingValue(x->data[i], x->missingValue) && !isMissingValue(y->data[i], y->missingValue))
      {
      SXX += (x->data[i] - xbar) * (x->data[i] - xbar);
      SYY += (y->data[i] - ybar) * (y->data[i] - ybar);
      SXY += (x->data[i] - xbar) * (y->data[i] - ybar);
      }
    }

  sdx = sqrt(SXX / (cnt - 1));
  sdy = sqrt(SYY / (cnt - 1));

  if (sdx == 0.0 || sdy == 0.0)
    return(2);

  sxy		= SXY / (cnt - 1);
  rxy		= sxy / (sdx * sdy);
  slope		= SXY / SXX;
  intercept	= ybar - slope * xbar;
  RSS		= SYY - slope * SXY;
  rms		= RSS / (cnt - 2);
  sereg		= sqrt(RSS / (cnt - 2));
  seintercept	= sqrt(rms * (1.0 / cnt + xbar * xbar / SXX));
  seslope	= sqrt(rms / SXX);
  SSreg		= SYY - RSS;
  F		= SSreg / rms;
  R2		= SSreg / SYY;

  printf("Number of data points\t\t = %6zu\n", cnt);
  printf("Mean of independent variable\t = %15.8f\n", xbar);
  printf("Mean of dependent variable\t = %15.8f\n", ybar);
  printf("Standard dev. of ind. variable\t = %15.8f\n", sdx);
  printf("Standard dev. of dep. variable\t = %15.8f\n", sdy);
  printf("Correlation coefficient\t\t = %15.8f\n", rxy);
  printf("Regression coefficient (SLOPE)\t = %15.8f\n", slope);
  printf("Standard error of coefficient\t = %15.8f\n", seslope);
  printf("t - value for coefficient\t = %15.8f\n", slope / seslope);
  printf("Regression constant (INTERCEPT)\t = %15.8f\n", intercept);
  printf("Standard error of constant\t = %15.8f\n", seintercept);
  printf("t - value for constant\t\t = %15.8f\n", intercept / seintercept);
  printf("\nAnalysis of variance\n");
  printf("Source\t\t d.f\t Sum of squares\t Mean Square\t F\n");
  printf("Regression\t   1\t%.7g\t%.7g\t%.7g\n", SSreg, SSreg, F);
  printf("Residual\t%5zu\t%.7g\t%.7g\n", cnt - 2, RSS, RSS / (cnt - 2));
  printf("Total\t\t%5zu\t%.7g\n\n", cnt - 1, SYY);

  regretCo[0] = intercept;
  regretCo[1] = slope;

  return(0);

}

/* END REGRET.C */
