/*
-------------------------------------------------------------------------
OBJECT NAME:	window.c

FULL NAME:	Window Functions

ENTRY POINTS:	Bartlett()
		Blackman()
		Hamming()
		Hanning()
		Parzen()
		Square()
		Welch()

STATIC FNS:	none

DESCRIPTION:	Data windowing functions.

INPUT:		index, window_size

OUTPUT:		window value

REFERENCES:	none

REFERENCED BY:	spctrm.c

COPYRIGHT:	Public Domain
-------------------------------------------------------------------------
*/

#include <math.h>

#include "define.h"
#include "spec.h"


/* -------------------------------------------------------------------- */
double Bartlett(int j, int N)
{
  return(0.42 -  0.5 * cos(2.0 * M_PI * j / (N - 1)) +
         0.08 * cos(4.0 * M_PI * j / (N - 1)));

}	/* END BARTLETT */

/* -------------------------------------------------------------------- */
double Blackman(int j, int N)
{
  return(0.42 -  0.5 * cos(2.0 * M_PI * j / (N - 1)) +
         0.08 * cos(4.0 * M_PI * j / (N - 1)));

}	/* END BLACKMAN */

/* -------------------------------------------------------------------- */
double Hamming(int j, int N)
{
  return(0.54 - 0.46 * cos(2.0 * M_PI * j / (N - 1)));

}	/* END HAMMING */

/* -------------------------------------------------------------------- */
double Hanning(int j, int N)
{
  return(0.5 * (1.0 - cos(2.0 * M_PI * j / (N - 1))));

}	/* END HANNING */

/* -------------------------------------------------------------------- */
double Parzen(int j, int N)
{
  return(1.0 - fabs((j - 0.5 * (N - 1)) / (0.5 * (N + 1))));

}	/* END PARZEN */

/* -------------------------------------------------------------------- */
double Square(int j, int N)
{
  return(1.0);

}	/* END SQUARE */

/* -------------------------------------------------------------------- */
double Triangle(int j, int N)
{
  return(1.0 - fabs(1.0 - ((2.0 * j) / (N - 1)) ));

}	/* END TRIANGLE */

/* -------------------------------------------------------------------- */
double Welch(int j, int N)
{
  double	rc;

  rc = (j - 0.5 * (N - 1)) / (0.5 * (N + 1));

  return(1.0 - rc * rc);

}	/* END WELCH */

/* END WINDOW.C */
