/*
-------------------------------------------------------------------------
OBJECT NAME:	ccb.c

FULL NAME:	Misc X menu callbacks

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

COPYRIGHT:	University Corporation for Atmospheric Research, 1994-2007
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
  QueryFile("Enter Data file to read:", const_cast<char *>(DataPath.c_str()),
	(XtCallbackProc)client);
}

/* -------------------------------------------------------------------- */
void ToggleProject(Widget w, XtPointer client, XtPointer call)
{
  if ((long)client)
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
  size_t i;

  allLabels = !allLabels;

  for (i = 0; i < NumberOfPanels-1; ++i)
    if (allLabels)
      mainPlot[i].Xaxis.label = mainPlot[NumberOfPanels-1].Xaxis.label;
    else
      mainPlot[i].Xaxis.label.clear();

  for (i = 1; i < NumberOfXYpanels; ++i)
    if (allLabels)
      {
      xyyPlot[i].Yaxis[0].label = xyyPlot[0].Yaxis[0].label;
      xyyPlot[i].Yaxis[0].label = xyyPlot[0].Yaxis[0].label;
      }
    else
      {
      xyyPlot[i].Yaxis[0].label.clear();
      xyyPlot[i].Yaxis[0].label.clear();
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
  XmToggleButtonCallbackStruct *cb = (XmToggleButtonCallbackStruct *)call;

  for (size_t i = 0; i < MAX_PANELS; ++i)
    mainPlot[i].grid = xyyPlot[i].grid = cb->set;

  xyzPlot.grid = False;		/* Grid not supported for XYZ */

  if (Interactive)
    DrawMainWindow();

}	/* END TOGGLEGRID */

/* -------------------------------------------------------------------- */
void ToggleTracking(Widget w, XtPointer client, XtPointer call)
{
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
  char *selectedVariable;
  int  position = ((XmListCallbackStruct *)call)->item_position - 1;
  XmString item = ((XmListCallbackStruct *)call)->item;
  XmStringGetLtoR(item, XmSTRING_DEFAULT_CHARSET, &selectedVariable);

  DataChanged = True;

  switch (PlotType)
    {
    case TIME_SERIES:
      /* Try to delete variable first, if no match, then add it.
       */
      if (DeleteVariable(dataSet, NumberDataSets, selectedVariable) == False)
        {
        if (NumberDataSets == MAX_DATASETS)
          {
          ShowError("Out of data sets.");
          return;
          }

        AddVariable(&dataSet[NumberDataSets++], selectedVariable);
        }
      else
        --NumberDataSets;

      SetYlabels(mainPlot, dataSet, NumberDataSets);
      break;

    case XY_PLOT:
      if (choosingXaxis())
        {
        if (DeleteVariable(xyXset, NumberXYXsets, selectedVariable) == False)
          {
          if (NumberXYXsets == MAX_DATASETS)
            {
            ShowError("Out of data sets.");
            return;
            }

          AddVariable(&xyXset[NumberXYXsets++], selectedVariable);
          }
        else
          --NumberXYXsets;

        SetXlabels(xyyPlot, xyXset, NumberXYXsets);
        }
      else
        {
        if (DeleteVariable(xyYset, NumberXYYsets, selectedVariable) == False)
          {
          if (NumberXYYsets == MAX_DATASETS)
            {
            ShowError("Out of data sets.");
            return;
            }

          AddVariable(&xyYset[NumberXYYsets++], selectedVariable);
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
          delete [] xyzSet[0].data;

        AddVariable(&xyzSet[0], selectedVariable);
        sprintf(buffer, "%s (%s)",
		xyzSet[0].varInfo->name.c_str(), xyzSet[0].stats.units.c_str());
        xyzPlot.Xaxis.label = buffer;
        }

      if (choosingYaxis())
        {
        if (xyzSet[2].varInfo)
          delete [] xyzSet[2].data;

        AddVariable(&xyzSet[2], selectedVariable);
        sprintf(buffer, "%s (%s)",
		xyzSet[2].varInfo->name.c_str(), xyzSet[2].stats.units.c_str());
        xyzPlot.Zaxis.label = buffer;
        }

      if (choosingZaxis())
        {
        if (xyzSet[1].varInfo)
          delete [] xyzSet[1].data;

        AddVariable(&xyzSet[1], selectedVariable);
        sprintf(buffer, "%s (%s)",
                xyzSet[1].varInfo->name.c_str(), xyzSet[1].stats.units.c_str());
        xyzPlot.Yaxis[0].label = buffer;
        }

      break;
    }

  XtFree(selectedVariable);
  findMinMax();
  DrawMainWindow();

}	/* END MODIFYACTIVEVARS */

/* -------------------------------------------------------------------- */
void ClearPlot(Widget w, XtPointer client, XtPointer call)
{
  size_t i, set;
 
  ClearAnnotations();

  switch (PlotType)
    {
    case TIME_SERIES:
      for (set = 0; set < NumberDataSets; ++set)
        {
        dataSet[set].varInfo = NULL;
        dataSet[set].nPoints = 0;
        delete [] dataSet[set].data;
        }

      NumberDataSets = 0;
      SetYlabels(mainPlot, dataSet, NumberDataSets);
      break;
 
    case XY_PLOT:
      for (i = 0; i < MAX_PANELS; ++i)
        {
        xyyPlot[i].Xaxis.label.clear();
        xyyPlot[i].Yaxis[0].label.clear();
        xyyPlot[i].Yaxis[1].label.clear();
        }

      for (set = 0; set < NumberXYXsets; ++set)
        if (xyXset[set].varInfo)
          {
          xyXset[set].varInfo = NULL;
          xyXset[set].nPoints = 0;
          delete [] xyXset[set].data;
          }

      for (set = 0; set < NumberXYYsets; ++set)
        if (xyYset[set].varInfo)
          {
          xyYset[set].varInfo = NULL;
          xyYset[set].nPoints = 0;
          delete [] xyYset[set].data;
          }

      ShowRegression = 0;
      NumberXYXsets = NumberXYYsets = 0;
      SetXlabels(xyyPlot, xyXset, NumberXYXsets);
      SetYlabels(xyyPlot, xyYset, NumberXYYsets);
      break;
 
    case XYZ_PLOT:
      xyzPlot.Xaxis.label.clear();
      xyzPlot.Yaxis[0].label.clear();
      xyzPlot.Yaxis[1].label.clear();

      for (set = 0; set < 3; ++set)
        if (xyzSet[set].varInfo)
          {
          xyzSet[set].varInfo = NULL;
          xyzSet[set].nPoints = 0;
          delete [] xyzSet[set].data;
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
    switch ((long)client)
      {
      case 1:
        execlp("firefox", "firefox", "http://www.eol.ucar.edu/raf", NULL);
        break;
      case 2:
        execlp("firefox", "firefox", "http://www.eol.ucar.edu/raf/Software", NULL);
        break;
      case 3:
        execlp("firefox", "firefox", "http://www.eol.ucar.edu/raf/Software/ncplot.html", NULL);
        break;
      }

    printf("exec of firefox failed, errno = %d\n", errno);
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
