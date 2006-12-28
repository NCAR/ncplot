/*
-------------------------------------------------------------------------
OBJECT NAME:	panel.c

FULL NAME:	Callbacks for Panels from Control Window

ENTRY POINTS:	SetPlotShape()
		AddPanel()
		DeletePanel()
		ClearPanel()
		SetCurrentPanel()
		SetActivePanels()

STATIC FNS:	clearSet()

DESCRIPTION:	

MODIFIES:	CurrentDataFile, CurrentPanel, NumberOfPanels

COPYRIGHT:	University Corporation for Atmospheric Research, 1998
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

void    SetInvertLogScale(Widget, XtPointer, XmToggleButtonCallbackStruct *);

extern Widget	MainShell, MainWindow;
extern std::vector<Widget> panelB;

/* -------------------------------------------------------------------- */
void SetPlotShape(PLOT_INFO *plot, int shape)
{
  int   n;
  Arg   args[4];
 
  if (shape == PORTRAIT)
    {
    plot->x.windowWidth = 703;
    plot->x.windowHeight = 900;
 
    n = 0;
    XtSetArg(args[n], XmNx, 10); ++n;
    XtSetArg(args[n], XmNy, 30); ++n;
    XtSetValues(MainShell, args, n);
    }
  else
    {
    plot->x.windowWidth = 900;
    plot->x.windowHeight = 703;
 
    n = 0;
    XtSetArg(args[n], XmNx, 10); ++n;
    XtSetArg(args[n], XmNy, 50); ++n;
    XtSetValues(MainShell, args, n);
    }
 
  n = 0;
  XtSetArg(args[n], XmNwidth, plot->x.windowWidth); ++n;
  XtSetArg(args[n], XmNheight, plot->x.windowHeight); ++n;
  XtSetValues(plot->canvas, args, n);
 
  SetPrinterShape(shape);
 
}       /* END SETPLOTSHAPE */

/* -------------------------------------------------------------------- */
void SetCurrentPanel(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
  if (call == NULL || call->set)
    CurrentPanel = (int)client;

  SetInvertLogScale(NULL, NULL, NULL);

}	/* END SETCURRENTPANEL */

/* -------------------------------------------------------------------- */
void SetActivePanels(size_t n)
{
  size_t i;

  XmToggleButtonSetState(panelB[0], True, True);

  for (i = 0; i < n; ++i)
    XtSetSensitive(panelB[i], True);

  for (; i < MAX_PANELS; ++i)
    XtSetSensitive(panelB[i], False);

}

/* -------------------------------------------------------------------- */
static int clearSet(DATASET_INFO set[], size_t nSets, size_t panel)
{
  size_t i, cnt;

  for (i = cnt = 0; i < nSets; ++i)
    {
    set[cnt] = set[i];

    if (set[i].panelIndex == panel)
      {
      delete [] set[i].data;
      set[i].nPoints = 0;
      }
    else
      ++cnt;
    }

  return(cnt);

}	/* END CLEARSET */

/* -------------------------------------------------------------------- */
void ClearPanel(Widget w, XtPointer client, XtPointer call)
{
  if (PlotType == TIME_SERIES)
    {
    NumberDataSets = clearSet(dataSet, NumberDataSets, CurrentPanel);
    SetYlabels(mainPlot, dataSet, NumberDataSets);
    }
  else
  if (PlotType == XY_PLOT)
    {
    ShowRegression = 0;
    NumberXYXsets = clearSet(xyXset, NumberXYXsets, CurrentPanel);
    NumberXYYsets = clearSet(xyYset, NumberXYYsets, CurrentPanel);
    SetXlabels(xyyPlot, xyXset, NumberXYXsets);
    SetYlabels(xyyPlot, xyYset, NumberXYYsets);
    }

  DataChanged = True;
  DrawMainWindow();

}	/* END CLEARPANEL */

