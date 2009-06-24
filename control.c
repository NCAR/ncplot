/*
-------------------------------------------------------------------------
OBJECT NAME:	control.c

FULL NAME:	Callbacks for Control Window

ENTRY POINTS:	CreateControlWindow()
		OpenControlWindow()
		ResizeMainWindow()
		DrawMainWindow()
		SetDataFile()
		SetTimeText()
		ChangePlotType()
		SetInvertLogScale()
		choosingXaxis()
		choosingYaxis()
		choosingZaxis()
		whichSide()

STATIC FNS:	ApplyTimeChange()
		ApplyInvert()
		ApplyLogScale()
		changeAxis()

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2007
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

std::vector<Widget> panelB;	/* For export to panel.c	*/

static std::vector<Widget> fileB;

static Widget	plotType[3], invert, logScale,
		whichAxis[3], scaleLocation[2], RC[9], frame[9],
		optButton[5], timeText[2], realTime, rtText, lineThickTxt;

static int	currentAxis = Y_AXIS;

static char *helpURL = "http://www.eol.ucar.edu/raf/Software/ncplot.html";

void	ChangePlotType(Widget, XtPointer, XtPointer);
void	CheckForTemplateFile();
void	SetInvertLogScale(Widget, XtPointer, XmToggleButtonCallbackStruct *);

static void	ApplyTimeChange(Widget, XtPointer, XtPointer),
		changeAxis(Widget, XtPointer, XtPointer);


extern Widget	ControlWindow;


/* -------------------------------------------------------------------- */
void ResizeMainWindow(Widget w, XtPointer client, XtPointer call)
{
  switch (PlotType)
    {
    case TIME_SERIES:
      ResizeTimeSeries();
      break;

    case XY_PLOT:
      ResizeXY();
      break;

    case XYZ_PLOT:
      ResizeXYZ();
      break;
    }

  DrawMainWindow();

}	/* END RESIZEMAINWINDOW */

/* -------------------------------------------------------------------- */
void DrawMainWindow()
{
  if (!RealTime)
    WaitCursorAll();

  if (DataChanged)
    {
    ClearAnnotations();

    if (StatsWinOpen)
      SetStatsData();

    if (diffPlot.windowOpen)
      PlotDifference(NULL, NULL, NULL);

    if (specPlot.windowOpen)
      SpecWinUp(NULL, NULL, NULL);
    else
    if (AsciiWinOpen)
      SetASCIIdata(NULL, NULL, NULL);
    }

  switch (PlotType)
    {
    case TIME_SERIES:
      AutoScale();
      DrawTimeSeries();

      XDrawString(mainPlot[0].dpy, mainPlot[0].win, mainPlot[0].gc,
                  360, 15, helpURL, strlen(helpURL));

      XCopyArea(mainPlot[0].dpy, mainPlot[0].win,
	XtWindow(mainPlot[0].canvas), mainPlot[0].gc,
	0, 0, mainPlot[0].x.windowWidth, mainPlot[0].x.windowHeight, 0, 0);

      break;

    case XY_PLOT:
      AutoScaleXY();
      DrawXY();
      XCopyArea(xyyPlot[0].dpy, xyyPlot[0].win,
	XtWindow(xyyPlot[0].canvas), xyyPlot[0].gc,
	0, 0, xyyPlot[0].x.windowWidth, xyyPlot[0].x.windowHeight, 0, 0);

      break;

    case XYZ_PLOT:
      AutoScaleXYZ();
      DrawXYZ();
      XCopyArea(xyzPlot.dpy, xyzPlot.win, XtWindow(xyzPlot.canvas), xyzPlot.gc,
           0, 0, xyzPlot.x.windowWidth, xyzPlot.x.windowHeight, 0, 0);

      break;
    }

  DataChanged = False;

  if (!RealTime)
    PointerCursorAll();

}	/* END DRAWMAINWINDOW */

