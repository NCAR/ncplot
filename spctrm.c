/*
-------------------------------------------------------------------------
OBJECT NAME:	spctrm.c

FULL NAME:	Compute Spectrum

ENTRY POINTS:	Spectrum()
		CoSpectrum()

STATIC FNS:	none

DESCRIPTION:	Power Spectral Density estimation via the Welch averaging
		periodogram.

INPUT:		

OUTPUT:		Total Variance

REFERENCES:	fft.c

REFERENCED BY:	ncplot.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"


/* -------------------------------------------------------------------- */
double Spectrum(float data[],		/* Input data			*/
		double Pxx[],		/* Output data			*/
		size_t K,		/* K overlapping segments	*/
		size_t M,		/* 2M points per segment	*/
		double (*window)(int, int),	/* Window function	*/
		size_t nPoints)
{
  size_t i, seg, segP, Base = 1, twoM;
  double KU, Wss, *currentSegment, *imaginary, *Window, variance;


  twoM = M << 1;

  currentSegment = new double[twoM];
  imaginary = new double[twoM];


  /* Figure out 2^Base.
   */
  for (i = 0; i < 32; ++i)
    if ((0x00000001 << i) & twoM)
      Base = i;


  /* Ouptut will consist of M + 1 points.
   */
  for (i = 0; i <= M; ++i)
    Pxx[i] = 0.0;


  /* Generate Window & compute Window Squared and Summed.
   */
  Window = new double[twoM];

  for (i = 0, Wss = 0.0; i < twoM; ++i)
    {
    Window[i] = (*window)(i, twoM);
    Wss += Window[i] * Window[i];
    }

  KU = K * Wss * twoM;

{
/* Following was added by Bill Anderson 4/97.  Verified by Lenschow.
 */
  /* Compute variance before and after windowing.
   */
  double d, varianceBef = 0.0, varianceAft = 0.0, scaleFactor;

  for (segP = seg = 0; seg < K; ++seg, segP += M)
    for (i = 0; i < twoM; ++i)
      {
      d = data[segP + i];

      varianceBef += d * d;
      varianceAft += (d * Window[i]) * (d * Window[i]);
      }

  varianceBef /= (K * twoM);
  varianceAft /= (K * Wss);
/*
  printf("Variance of detrended data (w/ zeroes) = %f\n", varianceBef);
  printf("Variance of detrended & windowed data = %f\n", varianceAft);
*/
  /* Now scale the time series by the ratio of the variances to remove
   * the effect of windowing and by the different numbers of points in
   * the data and actually used, to remove the effect of zero padding.
   */
  scaleFactor = sqrt(varianceBef / varianceAft) * sqrt(K*M / (double)nPoints);
  for (i = 0; i < (K+1)*M; ++i)
    data[i] *= scaleFactor;
}


  /* Do Segments.
   */
  for (segP = seg = 0; seg < K; ++seg, segP += M)
    {
    for (i = 0; i < twoM; ++i)
      {
      currentSegment[i] = data[segP+i] * Window[i];
      imaginary[i] = 0.0;
      }

    fft(currentSegment, imaginary, Base, 1);

    Pxx[0] += (currentSegment[0] * currentSegment[0]) +
              (imaginary[0] * imaginary[0]);

    for (i = 1; i < M; ++i)
      Pxx[i] +=	(currentSegment[i] * currentSegment[i]) +
                (imaginary[i] * imaginary[i]) +
                (currentSegment[twoM-i] * currentSegment[twoM-i]) +
                (imaginary[twoM-i] * imaginary[twoM-i]);

    Pxx[M] += (currentSegment[M] * currentSegment[M]) +
              (imaginary[M] * imaginary[M]);
    }


  /* Normalize, and compute variance.
   */
  variance = 0.0;

  for (i = 0; i <= M; ++i)
    {
    Pxx[i] /= KU;
    variance += Pxx[i];
    }

  delete [] Window;
  delete [] imaginary;
  delete [] currentSegment;

  return(variance);

}	/* END SPECTRUM */

