/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_xy.c

FULL NAME:	Callbacks for Edit XY Parameteres

ENTRY POINTS:	EditXYParms()
		SetXYDefaults()

STATIC FNS:	CreateXYParmsWindow()
		ApplyXYParms()
		SetXYAutoScale()

DESCRIPTION:	

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

static const size_t TOTAL_PARMS = 17;

extern Widget	AppShell;
static Widget	XYShell = NULL, XYParmsWindow, parmsText[TOTAL_PARMS],
		autoScaleButton, autoTicsButton;

static std::vector<Widget> panelB;

static int      currentPanel = 0;

static void	CreateXYParmsWindow(), 
		ApplyXYParms(Widget w, XtPointer client, XtPointer call);


/* -------------------------------------------------------------------- */
void EditXYParms(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = True;

  if (firstTime)
    {
    CreateXYParmsWindow();
    firstTime = False;
    }

  XtManageChild(XYParmsWindow);
  XtPopup(XtParent(XYParmsWindow), XtGrabNone);

  SetXYDefaults();

}	/* END EDITXYPARMS */

/* -------------------------------------------------------------------- */
void SetXYDefaults()
{
  size_t i;

  if (XYShell == NULL)
    return;

  SetDefaults(parmsText, &xyyPlot[currentPanel]);
//  SetLogInvert(parmsTB, &xyyPlot[currentPanel], X_AXIS | Y_AXIS);

  XmTextFieldSetString(parmsText[0], const_cast<char *>(xyyPlot[0].title.c_str()));
  XmTextFieldSetString(parmsText[1], const_cast<char *>(xyyPlot[0].subTitle.c_str()));
 
  for (i = 0; i < NumberOfXYpanels; ++i)
    XtSetSensitive(panelB[i], True);
 
  for (; i < MAX_PANELS; ++i)
    XtSetSensitive(panelB[i], False);
 
  XmToggleButtonSetState(autoScaleButton, xyyPlot[currentPanel].autoScale, False);
  XmToggleButtonSetState(autoTicsButton, xyyPlot[currentPanel].autoTics, False);
 
  for (i = 5; i < 11; ++i)
    XtSetSensitive(parmsText[i], 1-xyyPlot[currentPanel].autoScale);

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], 1-xyyPlot[currentPanel].autoTics);

}	/* END SETXYDEFAULTS */

/* -------------------------------------------------------------------- */
static void SetXYAutoScale(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 5; i < 11; ++i)
    XtSetSensitive(parmsText[i], xyyPlot[currentPanel].autoScale);

  xyyPlot[currentPanel].autoScale = 1 - xyyPlot[currentPanel].autoScale;

}	/* END SETXYDEFAULTS */

/* -------------------------------------------------------------------- */
static void SetXYAutoTics(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], xyyPlot[currentPanel].autoTics);

  xyyPlot[currentPanel].autoTics = 1 - xyyPlot[currentPanel].autoTics;

}	/* END SETXYDEFAULTS */

/* -------------------------------------------------------------------- */
static void ApplyXYParms(Widget w, XtPointer client, XtPointer call)
{
  ApplyParms(parmsText, &xyyPlot[currentPanel]);
//  ApplyLogInvert(parmsTB, &xyyPlot[currentPanel], X_AXIS | Y_AXIS);

  for (size_t i = 0; i < MAX_PANELS; ++i)
    {
    xyyPlot[i].title = xyyPlot[currentPanel].title;
    xyyPlot[i].subTitle = xyyPlot[currentPanel].subTitle;
    }

  DrawMainWindow();

}	/* END APPLYXYPARMS */

/* -------------------------------------------------------------------- */
static void SetXYPanel(Widget w, XtPointer client, XtPointer call)
{
  currentPanel = (long)client;
  EditXYParms(NULL, NULL, NULL);
 
}       /* END SETPLOTPANEL */

/* -------------------------------------------------------------------- */
static void CreateXYParmsWindow()
{
  size_t	i;
  Widget	RC[6];

  XYShell = XtCreatePopupShell("editXYShell",
  topLevelShellWidgetClass, AppShell, NULL, 0);

  XYParmsWindow = XmCreateRowColumn(XYShell, (char *)"parmsRC", NULL, 0);

  for (i = 0; i < TOTAL_PARMS; ++i)
    parmsText[i] = NULL;

  RC[0] = createParamsTitles(XYParmsWindow, parmsText);
  createPanelButts(RC[0], panelB, SetXYPanel);

  RC[1] = createParamsLabels(XYParmsWindow, parmsText, &xyyPlot[0]);
  RC[2] = createParamsMinMax(XYParmsWindow, parmsText, &xyyPlot[0], &autoScaleButton);

  RC[3] = createParamsTics(XYParmsWindow, parmsText, &xyyPlot[0], &autoTicsButton);
//  RC[4] = createLogInvert(XYParmsWindow, parmsTB, ApplyXYParms,
//				&xyyPlot[0], X_AXIS | Y_AXIS);
  RC[4] = createARDbuttons(XYParmsWindow);
  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]);

  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, ApplyXYParms, NULL);
  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, SetXYAutoScale, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, ApplyXYParms, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, SetXYAutoTics, NULL);

  for (i = 0; i < TOTAL_PARMS-1; ++i)
    if (parmsText[i])
      XtAddCallback(parmsText[i], XmNlosingFocusCallback, ApplyXYParms, NULL);

}	/* END CREATEXYPARMSWINDOW */

/* END ED_XY.C */
