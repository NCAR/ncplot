/*
-------------------------------------------------------------------------
OBJECT NAME:	cospec.c

FULL NAME:	Co-Spectrum

ENTRY POINTS:	ComputeCoSpectrum()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	spctrm.c

REFERENCED BY:	Callback

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2005
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

extern Widget SpectrumWindow;


/* -------------------------------------------------------------------- */
void ComputeCoSpectrum()
{
  size_t	i;
  int		pos, KxM, ts;
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
    if (psd[i].Pxx) delete [] psd[i].Pxx;
    if (psd[i].Qxx) delete [] psd[i].Qxx;
    if (psd[i].Special) delete [] psd[i].Special;

    psd[i].Pxx = psd[i].Qxx = psd[i].Special = NULL;
    }


  /* 50% overlapping.
   */
  if ((psd[0].K = dataSet[0].nPoints / psd[0].M) < 1)
    psd[0].K = 1;

  psd[0].K += 2;
  KxM = (psd[0].K + 1) * psd[0].M;

  detrendedData[0] = new float[KxM];
  detrendedData[1] = new float[KxM];
  psd[0].Pxx	= new double[psd[0].M+1];
  psd[0].Qxx	= new double[psd[0].M+1];


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
    float *f = new float[dataSet[0].nPoints];

    memcpy((char *)f, (char *)&detrendedData[1][pos],
           sizeof(float) * dataSet[0].nPoints);

    memset((char *)&detrendedData[1][pos], 0, nSamples * sizeof(float));

    memcpy((char *)&detrendedData[1][pos + nSamples], (char *)f,
           sizeof(float) * dataSet[0].nPoints);

    delete [] f;
    }


  psd[0].totalVariance =
      CoSpectrum(detrendedData[0], detrendedData[1], psd[0].Pxx, psd[0].Qxx,
		psd[0].K, psd[0].M, psd[0].windowFn, dataSet[0].nPoints);


  specPlot.Yaxis[0].smallestValue = 100000.0;
  specPlot.Yaxis[0].biggestValue = -100000.0;
  cf = (double)(psd[0].M << 1) / psd[0].frequency;

  std::string units0 = dataSet[0].stats.units;
  std::string units1 = dataSet[1].stats.units;
  if (units0.find('/') != std::string::npos)
    units0 = '(' + units0 + ')';
  if (units1.find('/') != std::string::npos)
    units1 = '(' + units1 + ')';

  if (psd[0].display == COSPECTRA)
    {
    snprintf(buffer, BUFFSIZE, "CoSpectrum of %s x %s",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str());
    specPlot.Yaxis[0].label = buffer;

    ComputeBandLimitedVariance(NULL, NULL, NULL);

    for (i = 1; i <= psd[0].M; ++i)
      {
      if (multiplyByFreq())
        {
        psd[0].Pxx[i] *= i;

        snprintf(buffer, BUFFSIZE, "f x Co of %s x %s (%s^2 x %s^2)",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str(),
            units0.c_str(), units1.c_str());
        specPlot.Yaxis[0].label = buffer;
        }
      else
      if (multiplyByFreq53())
        {
        psd[0].Pxx[i] *= pow((double)i, 5.0/3.0);

        snprintf(buffer, BUFFSIZE, "f^(5/3) x Co of %s x %s (%s^2 x %s^2)",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str(),
            units0.c_str(), units1.c_str());
        specPlot.Yaxis[0].label = buffer;
        }
      else
        {
        psd[0].Pxx[i] *= cf;

        snprintf(buffer, BUFFSIZE, "Co of %s x %s (%s^2 x %s^2 / Hz)",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str(),
            units0.c_str(), units1.c_str());
        specPlot.Yaxis[0].label = buffer;
        }

      specPlot.Yaxis[0].smallestValue =
           std::min(specPlot.Yaxis[0].smallestValue, psd[0].Pxx[i]);
      specPlot.Yaxis[0].biggestValue =
           std::max(specPlot.Yaxis[0].biggestValue, psd[0].Pxx[i]);
      }
    }

  if (psd[0].display == QUADRATURE)
    {
    snprintf(buffer, BUFFSIZE, "Quadrature of %s x %s",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str());
    specPlot.Yaxis[0].label = buffer;

    for (i = 1; i <= psd[0].M; ++i)
      {
      if (multiplyByFreq())
        {
        psd[0].Qxx[i] *= i;

        snprintf(buffer, BUFFSIZE, "f x Qxx of %s x %s (%s^2 x %s^2)",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str(),
            units0.c_str(), units1.c_str());
        specPlot.Yaxis[0].label = buffer;
        }
      else
        {
        psd[0].Qxx[i] *= cf;

        snprintf(buffer, BUFFSIZE, "Qxx of %s x %s (%s^2 x %s^2 / Hz)",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str(),
            units0.c_str(), units1.c_str());
        specPlot.Yaxis[0].label = buffer;
        }

      specPlot.Yaxis[0].smallestValue =
             std::min(specPlot.Yaxis[0].smallestValue, psd[0].Qxx[i]);
      specPlot.Yaxis[0].biggestValue =
             std::max(specPlot.Yaxis[0].biggestValue, psd[0].Qxx[i]);
      }
    }

  if (psd[0].display == COHERENCE || psd[0].display == RATIO)
    {
    psd[0].Special = new double[psd[0].M+1];

    Pxx1 = new double[psd[0].M+1];
    Pxx2 = new double[psd[0].M+1];

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
      snprintf(buffer, BUFFSIZE, "Coherence of %s x %s",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str());
      specPlot.Yaxis[0].label = buffer;

      for (i = 1; i <= psd[0].M; ++i)
        psd[0].Special[i] =
          sqrt((psd[0].Pxx[i] * psd[0].Pxx[i] + psd[0].Qxx[i] * psd[0].Qxx[i]) /
                               (Pxx1[i] * Pxx2[i]));
      }
    else
    if (psd[0].display == RATIO)
      {
      snprintf(buffer, BUFFSIZE, "Ratio of %s / %s",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str());
      specPlot.Yaxis[0].label = buffer;

      for (i = 1; i <= psd[0].M; ++i)
        psd[0].Special[i] = Pxx1[i] / Pxx2[i];
      }


    printf("variance1=%f, variance2=%f\n", variance1, variance2);
    delete [] Pxx1;
    delete [] Pxx2;
    }

  if (psd[0].display == PHASE)
    {
    psd[0].Special = new double[psd[0].M+1];

    snprintf(buffer, BUFFSIZE, "Phase of %s x %s",
            dataSet[0].varInfo->name.c_str(), dataSet[1].varInfo->name.c_str());
    specPlot.Yaxis[0].label = buffer;

    for (i = 1; i <= psd[0].M; ++i)
      psd[0].Special[i] = atan2(psd[0].Qxx[i], psd[0].Pxx[i]) * 180.0 / M_PI;
    }
printf("%s\n", buffer);
  delete [] detrendedData[0];
  delete [] detrendedData[1];

}	/* END COSPECTRUM */

/* END COSPEC.C */
