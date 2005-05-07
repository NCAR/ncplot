/*
-------------------------------------------------------------------------
OBJECT NAME:	spec.c

FULL NAME:	Spectrum

ENTRY POINTS:	SpecWinUp()
		SpecWinDown()
		ComputeSpectrum()
		SetSegLen()
		SetWindow()
		SetDetrend()

STATIC FNS:	CreateSpectrumWindow()
		AutoScaleSpectralWindow()
		setDefaults()

DESCRIPTION:	

REFERENCES:	spctrm.c

REFERENCED BY:	Callbacks, control.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-05
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

void SetSegLen(Widget w, XtPointer client, XtPointer call);
void SetWindow(Widget w, XtPointer client, XtPointer call);
void SetDetrend(Widget w, XtPointer client, XtPointer call);

struct ccb_info
        {
        char    *name;
	void	(*callBack)(Widget, XtPointer, XtPointer);
        void    *client;
        };
 
static struct ccb_info  segLenInfo[] = {
        { "   128",   SetSegLen,	(void *)64, },
        { "   256",   SetSegLen,	(void *)128, },
        { "   512",   SetSegLen,	(void *)256, },
        { " 1,024",   SetSegLen,	(void *)512, },
        { " 2,048",   SetSegLen,	(void *)1024, },
        { " 4,096",   SetSegLen,	(void *)2048, },
        { " 8,192",   SetSegLen,	(void *)4096, },
        { "16,384",   SetSegLen,	(void *)8192, },
        { "32,768",   SetSegLen,	(void *)16384, },
        { NULL,       NULL,	NULL }
        };
 
static struct ccb_info  windowInfo[] = {
        { "Bartlett", SetWindow,	(void *)Bartlett, },
        { "Blackman", SetWindow,	(void *)Blackman, },
        { "Hamming",  SetWindow,	(void *)Hamming, },
        { "Hanning",  SetWindow,	(void *)Hanning, },
        { "Parzen",   SetWindow,	(void *)Parzen, },
        { "Square",   SetWindow,	(void *)Square, },
        { "Triangle", SetWindow,	(void *)Triangle, },
        { "Welch",    SetWindow,	(void *)Welch, },
        { NULL,       NULL,	NULL }
        };
 
static struct ccb_info  detrendInfo[] = {
        { "None",     SetDetrend,	(void *)DetrendNone, },
        { "Difference",   SetDetrend,	(void *)DiffPreFilter, },
        { "Mean",     SetDetrend,	(void *)DetrendMean, },
        { "Linear",   SetDetrend,	(void *)DetrendLinear, },
        { NULL,       NULL,	NULL }
        };
 

extern Widget	AppShell, MainWindow, ControlWindow, SpecShell, SpectrumWindow;

static Widget   slOpMenu, winOpMenu, dtOpMenu, sFreq, eFreq,
		tShift, eliaText, pmOptButt[10], typeButts[6];

static void	CreateSpectrumWindow(), setDefaults();

static Widget	CreateDropDownMenu(Widget parent, char menu_name[],
	struct ccb_info menu_info[]);

/* -------------------------------------------------------------------- */
void SpecWinDown(Widget w, XtPointer client, XtPointer call)
{
  specPlot.windowOpen = false;

  if (SpectrumWindow)
    {
    XtUnmanageChild(SpectrumWindow);
    XtPopdown(XtParent(SpectrumWindow));
    }
 
}   /* END SPECWINDOWN */
 
