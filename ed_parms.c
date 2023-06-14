/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_parms.c

FULL NAME:	Building blocks for all ed_*.c files.

ENTRY POINTS:	ApplyParms()
		ApplyLogInvert()
		SetDefaults()
		SetLogInvert()
		SetXminMax()
		SetYminMax()
		SetZminMax()
		SetSubtitles()

STATIC FNS:	none

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2022
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

void	SetXminMax(Widget *, PLOT_INFO *),
	SetYminMax(Widget *, PLOT_INFO *),
	SetZminMax(Widget *, PLOT_INFO *);


/* -------------------------------------------------------------------- */
void ApplyParms(Widget parmsText[], PLOT_INFO *plot)
{
  char	*p;

  p = XmTextFieldGetString(parmsText[0]);
  plot->title = p;
  XtFree(p);

  p = XmTextFieldGetString(parmsText[1]);
  plot->subTitle = p;
  XtFree(p);

  p = XmTextFieldGetString(parmsText[2]);
  plot->Xaxis.label = p;
  XtFree(p);

  if (plot->plotType == XYZ_PLOT)
    {
    p = XmTextFieldGetString(parmsText[4]);
    plot->Yaxis[0].label = p;
    XtFree(p);

    p = XmTextFieldGetString(parmsText[3]);
    plot->Zaxis.label = p;
    XtFree(p);

    p = XmTextFieldGetString(parmsText[7]);
    plot->Zaxis.min = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[8]);
    plot->Zaxis.max = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[9]);
    plot->Yaxis[0].min = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[10]);
    plot->Yaxis[0].max = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[13]);
    plot->Zaxis.nMajorTics = atoi(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[14]);
    plot->Zaxis.nMinorTics = atoi(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[15]);
    plot->Yaxis[0].nMajorTics = atoi(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[16]);
    plot->Yaxis[0].nMinorTics = atoi(p);
    XtFree(p);
    }
  else
    {
    p = XmTextFieldGetString(parmsText[3]);
    plot->Yaxis[0].label = p;
    XtFree(p);

    p = XmTextFieldGetString(parmsText[4]);
    plot->Yaxis[1].label = p;
    XtFree(p);

    p = XmTextFieldGetString(parmsText[7]);
    plot->Yaxis[0].min = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[8]);
    plot->Yaxis[0].max = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[9]);
    plot->Yaxis[1].min = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[10]);
    plot->Yaxis[1].max = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[13]);
    plot->Yaxis[0].nMajorTics = atoi(p);
    plot->Yaxis[1].nMajorTics = atoi(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[14]);
    plot->Yaxis[0].nMinorTics = atoi(p);
    plot->Yaxis[1].nMinorTics = atoi(p);
    XtFree(p);
    }

  if (plot->plotType != TIME_SERIES)
    {
    p = XmTextFieldGetString(parmsText[5]);
    plot->Xaxis.min = atof(p);
    XtFree(p);

    p = XmTextFieldGetString(parmsText[6]);
    plot->Xaxis.max = atof(p);
    XtFree(p);
    }

  p = XmTextFieldGetString(parmsText[11]);
  plot->Xaxis.nMajorTics = atoi(p);
  XtFree(p);

  p = XmTextFieldGetString(parmsText[12]);
  plot->Xaxis.nMinorTics = atoi(p);
  XtFree(p);

}	/* END APPLYPARMS */

/* -------------------------------------------------------------------- */
void ApplyLogInvert(Widget parms[], PLOT_INFO *plot, int axies)
{
  if (axies & X_AXIS)
    {
    plot->Xaxis.logScale = XmToggleButtonGetState(parms[0]);
    plot->Xaxis.invertAxis = XmToggleButtonGetState(parms[3]);
    }

  plot->Yaxis[0].logScale = XmToggleButtonGetState(parms[1]);
  plot->Yaxis[0].invertAxis = XmToggleButtonGetState(parms[4]);

  plot->Yaxis[1].logScale = XmToggleButtonGetState(parms[2]);
  plot->Yaxis[1].invertAxis = XmToggleButtonGetState(parms[5]);

}	/* END APPLYLOGINVERT */

