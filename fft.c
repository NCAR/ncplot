/*
-------------------------------------------------------------------------
OBJECT NAME:	fft.c

FULL NAME:	Fast Fourier Transform

ENTRY POINTS:	fft()

STATIC FNS:	ShuffleIndex()
		Shuffle2Arr()

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	spctrm.c, cospec.c

COPYRIGHT:	Public Domain
-------------------------------------------------------------------------
*/

#include <math.h>


static void Shuffle2Arr(double *a, double *b, int bitLength);
static unsigned int ShuffleIndex(unsigned int i, unsigned int wordLength);

/* -------------------------------------------------------------------- */
static double CosArray[] =
{
-1.000000000000000,  0.000000000000000,  0.707106781186548,  0.923879532511287,
 0.980785280403230,  0.995184726672197,  0.998795456205172,  0.999698818696204,
 0.999924701839145,  0.999981175282601,  0.999995293809576,  0.999998823451702,
 0.999999705862882,  0.999999926465718, 

-1.000000000000000,  0.000000000000000,  0.707106781186548,  0.923879532511287,
 0.980785280403230,  0.995184726672197,  0.998795456205172,  0.999698818696204,
 0.999924701839145,  0.999981175282601,  0.999995293809576,  0.999998823451702,
 0.999999705862882,  0.999999926465718
};
 
static double SinArray[] =
{
-0.000000000000000, -1.000000000000000, -0.707106781186547, -0.382683432365090,
-0.195090322016128, -0.098017140329561, -0.049067674327418, -0.024541228522912,
-0.012271538285720, -0.006135884649154, -0.003067956762966, -0.001533980186285,
-0.000766990318743, -0.000383495187571, 

 0.000000000000000,  1.000000000000000,  0.707106781186547,  0.382683432365090,
 0.195090322016128,  0.098017140329561,  0.049067674327418,  0.024541228522912,
 0.012271538285720,  0.006135884649154,  0.003067956762966,  0.001533980186285,
 0.000766990318743,  0.000383495187571
};


/* -------------------------------------------------------------------- */
void fft(double *real, double *imaginary, int power, int direction)
{
  int		pwrHelp, N, Section, angleCounter, flyDistance, flyCount,
                index1, index2;
  double	tempr, tempi, c, s, scale, temp, Qr, Qi;


  Shuffle2Arr(real, imaginary, power);

  pwrHelp = power;
  N = 1;

  do
    {
    N <<= 1;
    }
  while (--pwrHelp > 0);

  /*				Fwd: Inverse	*/
  angleCounter = direction > 0 ? 0 : 14;
  Section = 1;

  while (Section < N)
    {
    flyDistance = Section << 1;

    c = CosArray[angleCounter];
    s = SinArray[angleCounter];

    Qr = 1; Qi = 0;

    for (flyCount = 0; flyCount < Section; ++flyCount)
      {
      index1 = flyCount;

      do
        {
        index2 = index1 + Section;

        tempr = Qr * real[index2] - Qi * imaginary[index2];
        tempi = Qr * imaginary[index2] + Qi * real[index2];
/*
        tempr = 1.0 * Qr * real[index2] - 1.0 * Qi * imaginary[index2];
        tempi = 1.0 * Qr * imaginary[index2] + 1.0 * Qi * real[index2];
*/
        real[index2] = real[index1] - tempr;
        real[index1] += tempr;

        imaginary[index2] = imaginary[index1] - tempi;
        imaginary[index1] += tempi;

        index1 += flyDistance;
        }
      while (index1 < N);

      temp = Qr;
      Qr = Qr * c - Qi * s;
      Qi = Qi * c + temp * s;
      }

    Section <<= 1;
    ++angleCounter;
    }

  if (direction < 0)
    {
    scale = 1.0 / N;

    for (index1 = 0; index1 < N; ++index1)
      {
      real[index1]	*= scale;
      imaginary[index1]	*= scale;
      }
    }

}	/* END FFT */

/* -------------------------------------------------------------------- */
static void Shuffle2Arr(double *a, double *b, int bitLength)
{
  unsigned int	N, indexOld, indexNew;
  double	temp;
  int		bitLengthTemp;

  bitLengthTemp = bitLength;
  N = 1;

  do
    {
    N <<= 1;
    }
  while (--bitLength > 0);


  for (indexOld = 0; indexOld < N; ++indexOld)
    {
    indexNew = ShuffleIndex(indexOld, bitLengthTemp);

    if (indexNew > indexOld)
      {
      temp = a[indexOld];
      a[indexOld] = a[indexNew];
      a[indexNew] = temp;

      temp = b[indexOld];
      b[indexOld] = b[indexNew];
      b[indexNew] = temp;
      }
    }

}	/* END SHUFFLE2ARR */

/* -------------------------------------------------------------------- */
static unsigned int ShuffleIndex(unsigned int i, unsigned int wordLength)
{
  unsigned int	bitNr, newIndex = 0;

  for (bitNr = 0; bitNr < wordLength; ++bitNr)
    {
    newIndex <<= 1;

    if ((i & 0x0001) != 0)
      ++newIndex;

    i >>= 1;
    }

  return(newIndex);

}	/* END SHUFFLEINDEX */

/* END FFT.C */