/* -------------------------------------------------------------------- */
void ExposeMainWindow(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *call)
{
  static bool firstTime = True;
  XExposeEvent *evt = (XExposeEvent *)call->event;

  if (firstTime)
    {
    DrawMainWindow();
    CheckForTemplateFile();
    firstTime = False;
    }

  switch (PlotType)
    {
    case TIME_SERIES:
      XCopyArea(mainPlot[0].dpy, mainPlot[0].win,
		XtWindow(mainPlot[0].canvas), mainPlot[0].gc,
		evt->x, evt->y, evt->width, evt->height, evt->x, evt->y);
      break;

    case XY_PLOT:
      XCopyArea(xyyPlot[0].dpy, xyyPlot[0].win,
		XtWindow(xyyPlot[0].canvas), xyyPlot[0].gc,
		evt->x, evt->y, evt->width, evt->height, evt->x, evt->y);
      break;

    case XYZ_PLOT:
      XCopyArea(xyzPlot.dpy, xyzPlot.win, XtWindow(xyzPlot.canvas), xyzPlot.gc,
           evt->x, evt->y, evt->width, evt->height, evt->x, evt->y);
      break;
    }

}	/* END EXPOSEMAINWINDOW */

/* -------------------------------------------------------------------- */
void OpenControlWindow(Widget w, XtPointer client, XtPointer call)
{
  size_t	i;
  XmString	label;
  Arg		args[2];

  if (!RealTime)
    {
    for (i = 0; i < NumberDataFiles; ++i)
      {
      XtSetSensitive(fileB[i], True);

      label = XmStringCreate(const_cast<char *>(dataFile[i].fileName.c_str()), XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[0], XmNlabelString, label);
      XtSetValues(fileB[i], args, 1);
      XmStringFree(label);

      if (i == CurrentDataFile)
        XmToggleButtonSetState(fileB[i], True, True);
      }

    for (; i < MAX_DATAFILES; ++i)
      {
      XtSetSensitive(fileB[i], False);

      label = XmStringCreate("none", XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[0], XmNlabelString, label);
      XtSetValues(fileB[i], args, 1);
      XmStringFree(label);
      }
    }
  else
    {
    sprintf(buffer, "%ld", NumberSeconds);
    XmTextFieldSetString(rtText, buffer);
    }

  for (i = 0; i < NumberOfPanels; ++i)
    XtSetSensitive(panelB[i], True);

  for (; i < MAX_PANELS; ++i)
    XtSetSensitive(panelB[i], False);

  XtManageChild(ControlWindow);
  XtPopup(XtParent(ControlWindow), XtGrabNone);

}	/* END OPENCONTROLWINDOW */

/* -------------------------------------------------------------------- */
void FromSecondsSinceMidnite(int timeS[])
{
  long	start = timeS[3];

  timeS[0] = start / 3600; start -= timeS[0] * 3600;
  timeS[1] = start / 60; start -= timeS[1] * 60;
  timeS[2] = start;

}	/* END FROMSECONDSSINCEMIDNITE */

/* -------------------------------------------------------------------- */
void SetTimeText()
{
  /* Set start/end time.
   */
  sprintf(buffer, "%02d:%02d:%02d",
          UserStartTime[0], UserStartTime[1], UserStartTime[2]);
  XmTextFieldSetString(timeText[0], buffer);

  sprintf(buffer, "%02d:%02d:%02d",
          UserEndTime[0], UserEndTime[1], UserEndTime[2]);
  XmTextFieldSetString(timeText[1], buffer);

}	/* END SETTIMETEXT */

/* -------------------------------------------------------------------- */
static void ApplyTimeChange(Widget w, XtPointer client, XtPointer call)
{
  char *p;

  if (RealTime)
    {
    p = XmTextFieldGetString(rtText);
    NumberSeconds = atoi(p);
    XtFree(p);
    }
  else
    {
    if ((unsigned long)client == 0xFFFF)	// Load whole flight
      {
      int	*l = dataFile[CurrentDataFile].FileStartTime;

      sprintf(buffer, "%02d:%02d:%02d", l[0], l[1], l[2]);
      XmTextFieldSetString(timeText[0], buffer);

      l = dataFile[CurrentDataFile].FileEndTime;
      sprintf(buffer, "%02d:%02d:%02d", l[0], l[1], l[2]);
      XmTextFieldSetString(timeText[1], buffer);
      }

    p = XmTextFieldGetString(timeText[0]);
    sscanf(p, "%2d:%2d:%2d",
            &UserStartTime[0], &UserStartTime[1], &UserStartTime[2]);
    XtFree(p);

    p = XmTextFieldGetString(timeText[1]);
    sscanf(p, "%2d:%2d:%2d", &UserEndTime[0], &UserEndTime[1], &UserEndTime[2]);
    XtFree(p);

    UserStartTime[3] = SecondsSinceMidnite(UserStartTime);
    UserEndTime[3] = SecondsSinceMidnite(UserEndTime);

    if (UserEndTime[3] == UserStartTime[3])
      UserEndTime[3] = UserStartTime[3] + 1;

    if (UserEndTime[3] < UserStartTime[3])
      UserEndTime[3] += 86400;
    }

  ReadData();
  DrawMainWindow();

}	/* END APPLYTIMECHANGE */

