/*
-------------------------------------------------------------------------
OBJECT NAME:	diff.c

FULL NAME:	Differences

ENTRY POINTS:	ComputeDiff()
		DiffWinUp()
		DiffWinDown()

STATIC FNS:	CreateDifferenceWindow()

DESCRIPTION:	

REFERENCES:	XplotDiff.c

REFERENCED BY:	Callback

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>

extern Widget	AppShell, DiffShell, DifferenceWindow;

static void	CreateDifferenceWindow();

float scanit(char *s);

/* -------------------------------------------------------------------- */
void ComputeDiff()
{
  bool	saveState = Freeze;
  int	i, in0, in1;
  char	myExpression[512];

  if (diffSet.data)
    {
    free((char *)diffSet.data);
    diffSet.data = NULL;
    }

  if  (NumberDataSets < 2)
    return;

  Freeze = True;

  diffSet.nPoints = dataSet[0].nPoints;
  diffSet.fileIndex = dataSet[0].fileIndex;
  diffSet.varInfo = dataSet[0].varInfo;
  diffSet.missingValue = dataSet[0].missingValue;
  diffSet.data = (float *)GetMemory(sizeof(NR_TYPE) * dataSet[0].nPoints);

  sprintf(diffPlot.Yaxis[0].label, "(%s-%s) %s", dataSet[0].varInfo->name,
            dataSet[1].varInfo->name, mainPlot[0].Yaxis[0].label);

  for (i = 0; i < dataSet[0].nPoints; ++i)
    {
    in0 = (dataSet[0].head + i) % dataSet[0].nPoints;
    in1 = (dataSet[1].head + i) % dataSet[1].nPoints;

    if (isMissingValue(dataSet[0].data[in0], dataSet[0].missingValue) ||
        isMissingValue(dataSet[1].data[in1], dataSet[1].missingValue))
      diffSet.data[i] = dataSet[0].missingValue;
    else
      diffSet.data[i] = dataSet[0].data[in0] - dataSet[1].data[in1];
    }

  diffSet.stats.outlierMin = -FLT_MAX;
  diffSet.stats.outlierMax = FLT_MAX;

  ComputeStats(&diffSet);
  AutoScaleDiff();

  Freeze = saveState;

}	/* END COMPUTEDIFF */

/* -------------------------------------------------------------------- */
void DiffWinDown(Widget w, XtPointer client, XtPointer call)
{
  diffPlot.windowOpen = False;
  XtUnmanageChild(DifferenceWindow);
  XtPopdown(XtParent(DifferenceWindow));

}	/* END DIFFWINDOWN */

/* -------------------------------------------------------------------- */
void DiffWinUp(Widget w, XtPointer client, XtPointer call)
{
  static int firstTime = True;

  if (NumberDataSets < 2)
    {
    HandleError("Two data sets required for difference.", Interactive, IRET);
    return;
    }

  if (dataSet[0].nPoints != dataSet[1].nPoints)
    {
    HandleError("Matching data rates required for difference.",
                Interactive, IRET);
    return;
    }


  if (firstTime)
    {
    CreateDifferenceWindow();
    initPlotGC(&diffPlot);

    diffSet.scaleLocation = LEFT_SIDE;
    diffSet.head = 0;
    diffSet.data = NULL;
    }

  ComputeDiff();
  diffPlot.windowOpen = True;
  XtManageChild(DifferenceWindow);
  XtPopup(XtParent(DifferenceWindow), XtGrabNone);

  if (firstTime)
    {
    ResizeDiffWindow(NULL, NULL, NULL);

    XtAddCallback(diffPlot.canvas, XmNexposeCallback,
                (XtCallbackProc)PlotDifference, (XtPointer)NULL);
    XtAddCallback(diffPlot.canvas, XmNresizeCallback, ResizeDiffWindow, NULL);
    XtAddCallback(diffPlot.canvas, XmNresizeCallback,
                (XtCallbackProc)PlotDifference, (XtPointer)NULL);

    firstTime = False;
    }

}	/* END DIFFWINUP */

/* -------------------------------------------------------------------- */
static void CreateDifferenceWindow()
{
  Widget	frame, rc, b[4];
  Cardinal	n;
  Arg		args[8];

  n = 0;
  DiffShell = XtCreatePopupShell("diffShell", topLevelShellWidgetClass,
                      AppShell, args, n);

  n = 0;
  DifferenceWindow = XmCreateForm(DiffShell, "diffForm", args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  frame = XmCreateFrame(DifferenceWindow, "buttonFrame", args, n);
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
  XtAddCallback(b[0], XmNactivateCallback, DiffWinDown, NULL);
  XtAddCallback(b[1], XmNactivateCallback, diffPostScript, NULL);
  XtAddCallback(b[2], XmNactivateCallback, EditDiffParms, NULL);
#ifdef PNG
  XtAddCallback(b[3], XmNactivateCallback, SavePNGdiff, NULL);
#endif


  /* Create Graphics Canvas
   */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, frame); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  diffPlot.canvas = XmCreateDrawingArea(DifferenceWindow,"diffCanvas",args,n);
  XtManageChild(diffPlot.canvas);

}	/* END CREATEDIFFERENCEWINDOW */

/* END DIFF.C */