/* -------------------------------------------------------------------- */
void AddPanel(Widget w, XtPointer client, XtPointer call)
{
  size_t	i, nPanels = 0;
  PLOT_INFO	*plot = 0;

  switch (PlotType)
    {
    case TIME_SERIES:
      plot = mainPlot;
      nPanels = NumberOfPanels;
      break;

    case XY_PLOT:
      plot = xyyPlot;
      nPanels = NumberOfXYpanels;
      break;
    }

  if (nPanels == MAX_PANELS)
    {
    HandleError("Currently at maximum allowed panels.", Interactive, IRET);
    return;
    }

  ClearAnnotations();

  plot[nPanels].Yaxis[0].label = plot[nPanels].Yaxis[1].label = "";
  plot[nPanels].Xaxis.label = plot[nPanels-1].Xaxis.label;
  plot[nPanels].windowOpen = plot[nPanels].autoScale = plot[nPanels].autoTics = true;

  ++nPanels;

  if (nPanels > 1)
    Statistics = False;

  CurrentPanel = nPanels-1;
  XtSetSensitive(panelB[CurrentPanel], True);
  XmToggleButtonSetState(panelB[CurrentPanel], True, True);

  switch (PlotType)
    {
    case TIME_SERIES:
      if (!allLabels)
        plot[CurrentPanel-1].Xaxis.label = "";

      for (i = 0; i < nPanels; ++i)
        plot[i].Yaxis[0].nMajorTics =
        plot[i].Yaxis[1].nMajorTics = 10 - (nPanels * 2);

      if ((NumberOfPanels = nPanels) == 3)
        SetPlotShape(&mainPlot[0], PORTRAIT);
      else
        ResizeMainWindow(NULL, NULL, NULL);

      SetMainDefaults();
      break;

    case XY_PLOT:
      if (!allLabels)
        {
        plot[CurrentPanel].Yaxis[0].label = "";
        plot[CurrentPanel].Yaxis[1].label = "";
        }

      for (i = 0; i < nPanels; ++i)
        plot[i].Xaxis.nMajorTics = 10 - (nPanels * 2);

      if ((NumberOfXYpanels = nPanels) == 2)
        SetPlotShape(&xyyPlot[0], LANDSCAPE);
      else
        ResizeMainWindow(NULL, NULL, NULL);

      SetXYDefaults();
      break;
    }

}	/* END ADDPANEL */

/* -------------------------------------------------------------------- */
void DeletePanel(Widget w, XtPointer client, XtPointer call)
{
  size_t	i, nPanels = 0;
  PLOT_INFO	*plot = 0;

  switch (PlotType)
    {
    case TIME_SERIES:
      nPanels = NumberOfPanels;
      if (nPanels < 2)
        return;
      plot = mainPlot;
      plot[nPanels-2].Xaxis.label = plot[nPanels-1].Xaxis.label;
      NumberDataSets = clearSet(dataSet, NumberDataSets, CurrentPanel);

      for (i = 0; i < NumberDataSets; ++i)
        if (dataSet[i].panelIndex > CurrentPanel)
          --dataSet[i].panelIndex;

      break;

    case XY_PLOT:
      nPanels = NumberOfXYpanels;
      if (nPanels < 2)
        return;
      plot = xyyPlot;
      NumberXYXsets = clearSet(xyXset, NumberXYXsets, CurrentPanel);
      NumberXYYsets = clearSet(xyYset, NumberXYYsets, CurrentPanel);

      for (i = 0; i < NumberXYXsets; ++i)
        if (xyXset[i].panelIndex > CurrentPanel)
          --xyXset[i].panelIndex;

      for (i = 0; i < NumberXYYsets; ++i)
        if (xyYset[i].panelIndex > CurrentPanel)
          --xyYset[i].panelIndex;

      break;
    }

  ClearAnnotations();

  if (CurrentPanel == 0)
    {
    plot[1].title = plot[0].title;
    plot[1].subTitle = plot[0].subTitle;
    }

  --nPanels;
  plot[CurrentPanel].windowOpen = False;

  for (i = CurrentPanel; i < nPanels; ++i)
    plot[i] = plot[i+1];

  if (nPanels == 1)
    Statistics = True;

  XtSetSensitive(panelB[nPanels], False);

  if (CurrentPanel >= nPanels)
    {
    CurrentPanel = nPanels - 1;
    XmToggleButtonSetState(panelB[CurrentPanel], True, True);
    }


  switch (PlotType)
    {
    case TIME_SERIES:
      for (i = 0; i < nPanels; ++i)
        plot[i].Yaxis[0].nMajorTics =
        plot[i].Yaxis[1].nMajorTics = 10 - (nPanels * 2);

      if ((NumberOfPanels = nPanels) == 2)
        SetPlotShape(&mainPlot[0], LANDSCAPE);
      else
        ResizeMainWindow(NULL, NULL, NULL);

      break;

    case XY_PLOT:
      for (i = 0; i < nPanels; ++i)
        plot[i].Xaxis.nMajorTics = 10 - (nPanels * 2);

      if ((NumberOfXYpanels = nPanels) == 1)
        SetPlotShape(&xyyPlot[0], PORTRAIT);
      else
        ResizeMainWindow(NULL, NULL, NULL);

      break;
    }

}	/* END DELETEPANEL */

/* END PANEL.C */