/* -------------------------------------------------------------------- */
void ChangePlotType(Widget w, XtPointer client, XtPointer call)
{
  int	n;
  Arg	args[5];

  if (call && ((XmToggleButtonCallbackStruct *)call)->set == False)
    return;

  ClearAnnotations();

  n = 0;
  switch ( (PlotType = (long)client) )
    {
    case TIME_SERIES:
      XtSetArg(args[n], XmNwidth, mainPlot[0].x.windowWidth); ++n;
      XtSetArg(args[n], XmNheight, mainPlot[0].x.windowHeight); ++n;
      XtSetValues(mainPlot[0].canvas, args, n);

      SetActivePanels(NumberOfPanels);
      XmToggleButtonSetState(whichAxis[1], True, True);

      if (NumberOfPanels == 1)
        Statistics = True;
      else
        Statistics = False;

      if (NumberOfPanels > 2)
        SetPrinterShape(PORTRAIT);
      else
        SetPrinterShape(LANDSCAPE);

      XtSetSensitive(RC[3], False);
      XtSetSensitive(RC[5], True);
      XtSetSensitive(RC[6], True);

      XtSetSensitive(optButton[1], True);

      break;

    case XY_PLOT:
      XtSetArg(args[n], XmNwidth, xyyPlot[0].x.windowWidth); ++n;
      XtSetArg(args[n], XmNheight, xyyPlot[0].x.windowHeight); ++n;
      XtSetValues(xyyPlot[0].canvas, args, n);

      SetActivePanels(NumberOfXYpanels);
      XmToggleButtonSetState(whichAxis[0], True, True);

      if (NumberOfXYpanels > 1)
        {
        SetPrinterShape(LANDSCAPE);
        Statistics = False;
        }
      else
        {
        SetPrinterShape(PORTRAIT);
        Statistics = True;
        }

      XtSetSensitive(RC[3], True);
      XtSetSensitive(RC[5], True);
      XtSetSensitive(RC[6], True);
      XtSetSensitive(whichAxis[2], False);

      XtSetSensitive(optButton[1], True);

      break;

    case XYZ_PLOT:
      XtSetArg(args[n], XmNwidth, xyzPlot.x.windowWidth); ++n;
      XtSetArg(args[n], XmNheight, xyzPlot.x.windowHeight); ++n;
      XtSetValues(xyzPlot.canvas, args, n);

      XmToggleButtonSetState(whichAxis[0], True, True);

      SetPrinterShape(LANDSCAPE);

      XtSetSensitive(RC[3], True);
      XtSetSensitive(RC[5], False);
      XtSetSensitive(RC[6], False);
      XtSetSensitive(whichAxis[2], True);

      XtSetSensitive(optButton[1], False);

      break;
    }

  SetInvertLogScale(NULL, NULL, NULL);
  DrawMainWindow();
  TrackOptWinControl();

}	/* END CHANGEPLOTTYPE */

/* -------------------------------------------------------------------- */
static void changeAxis(Widget w, XtPointer client, XtPointer call)
{
  if (!((XmToggleButtonCallbackStruct *)call)->set)
    return;

  currentAxis = (long)client;

  if (currentAxis == Y_AXIS && PlotType != XYZ_PLOT)
    {
    XtSetSensitive(RC[4], True);
    }
  else
    {
    XtSetSensitive(RC[4], False);
    }

  SetInvertLogScale(NULL, NULL, NULL);

}	/* END CHANGEAXIS */

