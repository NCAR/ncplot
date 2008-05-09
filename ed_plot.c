/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_plot.c

FULL NAME:	Callbacks for Edit Main Plot Parameters

ENTRY POINTS:	EditMainParms()
		SetMainDefaults()

STATIC FNS:	CreateMainParmsWindow()
		ApplyMainParms()
		SetMainAutoScale()
		SetPlotPanel()
		SetReadData()

DESCRIPTION:	

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

static const size_t TOTAL_PARMS = 17;

extern Widget	AppShell;

static Widget	MainParmsShell = NULL, MainParmsWindow, parmsText[TOTAL_PARMS],
		asciiText[2], autoScaleButton, autoTicsButton;

static std::vector<Widget> panelB;

static int	currentPanel = 0;

static void	CreateMainParmsWindow(),
		SetPlotPanel(Widget, XtPointer, XtPointer),
		ApplyMainParms(Widget w, XtPointer client, XtPointer call);

void SetReadData(Widget w, XtPointer client, XtPointer call);
void SetMainDefaults();


/* -------------------------------------------------------------------- */
void EditMainParms(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = True;

  if (firstTime)
    {
    CreateMainParmsWindow();
    firstTime = False;
    }

  XtManageChild(MainParmsWindow);
  XtPopup(XtParent(MainParmsWindow), XtGrabNone);

  SetMainDefaults();

}	/* END EDITMAINPARMS */

/* -------------------------------------------------------------------- */
void SetMainDefaults()
{
  size_t i;

  if (!MainParmsShell)
    return;

  SetDefaults(parmsText, &mainPlot[currentPanel]);
//  SetLogInvert(parmsTB, &mainPlot[currentPanel], Y_AXIS);

  /* For main plot the following are all tied together,
   * override current plot.
   */
  XmTextFieldSetString(parmsText[0], (char *)mainPlot[0].title.c_str());
  XmTextFieldSetString(parmsText[1], (char *)mainPlot[0].subTitle.c_str());
  XmTextFieldSetString(parmsText[2], (char *)mainPlot[0].Xaxis.label.c_str());

  for (i = 0; i < NumberOfPanels; ++i)
    XtSetSensitive(panelB[i], True);

  for (; i < MAX_PANELS; ++i)
    XtSetSensitive(panelB[i], False);

  XmToggleButtonSetState(autoScaleButton, mainPlot[currentPanel].autoScale, False);
  XmToggleButtonSetState(autoTicsButton, mainPlot[currentPanel].autoTics, False);

  for (i = 7; i < 11; ++i)
    XtSetSensitive(parmsText[i], 1-mainPlot[currentPanel].autoScale);

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], 1-mainPlot[currentPanel].autoTics);

  sprintf(buffer, "%ld", nASCIIpoints);
  XmTextFieldSetString(asciiText[0], buffer);
  XmTextFieldSetString(asciiText[1], asciiFormat);

}	/* END SETMAINDEFAULTS */

/* -------------------------------------------------------------------- */
static void SetMainAutoScale(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 7; i < 11; ++i)
    XtSetSensitive(parmsText[i], mainPlot[currentPanel].autoScale);

  mainPlot[currentPanel].autoScale = 1 - mainPlot[currentPanel].autoScale;

}	/* END SETMAINAUTOSCALE */

/* -------------------------------------------------------------------- */
static void SetMainAutoTics(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], mainPlot[currentPanel].autoTics);

  mainPlot[currentPanel].autoTics = 1 - mainPlot[currentPanel].autoTics;

}	/* END SETMAINAUTOSCALE */

/* -------------------------------------------------------------------- */
static void ApplyMainParms(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

  ApplyParms(parmsText, &mainPlot[currentPanel]);
//  ApplyLogInvert(parmsTB, &mainPlot[currentPanel], Y_AXIS);

  /* These are all tied to mainPlot[0].  Not indvidually setable.
  */
  for (size_t i = 0; i < MAX_PANELS; ++i)
    {
    mainPlot[i].title = mainPlot[currentPanel].title;
    mainPlot[i].subTitle = mainPlot[currentPanel].subTitle;
    mainPlot[i].Xaxis.label = mainPlot[currentPanel].Xaxis.label;

    mainPlot[i].Xaxis.nMajorTics = mainPlot[currentPanel].Xaxis.nMajorTics;
    mainPlot[i].Xaxis.nMinorTics = mainPlot[currentPanel].Xaxis.nMinorTics;
    }

  p = XmTextFieldGetString(asciiText[0]);
  nASCIIpoints = atoi(p);
  XtFree(p);

  p = XmTextFieldGetString(asciiText[1]);
  strcpy(asciiFormat, p);
  XtFree(p);

  DrawMainWindow();

  if (AsciiWinOpen)
    SetASCIIdata(NULL, NULL, NULL);

}	/* END APPLYMAINPARMS */

