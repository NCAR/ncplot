/*
-------------------------------------------------------------------------
OBJECT NAME:	ccb.c

FULL NAME:	Misc menu callbacks

ENTRY POINTS:	ChangeLineThickness()
		ClearPlot()
		ClearRegression()
		DismissWindow()
		findMinMax()
		ForkNetscape()
		GetDataFileName()
		LinearRegression()
		ModifyActiveVars()
		PolyRegression2()
		PolyRegression()
		Quit()
		ToggleProject()
		ToggleColor()
		ToggleUTC()
		ToggleScatter()
		ToggleGrid()
		ToggleTracking()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1994-2001
-------------------------------------------------------------------------
*/

#include "define.h"
#include <errno.h>
#include <unistd.h>

#include <Xm/List.h>
#include <Xm/TextF.h>


void findMinMax();


/* -------------------------------------------------------------------- */
void GetDataFileName(Widget w, XtPointer client, XtPointer call)
{
  QueryFile("Enter Data file to read:", DataPath, (XtCallbackProc)client);

}

/* -------------------------------------------------------------------- */
void ToggleProject(Widget w, XtPointer client, XtPointer call)
{
  if ((int)client)
    ProjectToBack = !ProjectToBack;
  else
    ProjectToXY = !ProjectToXY;

  DrawMainWindow();

}	/* END TOGGLEPROJECT */

/* -------------------------------------------------------------------- */
void ToggleColor(Widget w, XtPointer client, XtPointer call)
{
  Color = !Color;

  DrawMainWindow();

}	/* END TOGGLECOLOR */

/* -------------------------------------------------------------------- */
void ToggleUTC(Widget w, XtPointer client, XtPointer call)
{
  UTCseconds = !UTCseconds;

  DrawMainWindow();

}	/* END TOGGLEUTC */

/* -------------------------------------------------------------------- */
void ToggleLabels(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  allLabels = !allLabels;

  for (i = 0; i < NumberOfPanels-1; ++i)
    if (allLabels)
      strcpy(mainPlot[i].Xaxis.label, mainPlot[NumberOfPanels-1].Xaxis.label);
    else
      mainPlot[i].Xaxis.label[0] = '\0';

  for (i = 1; i < NumberOfXYpanels; ++i)
    if (allLabels)
      {
      strcpy(xyyPlot[i].Yaxis[0].label, xyyPlot[0].Yaxis[0].label);
      strcpy(xyyPlot[i].Yaxis[0].label, xyyPlot[0].Yaxis[0].label);
      }
    else
      {
      xyyPlot[i].Yaxis[0].label[0] = '\0';
      xyyPlot[i].Yaxis[0].label[0] = '\0';
      }

  if (Interactive)
    DrawMainWindow();

}	/* END TOGGLESCATTER */

/* -------------------------------------------------------------------- */
void ToggleScatter(Widget w, XtPointer client, XtPointer call)
{
  ScatterPlot = !ScatterPlot;

  if (Interactive)
    DrawMainWindow();

}	/* END TOGGLESCATTER */
 
/* -------------------------------------------------------------------- */
void ToggleGrid(Widget w, XtPointer client, XtPointer call)
{
  int	i;
  XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *)call;

  for (i = 0; i < MAX_PANELS; ++i)
    mainPlot[i].grid = xyyPlot[i].grid = cb->set;

  xyzPlot.grid = False;		/* Grid not supported for XYZ */

  if (Interactive)
    DrawMainWindow();

}	/* END TOGGLEGRID */

/* -------------------------------------------------------------------- */
void ToggleTracking(Widget w, XtPointer client, XtPointer call)
{
  int	i;
  XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *)call;

  if (cb->set)
    XtAddEventHandler(mainPlot[0].canvas, PointerMotionMask, False,
                (XtEventHandler)CanvasMotion, NULL);
  else
    XtRemoveEventHandler(mainPlot[0].canvas, PointerMotionMask, False,
                (XtEventHandler)CanvasMotion, NULL);

}	/* END TOGGLETRACKING */