/* -------------------------------------------------------------------- */
void SpecWinUp(Widget w, XtPointer client, XtPointer call)
{
  int	i, nSets;
  bool	saveState = Freeze;
  XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *)call;

  static bool firstTime = true;

  /* If this is 'unset' from one of the ToggleButtons, then bail.
   */
  if (cb && cb->reason == XmCR_VALUE_CHANGED && cb->set == false)
    return;

  if (NumberDataSets < 1)
    return;

  if ((int)client > 1 && NumberDataSets < 2)
    {
    HandleError("Co-PSD requires two data sets.", Interactive, IRET);
    return;
    }

  WaitCursor(MainWindow);
  WaitCursor(ControlWindow);

  if (client > 0)
    psd[0].display = (int)client;

  if (firstTime)
    {
    CreateSpectrumWindow();
    initPlotGC(&specPlot);
    }
  else
    WaitCursor(SpectrumWindow);

  for (i = 0; i < 6; ++i)
    if (i == psd[0].display - 1)
      XmToggleButtonSetState(typeButts[i], true, false);
    else
      XmToggleButtonSetState(typeButts[i], false, false);

  Freeze = true;
  psd[0].frequency = dataSet[0].nPoints / NumberSeconds;
  psd[0].freqPerBin = (double)psd[0].frequency / (psd[0].M << 1);

  switch (psd[0].display)
    {
    case SPECTRA:
      XtSetSensitive(pmOptButt[1], true);
      XtSetSensitive(pmOptButt[2], true);
      XtSetSensitive(pmOptButt[3], true);

      nSets = std::min(NumberDataSets, MAX_PSD);

      for (i = 1; i < nSets; ++i)
        {
        psd[i].frequency = dataSet[i].nPoints / NumberSeconds;
        psd[i].freqPerBin = (double)psd[i].frequency / (psd[i].M << 1);
        }

      ComputeSpectrum();
      break;

    case COSPECTRA:
    case QUADRATURE:
    case RATIO:
      XtSetSensitive(pmOptButt[1], false);
      XtSetSensitive(pmOptButt[2], true);
      XtSetSensitive(pmOptButt[3], true);
      ComputeCoSpectrum();
      break;

    case COHERENCE:
    case PHASE:
      XtSetSensitive(pmOptButt[1], false);
      XtSetSensitive(pmOptButt[2], false);
      XtSetSensitive(pmOptButt[3], false);
      ComputeCoSpectrum();
      break;

    default:
      fprintf(stderr, "SpecWinUp: Invalid spectral display.\n");
      return;
    }

  Freeze = saveState;

  specPlot.windowOpen = true;
  XtManageChild(SpectrumWindow);
  XtPopup(XtParent(SpectrumWindow), XtGrabNone);

  if (firstTime)
    {
    WaitCursor(SpectrumWindow);
    ResizeSpecWindow(NULL, NULL, NULL);
    setDefaults();

    XtAddCallback(specPlot.canvas, XmNexposeCallback,
				(XtCallbackProc)PlotSpectrum, NULL);
    XtAddCallback(specPlot.canvas, XmNresizeCallback,
				(XtCallbackProc)ResizeSpecWindow, NULL);
    XtAddCallback(specPlot.canvas, XmNresizeCallback,
				(XtCallbackProc)PlotSpectrum, NULL);

    firstTime = false;
    }

  PlotSpectrum(NULL, NULL, NULL);

  if (AsciiWinOpen)
    SetASCIIdata(NULL, NULL, NULL);

  PointerCursor(MainWindow);
  PointerCursor(ControlWindow);
  PointerCursor(SpectrumWindow);

}   /* END SPECWINUP */