/* -------------------------------------------------------------------- */
double CoSpectrum(float data1[],	/* Input data			*/
		float data2[],		/* Input data			*/
		double Pxx[],		/* Output CoSpectrum		*/
		double Qxx[],		/* Output Quadrature		*/
		size_t K,		/* K overlapping segments	*/
		size_t M,		/* 2M points per segment	*/
		double (*window)(int, int),	/* Window function	*/
		size_t nPoints)
{
  size_t i, seg, segP, Base = 0, twoM;
  double KU, Wss, *Window;
  double *currentSegment1, *imaginary1, *currentSegment2, *imaginary2,
	variance;

  twoM = M << 1;

  currentSegment1 = new double[twoM];
  currentSegment2 = new double[twoM];
  imaginary1 = new double[twoM];
  imaginary2 = new double[twoM];


  /* Figure out 2^Base.
   */
  for (i = 0; i < 32; ++i)
    if ((0x00000001 << i) & twoM)
      Base = i;


  for (i = 0; i <= M; ++i)
    Pxx[i] = Qxx[i] = 0.0;


  /* Generate Window & compute Window Squared and Summed.
   */
  Window = new double[twoM];

  for (i = 0, Wss = 0.0; i < twoM; ++i)
    {
    Window[i] = (*window)(i, twoM);
    Wss += Window[i] * Window[i];
    }

  KU = Wss * K * twoM;

{
/* Following was added by Bill Anderson 4/97.  Verified by Lenschow.
 */
  /* Compute variance before and after windowing.
   */
  double d1, d2, varianceBef = 0.0, varianceAft = 0.0, scaleFactor1,
         scaleFactor2;

  for (segP = seg = 0; seg < K; ++seg, segP += M)
    for (i = 0; i < twoM; ++i)
      {
      d1 = data1[segP + i];
      d2 = data2[segP + i];

      varianceBef += d1 * d2;
      varianceAft += (d1 * Window[i]) * (d2 * Window[i]);
      }

  varianceBef /= (K * twoM);
  varianceAft /= (K * Wss);
/*
  printf("Co-variance of detrended data (w/ zeroes) = %f\n", varianceBef);
  printf("Co-variance of detrended & windowed data = %f\n", varianceAft);
*/
  /* Now scale the time series by the ratio of the variances to remove
   * the effect of windowing and by the different numbers of points in
   * the data and actually used, to remove the effect of zero padding.
   */
  scaleFactor1 = sqrt(varianceBef / varianceAft) * sqrt(K*M / (double)nPoints);
  scaleFactor2 = sqrt(varianceBef / varianceAft) * sqrt(K*M / (double)nPoints);

  if (varianceBef / varianceAft < 0.0)
    scaleFactor1 = -scaleFactor1;

  for (i = 0; i < (K+1)*M; ++i)
    {
    data1[i] *= scaleFactor1;
    data2[i] *= scaleFactor2;
    }
}


  /* Do Segments.
  */
  for (segP = seg = 0; seg < K; ++seg, segP += M)
    {
    for (i = 0; i < twoM; ++i)
      {
      currentSegment1[i] = data1[segP+i] * Window[i];
      currentSegment2[i] = data2[segP+i] * Window[i];
      imaginary1[i] = imaginary2[i] = 0.0;
      }

    fft(currentSegment1, imaginary1, Base, 1);
    fft(currentSegment2, imaginary2, Base, 1);


    Pxx[0] += (currentSegment1[0] * currentSegment2[0]) +
	      (imaginary1[0] * imaginary2[0]);

    Qxx[0] += (imaginary1[0] * currentSegment2[0]) -
	      (currentSegment1[0] * imaginary2[0]);

    for (i = 1; i < M; ++i)
      {
      Pxx[i] += (currentSegment1[i] * currentSegment2[i]) +
                (imaginary1[i] * imaginary2[i]) +
                (currentSegment1[twoM-i] * currentSegment2[twoM-i]) +
                (imaginary1[twoM-i] * imaginary2[twoM-i]);

      Qxx[i] += (imaginary1[i] * currentSegment2[i]) -
                (currentSegment1[i] * imaginary2[i]) -
                (imaginary1[twoM-i] * currentSegment2[twoM-i]) +
                (currentSegment1[twoM-i] * imaginary2[twoM-i]);
      }

    Pxx[M] += (currentSegment1[M] * currentSegment2[M]) +
              (imaginary1[M] * imaginary2[M]);

    Qxx[M] += (-imaginary1[M] * currentSegment2[M]) +
              (currentSegment1[M] * imaginary2[M]);
    }


  /* Normalize, and compute variance.
   */
  variance = 0.0;

  for (i = 0; i <= M; ++i)
    {
    Pxx[i] /= KU;
    Qxx[i] /= -KU;	/* (-1) introduced by Al Cooper		*/
    variance += Pxx[i];
    }

  delete [] Window;
  delete [] imaginary2;
  delete [] imaginary1;
  delete [] currentSegment2;
  delete [] currentSegment1;

  return(variance);

}	/* END COSPECTRUM */

/* END SPCTRM.C */
