/*
-------------------------------------------------------------------------
OBJECT NAME:	cospec.c

FULL NAME:	Co-Spectrum

ENTRY POINTS:	ComputeCoSpectrum()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	spctrm.c

REFERENCED BY:	Callback

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2004
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

extern Widget SpectrumWindow;


/* -------------------------------------------------------------------- */
void ComputeCoSpectrum()
{
  int		i, pos, KxM, ts;
  double	variance1, variance2, *Pxx1, *Pxx2;
  float		*detrendedData[2], cf;

  if (NumberDataSets < 2 || dataSet[0].nPoints != dataSet[1].nPoints)
    {
    fprintf(stderr,
	"Unequal number if data points in sets, prepare for core dump.\n");
    return;
    }

  for (i = 0; i < MAX_PSD; ++i)
    {
    if (psd[i].Pxx) free(psd[i].Pxx);
    if (psd[i].Qxx) free(psd[i].Qxx);
    if (psd[i].Special) free(psd[i].Special);

    psd[i].Pxx = psd[i].Qxx = psd[i].Special = NULL;
    }


  /* 50% overlapping.
   */
  if ((psd[0].K = dataSet[0].nPoints / psd[0].M) < 1)
    psd[0].K = 1;

  psd[0].K += 2;
  KxM = (psd[0].K + 1) * psd[0].M;

  detrendedData[0] = (float *)GetMemory(sizeof(float) * KxM);
  detrendedData[1] = (float *)GetMemory(sizeof(float) * KxM);
  psd[0].Pxx	= (double *)GetMemory(sizeof(double) * (psd[0].M+1));
  psd[0].Qxx	= (double *)GetMemory(sizeof(double) * (psd[0].M+1));


  pos = (KxM - dataSet[0].nPoints) / 2;
  memset((char *)detrendedData[0], 0, sizeof(float) * KxM);
  memset((char *)detrendedData[1], 0, sizeof(float) * KxM);

  CleanAndCopyData(&dataSet[0], &detrendedData[0][pos]);
  CleanAndCopyData(&dataSet[1], &detrendedData[1][pos]);

  (*psd[0].detrendFn)(&dataSet[0], &detrendedData[0][pos]);
  (*psd[0].detrendFn)(&dataSet[1], &detrendedData[1][pos]);


  ts = timeShift();

  if (ts < 0)
    {
    int nSamples = (int)(-ts / (1000 / psd[0].frequency));

    memcpy((char *)&detrendedData[1][pos],
           (char *)&detrendedData[1][pos + nSamples],
           sizeof(float) * dataSet[0].nPoints);

    memset((char *)&detrendedData[1][pos+dataSet[0].nPoints], 0,
		nSamples * sizeof(float));
    }

  if (ts > 0)
    {
    int nSamples = (int)(ts / (1000 / psd[0].frequency));
    float *f = (float *)GetMemory(sizeof(float) * dataSet[0].nPoints);

    memcpy((char *)f, (char *)&detrendedData[1][pos],
           sizeof(float) * dataSet[0].nPoints);

    memset((char *)&detrendedData[1][pos], 0, nSamples * sizeof(float));

    memcpy((char *)&detrendedData[1][pos + nSamples], (char *)f,
           sizeof(float) * dataSet[0].nPoints);

    free(f);
    }


  psd[0].totalVariance =
      CoSpectrum(detrendedData[0], detrendedData[1], psd[0].Pxx, psd[0].Qxx,
		psd[0].K, psd[0].M, psd[0].windowFn, dataSet[0].nPoints);


  specPlot.Yaxis[0].smallestValue = 100000.0;
  specPlot.Yaxis[0].biggestValue = -100000.0;
  cf = (double)(psd[0].M << 1) / psd[0].frequency;

  if (psd[0].display == COSPECTRA)
    {
    sprintf(specPlot.Yaxis[0].label, "CoSpectrum of %s x %s",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name);

    ComputeBandLimitedVariance(NULL, NULL, NULL);

    for (i = 1; i <= psd[0].M; ++i)
      {
      if (multiplyByFreq())
        {
        psd[0].Pxx[i] *= i;

        sprintf(specPlot.Yaxis[0].label, "f x Co of %s x %s (%s^2 x %s^2)",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name,
            dataSet[0].stats.units, dataSet[1].stats.units);
        }
      else
      if (multiplyByFreq53())
        {
        psd[0].Pxx[i] *= pow((double)i, 5.0/3.0);

        sprintf(specPlot.Yaxis[0].label, "f^(5/3) x Co of %s x %s (%s^2 x %s^2)",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name,
            dataSet[0].stats.units, dataSet[1].stats.units);
        }
      else
        {
        psd[0].Pxx[i] *= cf;

        sprintf(specPlot.Yaxis[0].label, "Co of %s x %s (%s^2 x %s^2 / Hz)",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name,
            dataSet[0].stats.units, dataSet[1].stats.units);
        }

      specPlot.Yaxis[0].smallestValue =
           MIN(specPlot.Yaxis[0].smallestValue, psd[0].Pxx[i]);
      specPlot.Yaxis[0].biggestValue =
           MAX(specPlot.Yaxis[0].biggestValue, psd[0].Pxx[i]);
      }
    }

  if (psd[0].display == QUADRATURE)
    {
    sprintf(specPlot.Yaxis[0].label, "Quadrature of %s x %s",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name);

    for (i = 1; i <= psd[0].M; ++i)
      {
      if (multiplyByFreq())
        {
        psd[0].Qxx[i] *= i;

        sprintf(specPlot.Yaxis[0].label, "f x Qxx of %s x %s (%s^2 x %s^2)",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name,
            dataSet[0].stats.units, dataSet[1].stats.units);
        }
      else
        {
        psd[0].Qxx[i] *= cf;

        sprintf(specPlot.Yaxis[0].label, "Qxx of %s x %s (%s^2 x %s^2 / Hz)",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name,
            dataSet[0].stats.units, dataSet[1].stats.units);
        }

      specPlot.Yaxis[0].smallestValue =
             MIN(specPlot.Yaxis[0].smallestValue, psd[0].Qxx[i]);
      specPlot.Yaxis[0].biggestValue =
             MAX(specPlot.Yaxis[0].biggestValue, psd[0].Qxx[i]);
      }
    }

  if (psd[0].display == COHERENCE || psd[0].display == RATIO)
    {
    psd[0].Special = (double *)GetMemory(sizeof(double) * (psd[0].M+1));

    Pxx1 = (double *)GetMemory(sizeof(double) * (psd[0].M+1));
    Pxx2 = (double *)GetMemory(sizeof(double) * (psd[0].M+1));

    CleanAndCopyData(&dataSet[0], &detrendedData[0][pos]);
    CleanAndCopyData(&dataSet[1], &detrendedData[1][pos]);

    (*psd[0].detrendFn)(&dataSet[0], &detrendedData[0][pos]);
    (*psd[0].detrendFn)(&dataSet[1], &detrendedData[1][pos]);

    variance1 = Spectrum(detrendedData[0], Pxx1, psd[0].K, psd[0].M, psd[0].windowFn,
						dataSet[0].nPoints);
    variance2 = Spectrum(detrendedData[1], Pxx2, psd[0].K, psd[0].M, psd[0].windowFn,
						dataSet[1].nPoints);

    if (psd[0].display == COHERENCE)
      {
      sprintf(specPlot.Yaxis[0].label, "Coherence of %s x %s",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name);

      for (i = 1; i <= psd[0].M; ++i)
        psd[0].Special[i] =
          sqrt((psd[0].Pxx[i] * psd[0].Pxx[i] + psd[0].Qxx[i] * psd[0].Qxx[i]) /
                               (Pxx1[i] * Pxx2[i]));
      }
    else
    if (psd[0].display == RATIO)
      {
      sprintf(specPlot.Yaxis[0].label, "Ratio of %s / %s",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name);

      for (i = 1; i <= psd[0].M; ++i)
        psd[0].Special[i] = Pxx1[i] / Pxx2[i];
      }


    printf("variance1=%f, variance2=%f\n", variance1, variance2);
    free(Pxx1);
    free(Pxx2);
    }

  if (psd[0].display == PHASE)
    {
    psd[0].Special = (double *)GetMemory(sizeof(double) * (psd[0].M+1));

    sprintf(specPlot.Yaxis[0].label, "Phase of %s x %s",
            dataSet[0].varInfo->name, dataSet[1].varInfo->name);

    for (i = 1; i <= psd[0].M; ++i)
      psd[0].Special[i] = atan2(psd[0].Qxx[i], psd[0].Pxx[i]) * 180.0 / M_PI;
    }

  free(detrendedData[0]);
  free(detrendedData[1]);

}	/* END COSPECTRUM */

/* END COSPEC.C */