/* -------------------------------------------------------------------- */
void ComputeSpectrum()
{
  float *detrendedData = 0;
  
  size_t nSets = std::min(NumberDataSets, MAX_PSD);

  for (size_t set = 0; set < nSets; ++set)
    {
    delete [] psd[set].Pxx;
    delete [] psd[set].Qxx;

    psd[set].Pxx = psd[set].Qxx = NULL;


    /* 50% overlapping.
     */
    if ((psd[0].K = dataSet[set].nPoints / psd[0].M) < 1)
      psd[0].K = 1;

    psd[0].K += 2;
    size_t KxM = (psd[0].K + 1) * psd[0].M;

    detrendedData = new float[KxM];
    psd[set].Pxx = new double[psd[0].M+1];


    /* This computation of i, is to center the data in the zero padding.
     */
    size_t pos = (KxM - dataSet[set].nPoints) / 2;
    memset((char *)detrendedData, 0, sizeof(float) * KxM);
    CleanAndCopyData(&dataSet[set], &detrendedData[pos]);
    (*psd[0].detrendFn)(&dataSet[set], &detrendedData[pos]);

    psd[set].totalVariance =
      Spectrum(detrendedData, psd[set].Pxx, psd[0].K, psd[0].M,
			psd[0].windowFn, dataSet[set].nPoints);

    ComputeBandLimitedVariance(NULL, NULL, NULL);


    if (multiplyByFreq())
      {
      for (size_t i = 1; i <= psd[0].M; ++i)
        psd[set].Pxx[i] *= i;

      sprintf(buffer, "f x PSD of %s (%s^2)",
  	dataSet[0].varInfo->name.c_str(), dataSet[0].stats.units.c_str());
      specPlot.Yaxis[0].label = buffer;
      }
    else
    if (multiplyByFreq53())
      {
      for (size_t i = 1; i <= psd[0].M; ++i)
        psd[set].Pxx[i] *= pow((double)i, 5.0/3.0);

      sprintf(buffer, "f^(5/3) x PSD of %s (%s^2)",
  	dataSet[0].varInfo->name.c_str(), dataSet[0].stats.units.c_str());
      specPlot.Yaxis[0].label = buffer;
      }
    else
      {
      double cf = (double)(psd[0].M << 1) / psd[set].frequency;

      for (size_t i = 1; i <= psd[0].M; ++i)
        psd[set].Pxx[i] *= cf;

      sprintf(buffer, "PSD of %s (%s^2 / Hz)",
  	dataSet[0].varInfo->name.c_str(), dataSet[0].stats.units.c_str());
      specPlot.Yaxis[0].label = buffer;
      }
    }

  delete [] detrendedData;

}	/* END COMPUTESPECTRUM */

/* -------------------------------------------------------------------- */
void AutoScaleSpectralWindow()
{
  struct axisInfo *Yaxis = &specPlot.Yaxis[0];
 
  if (specPlot.autoTics)
    specPlot.Xaxis.nMajorTics = (int)(ceil(log10(specPlot.Xaxis.max)) - floor(log10(specPlot.Xaxis.min)));

  if (specPlot.autoScale)
  {
    specPlot.Xaxis.min = 0.001;
    specPlot.Xaxis.max = 1000;
  }

  switch (psd[0].display)
    {
    case COSPECTRA:
    case QUADRATURE:
      if (specPlot.autoTics)
        {
        Yaxis->nMajorTics = 8;
        Yaxis->nMinorTics = 4;
        }
      AutoScaleSpec();
      return;
 
    case COHERENCE:
      if (specPlot.autoScale)
        {
        Yaxis->min = 0.0;
        Yaxis->max = 1.0;
        }

      if (specPlot.autoTics)
        {
        Yaxis->nMajorTics = 10;
        Yaxis->nMinorTics = 4;
        }
      return;
 
    case RATIO:
      if (specPlot.autoScale)
        {
        Yaxis->min = 0.0;
        Yaxis->max = 10.0;
        }

      if (specPlot.autoTics)
        {
        Yaxis->nMajorTics = 10;
        Yaxis->nMinorTics = 3;
        }
      return;
 
    case PHASE:
      if (specPlot.autoScale)
        {
        Yaxis->min = -180.0;
        Yaxis->max = 180.0;
        }

      if (specPlot.autoTics)
        {
        Yaxis->nMajorTics = 8;
        Yaxis->nMinorTics = 4;
        }
      return;
    }

  Yaxis->smallestValue = FLT_MAX;
  Yaxis->biggestValue = -FLT_MAX;


  size_t nSets = std::min(NumberDataSets, MAX_PSD);

  for (size_t set = 0; set < nSets; ++set)
    {
    for (size_t i = 1; i <= psd[set].M; ++i)
      {
      if (psd[set].Pxx[i] < 0.0)
        continue;
 
      Yaxis->smallestValue = std::min(Yaxis->smallestValue, psd[set].Pxx[i]);
      Yaxis->biggestValue = std::max(Yaxis->biggestValue, psd[set].Pxx[i]);
      }
    }
 
  if (specPlot.autoTics)
    {
    Yaxis->nMinorTics = 10;
    Yaxis->nMajorTics = (int)(ceil(log10(Yaxis->biggestValue)) - floor(log10(Yaxis->smallestValue)));
    }

  AutoScaleSpec();
 
}	/* END AUTOSCALESPECTRALWINDOW */