/* -------------------------------------------------------------------- */
void ModifyActiveVars(Widget w, XtPointer client, XtPointer call)
{
  int	position;

  position = ((XmListCallbackStruct *)call)->item_position - 1;

  DataChanged = True;

  switch (PlotType)
    {
    case TIME_SERIES:
      /* Try to delete variable first, if no match, then add it.
       */
      if (DeleteVariable(dataSet, NumberDataSets, position) == False)
        {
        if (NumberDataSets == MAX_DATASETS)
          {
          ShowError("Out of data sets.");
          return;
          }

        AddVariable(&dataSet[NumberDataSets++], position);
        }
      else
        --NumberDataSets;

      SetYlabels(mainPlot, dataSet, NumberDataSets);
      break;

    case XY_PLOT:
      if (choosingXaxis())
        {
        if (DeleteVariable(xyXset, NumberXYXsets, position) == False)
          {
          if (NumberXYXsets == MAX_DATASETS)
            {
            ShowError("Out of data sets.");
            return;
            }

          AddVariable(&xyXset[NumberXYXsets++], position);
          }
        else
          --NumberXYXsets;

        SetXlabels(xyyPlot, xyXset, NumberXYXsets);
        }
      else
        {
        if (DeleteVariable(xyYset, NumberXYYsets, position) == False)
          {
          if (NumberXYYsets == MAX_DATASETS)
            {
            ShowError("Out of data sets.");
            return;
            }

          AddVariable(&xyYset[NumberXYYsets++], position);
          }
        else
          --NumberXYYsets;

        SetYlabels(xyyPlot, xyYset, NumberXYYsets);
        }

      break;

    case XYZ_PLOT:
      if (choosingXaxis())
        {
        if (xyzSet[0].varInfo)
          free((char *)xyzSet[0].data);

        AddVariable(&xyzSet[0], position);
        sprintf(xyzPlot.Xaxis.label, "%s (%s)",
		xyzSet[0].varInfo->name, xyzSet[0].stats.units);
        }

      if (choosingYaxis())
        {
        if (xyzSet[2].varInfo)
          free((char *)xyzSet[2].data);

        AddVariable(&xyzSet[2], position);
        sprintf(xyzPlot.Zaxis.label, "%s (%s)",
		xyzSet[2].varInfo->name, xyzSet[2].stats.units);
        }

      if (choosingZaxis())
        {
        if (xyzSet[1].varInfo)
          free((char *)xyzSet[1].data);

        AddVariable(&xyzSet[1], position);
        sprintf(xyzPlot.Yaxis[0].label, "%s (%s)",
                xyzSet[1].varInfo->name, xyzSet[1].stats.units);
        }

      break;
    }

  findMinMax();
  DrawMainWindow();

}	/* END MODIFYACTIVEVARS */

/* -------------------------------------------------------------------- */
void ClearPlot(Widget w, XtPointer client, XtPointer call)
{
  int   i, set;
 
  ClearAnnotations();

  switch (PlotType)
    {
    case TIME_SERIES:
      for (set = 0; set < NumberDataSets; ++set)
        {
        dataSet[set].varInfo = NULL;
        dataSet[set].nPoints = 0;
        free((char *)dataSet[set].data);
        }

      NumberDataSets = 0;
      SetYlabels(mainPlot, dataSet, NumberDataSets);
      break;
 
    case XY_PLOT:
      for (i = 0; i < MAX_PANELS; ++i)
        xyyPlot[i].Xaxis.label[0] = xyyPlot[i].Yaxis[0].label[0] =
            xyyPlot[i].Yaxis[1].label[0] = '\0';

      for (set = 0; set < NumberXYXsets; ++set)
        if (xyXset[set].varInfo)
          {
          xyXset[set].varInfo = NULL;
          xyXset[set].nPoints = 0;
          free((char *)xyXset[set].data);
          }

      for (set = 0; set < NumberXYYsets; ++set)
        if (xyYset[set].varInfo)
          {
          xyYset[set].varInfo = NULL;
          xyYset[set].nPoints = 0;
          free((char *)xyYset[set].data);
          }

      ShowRegression = 0;
      NumberXYXsets = NumberXYYsets = 0;
      SetXlabels(xyyPlot, xyXset, NumberXYXsets);
      SetYlabels(xyyPlot, xyYset, NumberXYYsets);
      break;
 
    case XYZ_PLOT:
      xyzPlot.Xaxis.label[0] = xyzPlot.Yaxis[0].label[0] =
            xyzPlot.Yaxis[1].label[0] = '\0';

      for (set = 0; set < 3; ++set)
        if (xyzSet[set].varInfo)
          {
          xyzSet[set].varInfo = NULL;
          xyzSet[set].nPoints = 0;
          free((char *)xyzSet[set].data);
          }

      break;
    }

  DataChanged = True;
  DrawMainWindow();

}	/* END CLEARPLOT */