/* -------------------------------------------------------------------- */
void SetDefaults(Widget parmsText[], PLOT_INFO *plot)
{
  XmTextFieldSetString(parmsText[0], const_cast<char *>(plot->title.c_str()));
  XmTextFieldSetString(parmsText[1], const_cast<char *>(plot->subTitle.c_str()));
  XmTextFieldSetString(parmsText[2], const_cast<char *>(plot->Xaxis.label.c_str()));
  XmTextFieldSetString(parmsText[3], const_cast<char *>(plot->Yaxis[0].label.c_str()));

  SetXminMax(parmsText, plot);
  SetYminMax(parmsText, plot);

  snprintf(buffer, BUFFSIZE, "%d", plot->Xaxis.nMajorTics);
  XmTextFieldSetString(parmsText[11], buffer);

  snprintf(buffer, BUFFSIZE, "%d", plot->Xaxis.nMinorTics);
  XmTextFieldSetString(parmsText[12], buffer);


  if (plot->plotType == XYZ_PLOT)
    {
    XmTextFieldSetString(parmsText[3], const_cast<char *>(plot->Zaxis.label.c_str()));
    XmTextFieldSetString(parmsText[4], const_cast<char *>(plot->Yaxis[0].label.c_str()));
    SetZminMax(parmsText, plot);

    snprintf(buffer, BUFFSIZE, "%d", plot->Zaxis.nMajorTics);
    XmTextFieldSetString(parmsText[13], buffer);

    snprintf(buffer, BUFFSIZE, "%d", plot->Zaxis.nMinorTics);
    XmTextFieldSetString(parmsText[14], buffer);

    snprintf(buffer, BUFFSIZE, "%d", plot->Yaxis[0].nMajorTics);
    XmTextFieldSetString(parmsText[15], buffer);

    snprintf(buffer, BUFFSIZE, "%d", plot->Yaxis[0].nMinorTics);
    XmTextFieldSetString(parmsText[16], buffer);
    }
  else
    {
    XmTextFieldSetString(parmsText[3], const_cast<char *>(plot->Yaxis[0].label.c_str()));
    XmTextFieldSetString(parmsText[4], const_cast<char *>(plot->Yaxis[1].label.c_str()));

    snprintf(buffer, BUFFSIZE, "%d", plot->Yaxis[0].nMajorTics);
    XmTextFieldSetString(parmsText[13], buffer);

    snprintf(buffer, BUFFSIZE, "%d", plot->Yaxis[0].nMinorTics);
    XmTextFieldSetString(parmsText[14], buffer);
    }

}	/* END SETDEFAULTS */

/* -------------------------------------------------------------------- */
void SetLogInvert(Widget parms[], PLOT_INFO *plot, int axies)
{
  if (axies & X_AXIS)
    {
    XmToggleButtonSetState(parms[0], plot->Xaxis.logScale, False);
    XmToggleButtonSetState(parms[3], plot->Xaxis.invertAxis, False);
    }

  XmToggleButtonSetState(parms[1], plot->Yaxis[0].logScale, False);
  XmToggleButtonSetState(parms[4], plot->Yaxis[0].invertAxis, False);

  XmToggleButtonSetState(parms[2], plot->Yaxis[1].logScale, False);
  XmToggleButtonSetState(parms[5], plot->Yaxis[1].invertAxis, False);

}	/* END SETLOGINVERT */

/* -------------------------------------------------------------------- */
void SetXminMax(Widget parmsText[], PLOT_INFO *plot)
{
  if (plot->plotType != TIME_SERIES)
    {
    snprintf(buffer, BUFFSIZE, "%g", plot->Xaxis.min);
    XmTextFieldSetString(parmsText[5], buffer);
    snprintf(buffer, BUFFSIZE, "%g", plot->Xaxis.max);
    XmTextFieldSetString(parmsText[6], buffer);
    }

}	/* END SETXMINMAX */

/* -------------------------------------------------------------------- */
void SetYminMax(Widget parmsText[], PLOT_INFO *plot)
{
  if (plot->plotType == XYZ_PLOT)
    {
    snprintf(buffer, BUFFSIZE, "%g", plot->Zaxis.min);
    XmTextFieldSetString(parmsText[7], buffer);
    snprintf(buffer, BUFFSIZE, "%g", plot->Zaxis.max);
    XmTextFieldSetString(parmsText[8], buffer);
    }
  else
    {
    snprintf(buffer, BUFFSIZE, "%g", plot->Yaxis[0].min);
    XmTextFieldSetString(parmsText[7], buffer);
    snprintf(buffer, BUFFSIZE, "%g", plot->Yaxis[0].max);
    XmTextFieldSetString(parmsText[8], buffer);

    snprintf(buffer, BUFFSIZE, "%g", plot->Yaxis[1].min);
    XmTextFieldSetString(parmsText[9], buffer);
    snprintf(buffer, BUFFSIZE, "%g", plot->Yaxis[1].max);
    XmTextFieldSetString(parmsText[10], buffer);
    }

}	/* END SETYMINMAX */

/* -------------------------------------------------------------------- */
void SetZminMax(Widget parmsText[], PLOT_INFO *plot)
{
  if (plot->plotType == XYZ_PLOT)
    {
    snprintf(buffer, BUFFSIZE, "%g", plot->Yaxis[0].min);
    XmTextFieldSetString(parmsText[9], buffer);
    snprintf(buffer, BUFFSIZE, "%g", plot->Yaxis[0].max);
    XmTextFieldSetString(parmsText[10], buffer);
    }

}	/* END SETZMINMAX */

/* -------------------------------------------------------------------- */
void SetSubtitles()
{
  char	tmp[256];

  if (dataFile[0].FlightDate.length() > 0)
    {
    strcpy(buffer, dataFile[0].FlightDate.c_str());
    strcat(buffer, ", ");
    }
  else
    buffer[0] = '\0';

  snprintf(tmp, 25, "%02d:%02d:%02d-%02d:%02d:%02d",
        UserStartTime[0], UserStartTime[1], UserStartTime[2],
        UserEndTime[0], UserEndTime[1], UserEndTime[2]);

  strcat(buffer, tmp);

  for (size_t i = 0; i < MAX_PANELS; ++i)
    {
    mainPlot[i].subTitle = buffer;
    xyyPlot[i].subTitle = buffer;
    }

  specPlot.subTitle = mainPlot[0].subTitle;
  xyzPlot.subTitle = mainPlot[0].subTitle;

  SetMainDefaults();
  SetSpecDefaults();
  SetTrackDefaults();
  SetXYDefaults();

}	/* END SETSUBTITLES */

/* END ED_PARMS.C */