/* -------------------------------------------------------------------- */
void SetSegLen(Widget w, XtPointer client, XtPointer call)
{
  for (size_t i = 0; i < MAX_PSD; ++i)
    psd[i].M = (int)client;
}

/* -------------------------------------------------------------------- */
void SetWindow(Widget w, XtPointer client, XtPointer call)
{
  for (size_t i = 0; i < MAX_PSD; ++i)
    psd[i].windowFn = (double(*)(int, int))client;
}

/* -------------------------------------------------------------------- */
void SetDetrend(Widget w, XtPointer client, XtPointer call)
{
  for (size_t i = 0; i < MAX_PSD; ++i)
    psd[i].detrendFn = (void(*)(DATASET_INFO *, float *))client;
}

/* -------------------------------------------------------------------- */
double startFreq()
{
  double	n;
  char		*p;

  p = XmTextFieldGetString(sFreq);
  n = atof(p);
  free(p);
 
  return(n);
}

/* -------------------------------------------------------------------- */
double endFreq()
{
  double	n;
  char		*p;

  p = XmTextFieldGetString(eFreq);
  n = atof(p);
  free(p);
 
  return(n);
}

/* -------------------------------------------------------------------- */
int timeShift()
{
  int	n;
  char *p;

  p = XmTextFieldGetString(tShift);
  n = atoi(p);
  free(p);
 
  return(n);
}
 
/* -------------------------------------------------------------------- */
void ToggleSpecGrid(Widget w, XtPointer client, XtPointer call)
{
  specPlot.grid = ((XmToggleButtonCallbackStruct *)call)->set;
}

/* -------------------------------------------------------------------- */
void ToggleWaveNumberScale(Widget w, XtPointer client, XtPointer call)
{
  static float	min, max;

  if (((XmToggleButtonCallbackStruct *)call)->set)
    {
    min = specPlot.Xaxis.min;
    max = specPlot.Xaxis.max;

    if (!plotWaveLength()) {
      if (tas.data) {
        delete [] tas.data;
        tas.data = NULL;
        }
 
      if (LoadVariable(&tas, tasVarName) == ERR)
        {
        sprintf(buffer, "Can't locate True Airspeed variable %s.", tasVarName.c_str());
        ShowError(buffer);
        }
      }

    specPlot.Xaxis.min = (min * 2.0*M_PI / tas.stats.mean);
    specPlot.Xaxis.max = (max * 2.0*M_PI / tas.stats.mean)+1;
    specPlot.Xaxis.label = "Wave Number (rad/m)";
    }
  else
    {
    specPlot.Xaxis.min = min;
    specPlot.Xaxis.max = max;
    specPlot.Xaxis.label = "Frequency (Hz)";

    if (!plotWaveLength()) {
      delete [] tas.data;
      tas.data = NULL;
      }
    }

  specPlot.Xaxis.nMajorTics = (int)(specPlot.Xaxis.max - specPlot.Xaxis.min);
  SetSpecDefaults();

}	/* END TOGGLEWAVENUMBERSCALE */
 
