/*
-------------------------------------------------------------------------
OBJECT NAME:	elia.c

FULL NAME:	Equal Log Interval Averaging.

ENTRY POINTS:	ComputeELIA()

STATIC FNS:	none

DESCRIPTION:	Part of Spectral.

NOTES:		

COPYRIGHT:	University Corporation for Atmospheric Research, 1998-2005
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"


/* -------------------------------------------------------------------- */
void ComputeELIA()
{
  size_t	i, cnt = 0, prevCnt = 0, set, nSets = 1;
  double	binSize, currentFreq, endFreq, sum, *dataP = 0;

  switch (psd[0].display)
    {
    case SPECTRA:
      nSets = std::min(NumberDataSets, MAX_PSD);
    case COSPECTRA:
      dataP = psd[0].Pxx;
      break;
    case QUADRATURE:
      dataP = psd[0].Qxx;
      break;
    case COHERENCE:
    case PHASE:
    case RATIO:
      dataP = psd[0].Special;
      break;
    }

  if (psd[0].display == SPECTRA)
    nSets = std::min(NumberDataSets, MAX_PSD);

  for (set = 0; set < nSets; ++set)	// Looping here is for SPECTRA only
    {
    if (psd[0].display == SPECTRA)
      dataP = psd[set].Pxx;

    if (psd[set].ELIAx) delete [] psd[set].ELIAx;
    if (psd[set].ELIAy) delete [] psd[set].ELIAy;

    psd[set].ELIAx = new double[eliaPoints()+1];
    psd[set].ELIAy = new double[eliaPoints()+1];

    binSize =	(log10(psd[0].M*psd[set].freqPerBin)-log10(psd[set].freqPerBin))
		/ eliaPoints();

    currentFreq = psd[set].freqPerBin;
    endFreq = log10(currentFreq) + binSize;
    psd[set].ELIAcnt = 1;

    for (i = 1; i <= psd[0].M; )
      {
      prevCnt = cnt;

      for (cnt = 0, sum = 0.0; i <= psd[0].M && log10(currentFreq) <= endFreq; ++cnt)
        {
        sum += dataP[i++];
        currentFreq += psd[set].freqPerBin;
        }

      endFreq += binSize;

      if (cnt > 0)
        {
        if (cnt == 1)
          psd[set].ELIAx[psd[set].ELIAcnt] = currentFreq - psd[set].freqPerBin;
        else
          psd[set].ELIAx[psd[set].ELIAcnt] = currentFreq - psd[set].freqPerBin * cnt / 2.0;

        psd[set].ELIAy[psd[set].ELIAcnt++] = sum / cnt;
        }
      }

    /* Make sure the last bin has sufficient points in it, otherwise ditch it.
     */
    if (cnt < prevCnt / 3)
      --psd[set].ELIAcnt;
    }

  printf("ELIA created %d points of a requested %d.\n", psd[0].ELIAcnt, eliaPoints());

}	/* END COMPUTEELIA */

/* END ELIA.C */