/* -------------------------------------------------------------------- */
void ClearRegression(Widget w, XtPointer client, XtPointer call)
{
  ShowRegression = 0;

  DrawMainWindow();

}	/* CLEARREGRESSION */
 
/* -------------------------------------------------------------------- */
void LinearRegression(Widget w, XtPointer client, XtPointer call)
{
  int nDegree = 1;

  if (NumberXYYsets < 1 || NumberXYXsets < 1)
    return;

  if (xyXset[0].nPoints != xyYset[0].nPoints)
    {
    HandleError("Matching data rates required for regression.",
                Interactive, IRET);
    return;
    }

  ShowRegression = 1;
  DrawMainWindow();

}	/* END LINEARREGRESSION */

/* -------------------------------------------------------------------- */
void PolyRegression2(Widget w, XtPointer client, XtPointer call)
{
  int	nDegree;

  ExtractAnswer(buffer);

  nDegree = atoi(buffer);

  if (nDegree < 2 || nDegree > 5)
    return;

  ShowRegression = nDegree;
  DrawMainWindow();
 
}	/* END POLYREGRESSION2 */

/* -------------------------------------------------------------------- */
void PolyRegression(Widget w, XtPointer client, XtPointer call)
{
  if (NumberXYYsets < 1 || NumberXYXsets < 1)
    return;

  if (xyXset[0].nPoints != xyYset[0].nPoints)
    {
    HandleError("Matching data rates required for regression.",
                Interactive, IRET);
    return;
    }

  QueryUser("Enter degree of polynomial (2-5):", 3, PolyRegression2);

}	/* END POLYREGRESSION */

/* -------------------------------------------------------------------- */
void DismissWindow(Widget w, XtPointer client, XtPointer call)
{
  XtPopdown(XtParent((Widget)client));
  XtUnmanageChild((Widget)client);

}	/* END DISMISSWINDOW */

/* -------------------------------------------------------------------- */
void ForkNetscape(Widget w, XtPointer client, XtPointer call)
{
  if (fork() == 0)
    {
    switch ((int)client)
      {
      case 1:
        execlp("netscape", "netscape", "http://raf.atd.ucar.edu", NULL);
        break;
      case 2:
        execlp("netscape", "netscape", "http://raf.atd.ucar.edu/Software", NULL);
        break;
      case 3:
        execlp("netscape", "netscape", "http://raf.atd.ucar.edu/Software/ncplot.html", NULL);
        break;
      }

    printf("exec of netscape failed, errno = %d\n", errno);
    exit(0);
    }

}	/* END FORKNETSCAPE */

/* -------------------------------------------------------------------- */
void ChangeLineThickness(Widget w, XtPointer client, XtPointer call)
{
  char *p = XmTextFieldGetString(w);

  if (atoi(p) > 0)
    LineThickness = atoi(p);

  DrawMainWindow();

}	/* END CHANGELINETHICKNESS */

/* -------------------------------------------------------------------- */
void Quit(Widget w, XtPointer client, XtPointer call)
{
  exit(0);

}	/* END QUIT */

/* END CCB.C */