/* -------------------------------------------------------------------- */
int whichSide()
{
  if (XmToggleButtonGetState(scaleLocation[0]) == True)
    return(LEFT_SIDE);
  else
    return(RIGHT_SIDE);
}

/* -------------------------------------------------------------------- */
int choosingXaxis()
{
  return(currentAxis == X_AXIS);
}

/* -------------------------------------------------------------------- */
int choosingYaxis()
{
  return(currentAxis == Y_AXIS);
}

/* -------------------------------------------------------------------- */
int choosingZaxis()
{
  return(currentAxis == Z_AXIS);
}

/* -------------------------------------------------------------------- */
void SetInvertLogScale(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
  switch (PlotType)
    {
    case TIME_SERIES:
      XmToggleButtonSetState(invert,
	mainPlot[CurrentPanel].Yaxis[whichSide()].invertAxis, False);
      XmToggleButtonSetState(logScale,
	mainPlot[CurrentPanel].Yaxis[whichSide()].logScale, False);
      break;

    case XY_PLOT:
      if (choosingXaxis())
        {
        XmToggleButtonSetState(invert, xyyPlot[CurrentPanel].Xaxis.invertAxis, False);
        XmToggleButtonSetState(logScale, xyyPlot[CurrentPanel].Xaxis.logScale, False);
        }
      else
        {
        XmToggleButtonSetState(invert,
		xyyPlot[CurrentPanel].Yaxis[whichSide()].invertAxis, False);
        XmToggleButtonSetState(logScale,
		xyyPlot[CurrentPanel].Yaxis[whichSide()].logScale, False);
        }
      break;

    case XYZ_PLOT:

      break;
    }

  DrawMainWindow();

}	/* END APPLYINVERT */

/* -------------------------------------------------------------------- */
void ApplyInvert(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
  switch (PlotType)
    {
    case TIME_SERIES:
      mainPlot[CurrentPanel].Yaxis[whichSide()].invertAxis =
		XmToggleButtonGetState(invert);
      break;

    case XY_PLOT:
      if (choosingXaxis())
        xyyPlot[CurrentPanel].Xaxis.invertAxis = XmToggleButtonGetState(invert);
      else
        xyyPlot[CurrentPanel].Yaxis[whichSide()].invertAxis =
		XmToggleButtonGetState(invert);
      break;

    case XYZ_PLOT:

      break;
    }

  DrawMainWindow();

}	/* END APPLYINVERT */

/* -------------------------------------------------------------------- */
void ApplyLogScale(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
  switch (PlotType)
    {
    case TIME_SERIES:
      mainPlot[CurrentPanel].Yaxis[whichSide()].logScale =
		XmToggleButtonGetState(logScale);
      break;

    case XY_PLOT:
      if (choosingXaxis())
        xyyPlot[CurrentPanel].Xaxis.logScale = XmToggleButtonGetState(logScale);
      else
        xyyPlot[CurrentPanel].Yaxis[whichSide()].logScale =
		XmToggleButtonGetState(logScale);
      break;

    case XYZ_PLOT:

      break;
    }

  DrawMainWindow();

}	/* END APPLYLOGSCALE */

/* -------------------------------------------------------------------- */
void SetDataFile(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
  if (call == NULL || call->set)
    {
    CurrentDataFile = (long)client;
    SetList();
    }
 
}   /* END SETDATAFILE */

