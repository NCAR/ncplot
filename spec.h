/*
-------------------------------------------------------------------------
OBJECT NAME:	spec.h

FULL NAME:	Spectral Header File

DESCRIPTION:	Prototypes for Spectral Density and FFT functions.

COPYRIGHT:	University Corporation for Atmospheric Research, 1995-7
-------------------------------------------------------------------------
*/


#ifndef SPEC_H
#define SPEC_H

#include <Xm/Xm.h>

static const size_t MAX_PSD = 4;

typedef struct  /* Spectral information.        */
	{
	int	display;	/* Plot type, SPEC, CO-SPEC	*/

	size_t	K;		/* Number of Segments.		*/
	size_t	M;		/* 1/2 segment length.		*/
	float	frequency;	/* Sample Rate of data.		*/

	void	(*detrendFn)(DATASET_INFO *, float *);
	double	(*windowFn)(int, int);

	double	*Pxx;		/* Output array		*/
	double	*Qxx;		/* Quadrature (CSD)	*/
	double	*Special;	/* Coherence, Phase or Ratio (CSD)	*/

	double	freqPerBin;
	double	totalVariance;
	double	bandVariance;

	int     ELIAcnt;
	double  *ELIAx, *ELIAy;
	} PSD_INFO;


extern Widget	specCanvas;

extern PSD_INFO	psd[];
extern DATASET_INFO	tas;

void	ComputeBandLimitedVariance(Widget w, XtPointer client, XtPointer call);

double Spectrum(float data[], double Pxx[], size_t K, size_t M,
		double (*window)(int, int), size_t nPoints);

double CoSpectrum(float data1[], float data2[], double Pxx[], double Qxx[],
	size_t K, size_t M, double (*window)(int, int), size_t nPoints);

void fft(double *real, double *imaginary, int power, int direction);

void DetrendLinear(DATASET_INFO *in, float out[]);
void DetrendMean(DATASET_INFO *in, float out[]);
void DetrendNone(DATASET_INFO *in, float out[]);

void LinearLeastSquare(DATASET_INFO *set, float inp[], float out[]);

void CleanAndCopyData(DATASET_INFO *set, float out[]);
void DiffPreFilter(DATASET_INFO *set, float out[]);

void	ComputeSpectrum(), ComputeCoSpectrum(), AutoScaleSpectralWindow();

double Bartlett(int j, int N);
double Blackman(int j, int N);
double Hamming(int j, int N);
double Hanning(int j, int N);
double Parzen(int j, int N);
double Square(int j, int N);
double Triangle(int j, int N);
double Welch(int j, int N);

int  eliaPoints();
bool multiplyByFreq(), plotWaveNumber(), plotWaveLength(), equalLogInterval(),
     plotFiveThirds(), multiplyByFreq53();

void ToggleSpecGrid(Widget w, XtPointer client, XtPointer call);
void ToggleWaveNumberScale(Widget w, XtPointer client, XtPointer call);
void ToggleWaveLengthScale(Widget w, XtPointer client, XtPointer call);
void ToggleMultByFreq(Widget w, XtPointer client, XtPointer call);


int timeShift();
double startFreq(), endFreq();

#endif
