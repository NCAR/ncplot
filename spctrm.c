/*
-------------------------------------------------------------------------
OBJECT NAME:	spctrm.c

FULL NAME:	Compute Spectrum

ENTRY POINTS:	Spectrum()
		CoSpectrum()

STATIC FNS:	none

DESCRIPTION:	Power Spectral Density estimation via the Welch averaging
		periodogram.

REFERENCED BY:	spec.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2022
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <gsl/gsl_fft_complex.h>

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])


/* -------------------------------------------------------------------- */
double Spectrum(float data[],		/* Input data			*/
		PSD_INFO *psd,
		size_t nPoints)
{
  size_t i, seg, segP, twoM, M = psd->M, K = psd->K;
  double KU, Wss, *currentSegment, *Window, variance;


  twoM = M << 1;

  currentSegment = new double[twoM * 2];	// Imaginary is interleaved.

  /* Output will consist of M + 1 points.
   */
  for (i = 0; i <= M; ++i)
    {
    psd->Pxx[i] = 0.0;
    if (psd->Real) psd->Real[i] = 0.0;
    if (psd->Imaginary) psd->Imaginary[i] = 0.0;
    }


  /* Generate Window & compute Window Squared and Summed.
   */
  Window = new double[twoM];

  for (i = 0, Wss = 0.0; i < twoM; ++i)
    {
    Window[i] = (*psd->windowFn)(i, twoM);
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
      REAL(currentSegment,i) = data[segP+i] * Window[i];
      IMAG(currentSegment,i) = 0.0;
      }

    gsl_fft_complex_radix2_forward(currentSegment, 1, twoM);

    psd->Pxx[0] += (REAL(currentSegment,0) * REAL(currentSegment,0)) +
		(IMAG(currentSegment,0) * IMAG(currentSegment,0));

    for (i = 1; i < M; ++i)
      psd->Pxx[i] += (REAL(currentSegment,i) * REAL(currentSegment,i)) +
                (IMAG(currentSegment,i) * IMAG(currentSegment,i)) +
                (REAL(currentSegment,twoM-i) * REAL(currentSegment,twoM-i)) +
                (IMAG(currentSegment,twoM-i) * IMAG(currentSegment,twoM-i));

    psd->Pxx[M] += (REAL(currentSegment,M) * REAL(currentSegment,M)) +
              (IMAG(currentSegment,M) * IMAG(currentSegment,M));

    if (psd->Real)
      {
      for (i = 0; i <= M; ++i)
        psd->Real[i] += REAL(currentSegment, i);
      }

    if (psd->Imaginary)
      {
      for (i = 0; i <= M; ++i)
        psd->Imaginary[i] += IMAG(currentSegment, i);
      }
    }


  /* Normalize, and compute variance.
   */
  variance = 0.0;

  for (i = 0; i <= M; ++i)
    {
    psd->Pxx[i] /= KU;
    variance += psd->Pxx[i];
    }

  delete [] Window;
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
  size_t i, seg, segP, twoM;
  double KU, Wss, *Window;
  double *currentSegment1, *currentSegment2, variance;

  twoM = M << 1;

  currentSegment1 = new double[twoM * 2];
  currentSegment2 = new double[twoM * 2];

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
      REAL(currentSegment1, i) = data1[segP+i] * Window[i];
      REAL(currentSegment2, i) = data2[segP+i] * Window[i];
      IMAG(currentSegment1, i) = IMAG(currentSegment2, i) = 0.0;
      }

    gsl_fft_complex_radix2_forward(currentSegment1, 1, twoM);
    gsl_fft_complex_radix2_forward(currentSegment2, 1, twoM);


    Pxx[0] += (REAL(currentSegment1, 0) * REAL(currentSegment2, 0)) +
	      (IMAG(currentSegment1, 0) * IMAG(currentSegment2, 0));

    Qxx[0] += (IMAG(currentSegment1, 0) * REAL(currentSegment2, 0)) -
	      (REAL(currentSegment1, 0) * IMAG(currentSegment2, 0));

    for (i = 1; i < M; ++i)
      {
      Pxx[i] += (REAL(currentSegment1, i) * REAL(currentSegment2, i)) +
                (IMAG(currentSegment1, i) * IMAG(currentSegment2, i)) +
                (REAL(currentSegment1, twoM-i) * REAL(currentSegment2, twoM-i)) +
                (IMAG(currentSegment1, twoM-i) * IMAG(currentSegment2, twoM-i));

      Qxx[i] += (IMAG(currentSegment1, i) * REAL(currentSegment2, i)) -
                (REAL(currentSegment1, i) * IMAG(currentSegment2, i)) -
                (IMAG(currentSegment1, twoM-i) * REAL(currentSegment2, twoM-i)) +
                (REAL(currentSegment1, twoM-i) * IMAG(currentSegment2, twoM-i));
      }

    Pxx[M] += (REAL(currentSegment1, M) * REAL(currentSegment2, M)) +
              (IMAG(currentSegment1, M) * IMAG(currentSegment2, M));

    Qxx[M] += (-IMAG(currentSegment1, M) * REAL(currentSegment2, M)) +
              (REAL(currentSegment1, M) * IMAG(currentSegment2, M));
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
  delete [] currentSegment2;
  delete [] currentSegment1;

  return(variance);

}	/* END COSPECTRUM */

/* END SPCTRM.C */