/* -------------------------------------------------------------------- */
void CreateControlWindow(Widget parent)
{
  Widget	plRC[8], label, b[8], controlRC;
  Widget	form, title[9], brc, bkd, fwd;
  Arg		args[8];
  Cardinal	n;

  n = 0;
  ControlWindow = XmCreateForm(parent, "controlForm", args, n);

  /* Page Fwd & Bkwd buttons, and variable list.
   */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  form = XmCreateForm(ControlWindow, "form", args, n);
  XtManageChild(form);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  frame[0] = XmCreateFrame(form, "buttonFrame", args, n);
  XtManageChild(frame[0]);

  n = 0;
  brc = XmCreateRowColumn(frame[0], "pgButtRC", args, n);
  XtManageChild(brc);

  n = 0;
  bkd = XmCreatePushButton(brc, "pageBkd", args, n);
  XtAddCallback(bkd, XmNactivateCallback, PageBackward, NULL);
  XtManageChild(bkd);

  n = 0;
  fwd = XmCreatePushButton(brc, "pageFwd", args, n);
  XtAddCallback(fwd, XmNactivateCallback, PageForward, NULL);
  XtManageChild(fwd);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, frame[0]); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  varList = XmCreateScrolledList(form, "varList", args, n);
  XtAddCallback(varList, XmNbrowseSelectionCallback, ModifyActiveVars, NULL);
  XtManageChild(varList);


  /* Options RC.
   */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, form); n++;
  controlRC = XmCreateRowColumn(ControlWindow, "controlRC", args, n);
  XtManageChild(controlRC);

  n = 0;
  frame[0] = XmCreateFrame(controlRC, "timeFrame", args, 0);
  frame[1] = XmCreateFrame(controlRC, "fileFrame", args, 0);
  frame[2] = XmCreateFrame(controlRC, "plotFrame", args, 0);
  frame[3] = XmCreateFrame(controlRC, "axisFrame", args, 0);
  frame[4] = XmCreateFrame(controlRC, "panelFrame", args, 0);
  frame[5] = XmCreateFrame(controlRC, "trackFrame", args, 0);
  XtManageChildren(frame, 6);

  n = 0;
  title[0] = XmCreateLabel(frame[0], "timeTitle", args, 0);
  title[1] = XmCreateLabel(frame[1], "fileTitle", args, 0);
  title[2] = XmCreateLabel(frame[2], "plotTitle", args, 0);
  title[3] = XmCreateLabel(frame[3], "axisTitle", args, 0);
  title[4] = XmCreateLabel(frame[4], "panelTitle", args, 0);
  title[5] = XmCreateLabel(frame[5], "trackTitle", args, 0);
  XtManageChild(title[0]); XtManageChild(title[1]);
  XtManageChild(title[2]); XtManageChild(title[3]);
  XtManageChild(title[4]); XtManageChild(title[5]);

  n = 0;
  RC[0] = XmCreateRowColumn(frame[0], "plRC", args, n);
  RC[2] = XmCreateRadioBox(frame[2], "plotRC", args, 0);
  plRC[0] = XmCreateRowColumn(frame[3], "plRC", args, n);
  RC[3] = XmCreateRadioBox(plRC[0], "axisRC", args, 0);
  RC[4] = XmCreateRadioBox(plRC[0], "scaleRC", args, 0);
  RC[5] = XmCreateRowColumn(plRC[0], "axoptRC", args, 0);
  RC[6] = XmCreateRowColumn(frame[4], "panelRC", args, 0);
  RC[7] = XmCreateRowColumn(frame[5], "trackRC", args, 0);
  XtManageChild(plRC[0]); //XtManageChild(plRC[1]);
  XtManageChild(RC[0]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]); XtManageChild(RC[5]);
  XtManageChild(RC[6]); XtManageChild(RC[7]);


  /* Start & End Time widgets.
   */
  n = 0;
  timeText[0] = XmCreateTextField(RC[0], "timeText", args, n);
  timeText[1] = XmCreateTextField(RC[0], "timeText", args, n);
  XtManageChildren(timeText, 2);
  XtAddCallback(timeText[0], XmNlosingFocusCallback, ValidateTime, NULL);
  XtAddCallback(timeText[1], XmNlosingFocusCallback, ValidateTime, NULL);

  n = 0;
  b[0] = XmCreatePushButton(RC[0], "applyButton", args, n);
  b[1] = XmCreatePushButton(RC[0], "All", args, n);
  XtManageChildren(b, 2);
  XtAddCallback(b[0], XmNactivateCallback, ApplyTimeChange, NULL);
  XtAddCallback(b[1], XmNactivateCallback, ApplyTimeChange, (XtPointer)0xFFFF);


  if (RealTime)
    {
    n = 0;
    RC[1] = XmCreateRowColumn(frame[1], "plRC", args, n);
    XtManageChild(RC[1]);

    n = 0;
    realTime = XmCreateToggleButton(RC[1], "Realtime", args, n);
    XtManageChild(realTime);
    XmToggleButtonSetState(realTime, True, False);
    XtSetSensitive(RC[0], False);

    n = 0;
    rtText = XmCreateTextField(RC[1], "rtText", args, n);
    XtManageChild(rtText);

    n = 0;
    b[0] = XmCreatePushButton(RC[1], "applyButton", args, n);
    XtManageChild(b[0]);
    XtAddCallback(b[0], XmNactivateCallback, ApplyTimeChange, NULL);
    }
  else
    {
    RC[1] = XmCreateRadioBox(frame[1], "fileRC", args, 0);
    XtManageChild(RC[1]);

    /* File Toggle Buttons.
     */
    for (size_t i = 0; i < MAX_DATAFILES; ++i)
      {
      n = 0;
      fileB.push_back(XmCreateToggleButton(RC[1], "none", NULL, 0));
      XtAddCallback(fileB[i], XmNvalueChangedCallback,
                    (XtCallbackProc)SetDataFile, (XtPointer)i);
      }

    XtManageChildren(&fileB[0], MAX_DATAFILES);

    if (NumberDataFiles > 0)
      XmToggleButtonSetState(fileB[0], True, False);
    }


  /* Plot, axis, and scale stuff.
   */
  n = 0;
  plotType[0] = XmCreateToggleButton(RC[2], "Time Series", NULL, 0);
  plotType[1] = XmCreateToggleButton(RC[2], "XY plot", NULL, 0);
  plotType[2] = XmCreateToggleButton(RC[2], "XYZ plot", NULL, 0);

  XtAddCallback(plotType[0], XmNvalueChangedCallback,
    (XtCallbackProc)ChangePlotType, (XtPointer)TIME_SERIES);
  XtAddCallback(plotType[1], XmNvalueChangedCallback,
    (XtCallbackProc)ChangePlotType, (XtPointer)XY_PLOT);
  XtAddCallback(plotType[2], XmNvalueChangedCallback,
    (XtCallbackProc)ChangePlotType, (XtPointer)XYZ_PLOT);

  XtManageChildren(plotType, 3);
  XmToggleButtonSetState(plotType[0], True, False);


  whichAxis[0] = XmCreateToggleButton(RC[3], "X axis  ", NULL, 0);
  whichAxis[1] = XmCreateToggleButton(RC[3], "Y axis  ", NULL, 0);
  whichAxis[2] = XmCreateToggleButton(RC[3], "Z axis  ", NULL, 0);
  XtManageChildren(whichAxis, 3);

  XtSetSensitive(RC[3], False);
  XtAddCallback(whichAxis[0], XmNvalueChangedCallback, changeAxis, (XtPointer)X_AXIS);
  XtAddCallback(whichAxis[1], XmNvalueChangedCallback, changeAxis, (XtPointer)Y_AXIS);
  XtAddCallback(whichAxis[2], XmNvalueChangedCallback, changeAxis, (XtPointer)Z_AXIS);
  XmToggleButtonSetState(whichAxis[1], True, False);


  scaleLocation[0] = XmCreateToggleButton(RC[4], "Left", NULL, 0);
  scaleLocation[1] = XmCreateToggleButton(RC[4], "Right", NULL, 0);
  XtManageChildren(scaleLocation, 2);

  XtAddCallback(scaleLocation[0], XmNvalueChangedCallback, (XtCallbackProc)SetInvertLogScale, NULL);
  XtAddCallback(scaleLocation[1], XmNvalueChangedCallback, (XtCallbackProc)SetInvertLogScale, NULL);
  XmToggleButtonSetState(scaleLocation[0], True, False);

  invert = XmCreateToggleButton(RC[5], "Invert", NULL, 0);
  logScale = XmCreateToggleButton(RC[5], "Log Scale", NULL, 0);
  XtManageChild(invert); XtManageChild(logScale);
  XtAddCallback(invert, XmNvalueChangedCallback, (XtCallbackProc)ApplyInvert, NULL);
  XtAddCallback(logScale, XmNvalueChangedCallback, (XtCallbackProc)ApplyLogScale, NULL);

  /* Panel stuff.
   */
  n = 0;
  plRC[0] = XmCreateRowColumn(RC[6], "plRC", args, n);
  plRC[1] = XmCreateRowColumn(RC[6], "plRC", args, n);
  plRC[2] = XmCreateRowColumn(RC[6], "plRC", args, n);
  XtManageChildren(plRC, 3);

  label = XmCreateLabel(plRC[0], "Panel", args, n);
  XtManageChild(label);

  plRC[4] = XmCreateRadioBox(plRC[0], "pnRC", args, n);
  XtManageChild(plRC[4]);

  for (size_t i = 0; i < MAX_PANELS; ++i)
    {
    sprintf(buffer, "%ld", i+1);
    panelB.push_back(XmCreateToggleButton(plRC[4], buffer, NULL, 0));
    
    XtAddCallback(panelB[i], XmNvalueChangedCallback,
                  (XtCallbackProc)SetCurrentPanel, (XtPointer)i);
    }

  XtManageChildren(&panelB[0], MAX_PANELS);
  XmToggleButtonSetState(panelB[0], True, False);

  n = 0;
  b[0] = XmCreatePushButton(plRC[1], "Add", args, n);
  b[1] = XmCreatePushButton(plRC[1], "Delete", args, n);
  b[2] = XmCreatePushButton(plRC[1], "Clear", args, n);
  XtManageChildren(b, 3);
  XtAddCallback(b[0], XmNactivateCallback, AddPanel, NULL);
  XtAddCallback(b[1], XmNactivateCallback, DeletePanel, NULL);
  XtAddCallback(b[2], XmNactivateCallback, ClearPanel, NULL);


  b[3] = XmCreateToggleButton(plRC[2], "Labels on all multi-panels", args, n);
  XtManageChild(b[3]);
  XtAddCallback(b[3], XmNvalueChangedCallback, ToggleLabels, NULL);
  XmToggleButtonSetState(b[3], True, False);


  /* Options.
  */
  n = 0;
  optButton[0] = XmCreateToggleButton(RC[7], "Black & White (on/off)", args, n);
  optButton[1] = XmCreateToggleButton(RC[7], "UT seconds (on/off)", args, n);
  optButton[2] = XmCreateToggleButton(RC[7], "Grid (on/off)", args, n);
  optButton[3] = XmCreateToggleButton(RC[7], "Cross-hair (on/off)", args, n);
  XtManageChildren(optButton, 4);

  plRC[0] = XmCreateRowColumn(RC[7], "pnRC", args, n); XtManageChild(plRC[0]);
  lineThickTxt = XmCreateTextField(plRC[0], "lineThick", args, n);
  XtManageChild(lineThickTxt);
  label = XmCreateLabel(plRC[0], "Line Thickness", args, n);
  XtManageChild(label);

  sprintf(buffer, "%ld", LineThickness);
  XmTextFieldSetString(lineThickTxt, buffer);


  XtAddCallback(optButton[0], XmNvalueChangedCallback, ToggleColor, NULL);
  XtAddCallback(optButton[1], XmNvalueChangedCallback, ToggleUTC, NULL);
  XtAddCallback(optButton[2], XmNvalueChangedCallback, ToggleGrid, NULL);
  XtAddCallback(optButton[3], XmNvalueChangedCallback, ToggleTracking, NULL);
  XtAddCallback(lineThickTxt, XmNvalueChangedCallback, ChangeLineThickness, lineThickTxt);

  /* Check positioning and keep it on the screen (laptops).
   */
  Position controlY, controlX;
  const Dimension controlWidth = 400;
  n = 0;
  XtSetArg(args[n], XtNx, &controlX); ++n;
  XtSetArg(args[n], XtNy, &controlY); ++n;
  XtGetValues(parent, args, n);

  int screenWidth = WidthOfScreen(XtScreen(parent));

  if (controlX + controlWidth > screenWidth)
  {
    controlX = screenWidth - controlWidth;
    n = 0;
    XtSetArg(args[n], XmNx, controlX); ++n;
    XtSetValues(parent, args, n);
  }

}	/* END CREATECONTROLWINDOW */

/* END CONTROL.C */