/* -------------------------------------------------------------------- */
void ToggleWaveLengthScale(Widget w, XtPointer client, XtPointer call)
{
  if (((XmToggleButtonCallbackStruct *)call)->set)
    {
    if (!plotWaveNumber()) {
      if (tas.data) {
        delete [] tas.data;
        tas.data = NULL;
        }
 
      if (LoadVariable(&tas, tasVarName) == ERR)
        {
        XmToggleButtonSetState(pmOptButt[5], false, false);
        sprintf(buffer, "Can't locate True Airspeed variable %s.", tasVarName.c_str());
        ShowError(buffer);
        return;
        }
      }
    }
  else
    {
    if (!plotWaveNumber()) {
      delete [] tas.data;
      tas.data = NULL;
      }
    }

}	/* END TOGGLEWAVELENGTHSCALE */

/* -------------------------------------------------------------------- */
void ToggleMultByFreq(Widget w, XtPointer client, XtPointer call)
{
  /* Only mult by freq or mult by freq^5/3 can be selected.  Grey out the other
   */
  if (multiplyByFreq())
    XtSetSensitive(pmOptButt[3], false);
  else
    XtSetSensitive(pmOptButt[3], true);

  if (multiplyByFreq53())
    XtSetSensitive(pmOptButt[2], false);
  else
    XtSetSensitive(pmOptButt[2], true);


  if (psd[0].display == SPECTRA)
    ComputeSpectrum();
  else
    ComputeCoSpectrum();

}	/* END TOGGLEMULTBYFREQ */

/* -------------------------------------------------------------------- */
bool plotFiveThirds()
{
  return(XmToggleButtonGetState(pmOptButt[1]));
}
 
/* -------------------------------------------------------------------- */
bool multiplyByFreq()
{
  return(XmToggleButtonGetState(pmOptButt[2]));
}
 
/* -------------------------------------------------------------------- */
bool multiplyByFreq53()	/* freq to the 5/3 power */
{
  return(XmToggleButtonGetState(pmOptButt[3]));
}
 
/* -------------------------------------------------------------------- */
bool plotWaveNumber()
{
  if (SpectrumWindow == NULL)
    return(false);

  return(XmToggleButtonGetState(pmOptButt[4]));
}
 
/* -------------------------------------------------------------------- */
bool plotWaveLength()
{
  if (SpectrumWindow == NULL)
    return(false);

  return(XmToggleButtonGetState(pmOptButt[5]));
}
 
/* -------------------------------------------------------------------- */
bool equalLogInterval()
{
  return(XmToggleButtonGetState(pmOptButt[6]));
}

/* -------------------------------------------------------------------- */
int eliaPoints()
{
  char *p;
  int	x;

  p = XmTextFieldGetString(eliaText);
  x = atoi(p);

  return(x);

}

/* -------------------------------------------------------------------- */
static void CreateSpectrumWindow()
{
  Widget	optRC, frame, plRC, rc, RC[8], b[8], pb;
  Cardinal	n;
  Arg		args[8];


  if (SpectrumWindow)
    return;

  n = 0;
  SpecShell = XtCreatePopupShell("specShell", topLevelShellWidgetClass,
              AppShell, args, n);

  n = 0;
  SpectrumWindow = XmCreateForm(SpecShell, "specForm", args, n);


  /* RC for Options.
   */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  optRC = XmCreateRowColumn(SpectrumWindow, "specOptForm", args, n);
  XtManageChild(optRC);


  /* Command buttons.
   */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNrightWidget, optRC); n++;
  frame = XmCreateFrame(SpectrumWindow, "buttonFrame", args, n);
  XtManageChild(frame);

  n = 0;
  rc = XmCreateRowColumn(frame, "buttonRC", args, n);
  XtManageChild(rc);

  n = 0;
  b[0] = XmCreatePushButton(rc, "dismissButton", args, n);
  b[1] = XmCreatePushButton(rc, "printButton", args, n);
  b[2] = XmCreatePushButton(rc, "parmsButton", args, n);
  b[3] = XmCreatePushButton(rc, "savepngButton", args, n);
  XtManageChildren(b, 4);
  XtAddCallback(b[0], XmNactivateCallback, SpecWinDown, NULL);
  XtAddCallback(b[1], XmNactivateCallback, specPostScript, NULL);
  XtAddCallback(b[2], XmNactivateCallback, EditSpecParms, NULL);
