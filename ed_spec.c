/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_spec.c

FULL NAME:	Callbacks for Edit Spectral Parameteres

ENTRY POINTS:	CreateSpecParmsWindow()
		EditSpecParms()

STATIC FNS:	SetSpecDefaults()
		ApplySpecParms()
		SetSpecAutoScale()
		SetSpecAutoTics()

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2002
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>

static const int TOTAL_PARMS = 17;

extern Widget	AppShell;
static Widget	SpecShell = NULL, SpecParmsWindow, parmsText[TOTAL_PARMS],
		parmsTB[6], autoScaleButton, autoTicsButton;
 
static void	CreateSpecParmsWindow(),
		ApplySpecParms(Widget w, XtPointer client, XtPointer call);

void	SetSpecDefaults();


/* -------------------------------------------------------------------- */
void EditSpecParms(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = True;

  if (firstTime)
    {
    CreateSpecParmsWindow();
    firstTime = False;
    }

  XtManageChild(SpecParmsWindow);
  XtPopup(XtParent(SpecParmsWindow), XtGrabNone);

  SetSpecDefaults();

}	/* END EDITSPECPARMS */

/* -------------------------------------------------------------------- */
void SetSpecDefaults()
{
  int	i;

  if (SpecShell == NULL)
    return;

  SetDefaults(parmsText, &specPlot);
  SetLogInvert(parmsTB, &specPlot, X_AXIS | Y_AXIS);

  XmToggleButtonSetState(autoScaleButton, specPlot.autoScale, False);
  XmToggleButtonSetState(autoTicsButton, specPlot.autoTics, False);

  for (i = 5; i < 9; ++i)
    XtSetSensitive(parmsText[i], 1-specPlot.autoScale);

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], 1-specPlot.autoTics);

}	/* END SETSPECDEFAULTS */

/* -------------------------------------------------------------------- */
static void ApplySpecParms(Widget w, XtPointer client, XtPointer call)
{
  ApplyParms(parmsText, &specPlot);
ApplyLogInvert(parmsTB, &specPlot, X_AXIS | Y_AXIS);

  if (specPlot.windowOpen)
    PlotSpectrum(NULL, NULL, NULL);

  SetSpecDefaults();

}	/* END APPLYSPECPARMS */

/* -------------------------------------------------------------------- */
static void SetSpecAutoScale(Widget w, XtPointer client, XtPointer call)
{
  int     i;

  for (i = 5; i < 9; ++i)
    XtSetSensitive(parmsText[i], specPlot.autoScale);

  specPlot.autoScale = 1 - specPlot.autoScale;

}   /* END SETDIFFDEFAULTS */

/* -------------------------------------------------------------------- */
static void SetSpecAutoTics(Widget w, XtPointer client, XtPointer call)
{
  int     i;

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], specPlot.autoTics);

  specPlot.autoTics = 1 - specPlot.autoTics;

}   /* END SETDIFFDEFAULTS */

/* -------------------------------------------------------------------- */
void CreateSpecParmsWindow()
{
  int		n, i;
  Widget	RC[6];
  Arg		args[10];

  n = 0;
  SpecShell = XtCreatePopupShell("editSpecShell",
                topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  SpecParmsWindow = XmCreateRowColumn(SpecShell, "parmsRC", args, n);

  for (i = 0; i < TOTAL_PARMS; ++i)
    parmsText[i] = NULL;

  RC[0] = createParamsTitles(SpecParmsWindow, parmsText);
  RC[1] = createParamsLabels(SpecParmsWindow, parmsText, &specPlot);
  RC[2] = createParamsMinMax(SpecParmsWindow, parmsText, &specPlot, &autoScaleButton);
  RC[3] = createParamsTics(SpecParmsWindow, parmsText, &specPlot, &autoTicsButton);
RC[4] = createLogInvert(SpecParmsWindow, parmsTB, ApplySpecParms,
                                &specPlot, X_AXIS | Y_AXIS);
  RC[5] = createARDbuttons(SpecParmsWindow);

  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]); XtManageChild(RC[5]);


  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, SetSpecAutoScale, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, SetSpecAutoTics, NULL);
  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, ApplySpecParms, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, ApplySpecParms, NULL);

  for (i = 0; i < TOTAL_PARMS-1; ++i)
    if (parmsText[i])
      XtAddCallback(parmsText[i], XmNlosingFocusCallback, ApplySpecParms, NULL);

}	/* END CREATESPECPARMSWINDOW */

/* END ED_SPEC.C */