/* -------------------------------------------------------------------- */
static void SetPlotPanel(Widget w, XtPointer client, XtPointer call)
{
  currentPanel = (long)client;
  EditMainParms(NULL, NULL, NULL);

}	/* END SETPLOTPANEL */

/* -------------------------------------------------------------------- */
static void CreateMainParmsWindow()
{
  Cardinal	n;
  Arg		args[2];
  Widget	RC[9], plRC, label;

  for (size_t i = 0; i < TOTAL_PARMS; ++i)
    parmsText[i] = NULL;

  MainParmsShell = XtCreatePopupShell("editParmsShell",
                   topLevelShellWidgetClass, AppShell, NULL, 0);

  MainParmsWindow = XmCreateRowColumn(MainParmsShell, "parmsRC", NULL, 0);

  RC[0] = createParamsTitles(MainParmsWindow, parmsText);


  /* Panel stuff.
   */
  n = 0;
  plRC = XmCreateRowColumn(RC[0], "plRC", args, n);
  XtManageChild(plRC);

  label = XmCreateLabel(plRC, "Panel", args, n);
  plRC = XmCreateRadioBox(plRC, "pnRC", args, n);
  XtManageChild(label);
  XtManageChild(plRC);

  for (size_t i = 0; i < MAX_PANELS; ++i)
    {
    sprintf(buffer, "%ld", i+1);
    panelB.push_back(XmCreateToggleButton(plRC, buffer, NULL, 0));

    XtAddCallback(panelB[i], XmNvalueChangedCallback,SetPlotPanel,(XtPointer)i);
    }

  XtManageChildren(&panelB[0], MAX_PANELS);
  XmToggleButtonSetState(panelB[0], True, False);



  RC[1] = createParamsLabels(MainParmsWindow, parmsText, &mainPlot[0]);
  RC[2] = createParamsMinMax(MainParmsWindow, parmsText, &mainPlot[0],
			&autoScaleButton);
  RC[3] = createParamsTics(MainParmsWindow, parmsText, &mainPlot[0],
			&autoTicsButton);
//  RC[4] = createLogInvert(MainParmsWindow, parmsTB, ApplyMainParms,
//				&mainPlot[0], Y_AXIS);

  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, SetMainAutoScale, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, SetMainAutoTics, NULL);
  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, ApplyMainParms, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, ApplyMainParms, NULL);

  /* ASCII parameters.
   */
  n = 0;
  RC[4] = XmCreateFrame(MainParmsWindow, "ASCIIframe", args, n);

  label = XmCreateLabel(RC[4], "ASCIIparms", args, n);
  XtManageChild(label);

  plRC = XmCreateRowColumn(RC[4], "plRC", args, n);
  XtManageChild(plRC);


  label = XmCreateLabel(plRC, "asciiNPlbl", args, n);
  XtManageChild(label);
  asciiText[0] = XmCreateTextField(plRC, "asciiNPtxt", args, n);
  XtAddCallback(asciiText[0], XmNlosingFocusCallback, ApplyMainParms, NULL);

  label = XmCreateLabel(plRC, "asciiFlbl", args, n);
  XtManageChild(label);
  asciiText[1] = XmCreateTextField(plRC, "asciiFtxt", args, n);
  XtAddCallback(asciiText[1], XmNlosingFocusCallback, ApplyMainParms, NULL);

  XtManageChildren(asciiText, 2);

  RC[5] = createARDbuttons(MainParmsWindow);
  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]); XtManageChild(RC[5]);

  for (size_t i = 0; i < TOTAL_PARMS; ++i)
    if (parmsText[i])
      XtAddCallback(parmsText[i], XmNlosingFocusCallback, ApplyMainParms, NULL);

}	/* END CREATEPLOTPARMSWINDOW */

/* END ED_PLOT.C */