#ifdef PNG
  XtAddCallback(b[3], XmNactivateCallback, SavePNGspec, NULL);
#endif



  /* Create Graphics Canvas
  */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, frame); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNrightWidget, optRC); n++;
  specPlot.canvas = XmCreateDrawingArea(SpectrumWindow, "specCanvas", args,n);
  XtManageChild(specPlot.canvas);



  n = 0;
  frame = XmCreateFrame(optRC, "specDefsFrame", args, 0);
  XtManageChild(frame);

  n = 0;
  RC[0] = XmCreateRadioBox(frame, "specTypeRB", args, n);

  n = 0;
  typeButts[0] = XmCreateToggleButton(RC[0], "Spectrum", args, n);
  typeButts[1] = XmCreateToggleButton(RC[0], "Co-Spectrum", args, n);
  typeButts[2] = XmCreateToggleButton(RC[0], "Quadrature", args, n);
  typeButts[3] = XmCreateToggleButton(RC[0], "Coherence", args, n);
  typeButts[4] = XmCreateToggleButton(RC[0], "Phase", args, n);
  typeButts[5] = XmCreateToggleButton(RC[0], "Spectral ratio", args, n);
  XtManageChildren(typeButts, 6);

  XtAddCallback(typeButts[0],XmNvalueChangedCallback,SpecWinUp,(XtPointer)SPECTRA);
  XtAddCallback(typeButts[1],XmNvalueChangedCallback,SpecWinUp,(XtPointer)COSPECTRA);
  XtAddCallback(typeButts[2],XmNvalueChangedCallback,SpecWinUp,(XtPointer)QUADRATURE);
  XtAddCallback(typeButts[3],XmNvalueChangedCallback,SpecWinUp,(XtPointer)COHERENCE);
  XtAddCallback(typeButts[4],XmNvalueChangedCallback,SpecWinUp,(XtPointer)PHASE);
  XtAddCallback(typeButts[5],XmNvalueChangedCallback,SpecWinUp,(XtPointer)RATIO);


  /* Optional stuff.  PreFilter, SegLen, Window, Detrend menus.
   */
  n = 0;
  frame = XmCreateFrame(optRC, "specDefsFrame", args, 0);
  XtManageChild(frame);

  n = 0;
  RC[1] = XmCreateRowColumn(frame, "specDefsRC", args, n);

  dtOpMenu = CreateDropDownMenu(RC[1], "dtOpMenu", detrendInfo);
  slOpMenu = CreateDropDownMenu(RC[1], "slOpMenu", segLenInfo);
  winOpMenu = CreateDropDownMenu(RC[1], "winOpMenu", windowInfo);

  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
  pb = XmCreatePushButton(RC[1], "Recompute", args, n);
  XtManageChild(pb);
  XtAddCallback(pb, XmNactivateCallback, SpecWinUp, NULL);



  /* Variance widgets.
   */
  n = 0;
  frame = XmCreateFrame(optRC, "specDefsFrame", args, 0);
  XtManageChild(frame);

  XtManageChild(XmCreateLabel(frame, "fluxLabel", args, n));

  n = 0;
  RC[2] = XmCreateRowColumn(frame, "plRCv", args, n);

  plRC = XmCreateRowColumn(RC[2], "plRC", args, n);
  XtManageChild(XmCreateLabel(plRC, "Start freq", args, n));
  sFreq = XmCreateTextField(plRC, "fluxFreq", args, n);
  XtManageChild(XmCreateLabel(plRC, "Hz", args, n));
  XtManageChild(plRC);
  XtManageChild(sFreq);
  XtAddCallback(sFreq, XmNlosingFocusCallback, ValidateFloat, (XtPointer)"%g");
  XtAddCallback(sFreq, XmNlosingFocusCallback, ComputeBandLimitedVariance,NULL);
  XtAddCallback(sFreq, XmNlosingFocusCallback, (XtCallbackProc)PlotSpectrum, NULL);

  plRC = XmCreateRowColumn(RC[2], "plRC", args, n);
  XtManageChild(XmCreateLabel(plRC, "End freq", args, n));
  eFreq = XmCreateTextField(plRC, "fluxFreq", args, n);
  XtManageChild(XmCreateLabel(plRC, "Hz", args, n));
  XtManageChild(plRC);
  XtManageChild(eFreq);
  XtAddCallback(eFreq, XmNlosingFocusCallback, ValidateFloat, (XtPointer)"%g");
  XtAddCallback(eFreq, XmNlosingFocusCallback, ComputeBandLimitedVariance,NULL);
  XtAddCallback(eFreq, XmNlosingFocusCallback, (XtCallbackProc)PlotSpectrum, NULL);

  XmTextFieldSetString(sFreq, "0.0");
  XmTextFieldSetString(eFreq, "5000.0");


  /* Plot Methods.
   */
  n = 0;
  frame = XmCreateFrame(optRC, "specDefsFrame", args, 0);
  XtManageChild(frame);

  XtManageChild(XmCreateLabel(frame, "plotMethodLabel", args, n));

  n = 0;
  RC[3] = XmCreateRowColumn(frame, "plRCv", args, n);

  n = 0;
  pmOptButt[0] = XmCreateToggleButton(RC[3], "Grid", args,n);
  pmOptButt[1] = XmCreateToggleButton(RC[3], "-5/3 (-2/3 x f) slope line", args,n);
  pmOptButt[2] = XmCreateToggleButton(RC[3], "Multiply output by frequency", args,n);
  pmOptButt[3] = XmCreateToggleButton(RC[3], "Multiply output by freq^(5/3)", args,n);
  pmOptButt[4] = XmCreateToggleButton(RC[3], "Wave number scale", args,n);
  pmOptButt[5] = XmCreateToggleButton(RC[3], "Wave length scale", args,n);
  XtManageChildren(pmOptButt, 6);

  XtAddCallback(pmOptButt[0], XmNvalueChangedCallback, ToggleSpecGrid, NULL);
  XtAddCallback(pmOptButt[0], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);
  XtAddCallback(pmOptButt[1], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);
  XtAddCallback(pmOptButt[2], XmNvalueChangedCallback, ToggleMultByFreq, NULL);
  XtAddCallback(pmOptButt[2], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);
  XtAddCallback(pmOptButt[3], XmNvalueChangedCallback, ToggleMultByFreq, NULL);
  XtAddCallback(pmOptButt[3], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);
  XtAddCallback(pmOptButt[4], XmNvalueChangedCallback, ToggleWaveNumberScale, NULL);
  XtAddCallback(pmOptButt[4], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);
  XtAddCallback(pmOptButt[5], XmNvalueChangedCallback, ToggleWaveLengthScale, NULL);
  XtAddCallback(pmOptButt[5], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);

  XmToggleButtonSetState(pmOptButt[1], true, false);



  /* Equal-log interval averaging
   */
  n = 0;
  frame = XmCreateFrame(optRC, "eliaFrame", args, 0);
  XtManageChild(frame);

  n = 0;
  RC[4] = XmCreateRowColumn(frame, "plRCv", args, n);

  n = 0;
  pmOptButt[6] = XmCreateToggleButton(RC[4], "Equal-log interval averaging", args,n);
  XtManageChild(pmOptButt[6]);
  XtAddCallback(pmOptButt[6], XmNvalueChangedCallback, (XtCallbackProc)PlotSpectrum, NULL);

  n = 0;
  plRC = XmCreateRowColumn(RC[4], "plRC", args, n);
  XtManageChild(plRC);

  XtManageChild(XmCreateLabel(plRC, "Total number points", args, n));
  eliaText = XmCreateTextField(plRC, "eliaText", args, n);
  XtManageChild(eliaText);
  XmTextFieldSetString(eliaText, "30");

  XtAddCallback(eliaText, XmNlosingFocusCallback, ValidateInteger, NULL);
  XtAddCallback(eliaText, XmNlosingFocusCallback, (XtCallbackProc)PlotSpectrum, NULL);


  /* Time shift.
   */
  n = 0;
  frame = XmCreateFrame(optRC, "specDefsFrame", args, 0);
  XtManageChild(frame);

  n = 0;
  RC[5] = XmCreateRowColumn(frame, "plRC", args, n);

  XtManageChild(XmCreateLabel(RC[5], "Time shift", args, n));
  tShift = XmCreateTextField(RC[5], "timeShift", args, n);
  XtManageChild(tShift);

  XtAddCallback(tShift, XmNlosingFocusCallback, ValidateInteger, NULL);
  XtAddCallback(tShift, XmNlosingFocusCallback, SpecWinUp, NULL);

  XtManageChild(XmCreateLabel(RC[5], "milliseconds", args, n));


  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]); XtManageChild(RC[5]);

}	/* END CREATESPECTRUMWINDOW */

/* -------------------------------------------------------------------- */
static void setDefaults()
{
  size_t i, x;
  Arg	args[2];
  XmString name;

  /* Set default values for SegLen, Detrend, and Window.
   */
  for (x = i = 0; segLenInfo[i].name; ++i)
    if ((size_t)segLenInfo[i].client == psd[0].M)
      x = i;

  name = XmStringCreateLocalized(segLenInfo[x].name);
  XtSetArg(args[0], XmNlabelString, name);
  XtSetValues(XmOptionButtonGadget(slOpMenu), args, 1);
  XmStringFree(name);
 
  for (x = i = 0; windowInfo[i].name; ++i)
    if ((int)windowInfo[i].client == (int)psd[0].windowFn)
      x = i;

  name = XmStringCreateLocalized(windowInfo[x].name);
  XtSetArg(args[0], XmNlabelString, name);
  XtSetValues(XmOptionButtonGadget(winOpMenu), args, 1);
  XmStringFree(name);
 
  for (x = i = 0; detrendInfo[i].name; ++i)
    if ((int)detrendInfo[i].client == (int)psd[0].detrendFn)
      x = i;
 
  name = XmStringCreateLocalized(detrendInfo[x].name);
  XtSetArg(args[0], XmNlabelString, name);
  XtSetValues(XmOptionButtonGadget(dtOpMenu), args, 1);
  XmStringFree(name);
 
}	/* END SETDEFAULTS */

/* -------------------------------------------------------------------- */
static Widget CreateDropDownMenu(Widget parent, char menu_name[],
	struct ccb_info menu_info[])
{
  int           i;
  Widget        PD, OM, buttons[16];
  Cardinal      n;
  Arg           args[4];
  XmString      name;


  /* Pre-filter menu
   */
  n = 0;
  PD = XmCreatePulldownMenu(parent, "pullDown", args, n);

  n = 0;
  XtSetArg(args[n], XmNsubMenuId, PD); ++n;
  OM = XmCreateOptionMenu(parent, menu_name, args, n);
  XtManageChild(OM);


  for (i = 0; menu_info[i].name; ++i)
    {
    name = XmStringCreateLocalized(menu_info[i].name);

    n = 0;
    XtSetArg(args[n], XmNlabelString, name); ++n;
    buttons[i] = XmCreatePushButton(PD, "opMenB", args, n);
    XtAddCallback(buttons[i], XmNactivateCallback, menu_info[i].callBack,
                  (XtPointer)menu_info[i].client);

    XmStringFree(name);
    }

  XtManageChildren(buttons, i);

  return(OM);

}	/* END CREATEDROPDOWNMENU */

/* END SPEC.C */
