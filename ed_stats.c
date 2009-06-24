/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_stats.c

FULL NAME:	Callbacks for Edit Statistics Parameters

ENTRY POINTS:	EditStatsParms()
		SetStatsDefaults()

STATIC FNS:	CreateStatsParmsWindow()
		ApplyStatsParms()
		setOutlierList()

DESCRIPTION:	

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1998
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

static const int TOTAL_PARMS = 2;

extern Widget	AppShell;
static Widget	StatsShell = NULL, StatsParmsWindow, parmsText[TOTAL_PARMS],
		list;

void SetStatsDefaults();

static void	CreateStatsParmsWindow(), setOutlierList(),
		setOutlierVar(Widget, XtPointer, XtPointer),
		ApplyStatsParms(Widget, XtPointer, XtPointer);


/* -------------------------------------------------------------------- */
void EditStatsParms(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = True;

  if (firstTime)
    {
    CreateStatsParmsWindow();
    firstTime = False;
    }

  XtManageChild(StatsParmsWindow);
  XtPopup(XtParent(StatsParmsWindow), XtGrabNone);

/*  setOutlierList(); */
  SetStatsDefaults();

}	/* END EDITSTATSPARMS */

/* -------------------------------------------------------------------- */
void SetStatsDefaults()
{
  if (StatsShell)
;/* Fill in widgets. */

}	/* END SETSTATSDEFAULTS */

/* -------------------------------------------------------------------- */
static void setOutlierVar(Widget w, XtPointer client, XtPointer call)
{
  int		position;
  DATASET_INFO	*set;

  position = ((XmListCallbackStruct *)call)->item_position - 1;

  if ((size_t)position < NumberDataSets)
    set = &dataSet[position];
  else
    {
    position -= NumberDataSets;

    if (xyXset[0].varInfo)
      {
      if (position == 0)
        set = &xyXset[0];

      --position;
      }

    if ((size_t)position < NumberXYYsets)
      set = &xyYset[position];



    }

}	/* END SETOUTLIERVAR */

/* -------------------------------------------------------------------- */
static void ApplyStatsParms(Widget w, XtPointer client, XtPointer call)
{
  /* Re-compute stats for given variable. */
  ViewStats(NULL, NULL, NULL);

}	/* END APPLYSTATSPARMS */

/* -------------------------------------------------------------------- */
static void setOutlierList()
{
  size_t	i, cnt = 0;
  XmString      item[MAX_DATASETS<<1];
 
  XmListDeleteAllItems(list);
 
  for (i = 0; i < NumberDataSets; ++i)
    item[cnt++] = XmStringCreateLocalized(const_cast<char*>(dataSet[i].varInfo->name.c_str()));

  for (i = 0; i < NumberXYXsets; ++i)
    item[cnt++] = XmStringCreateLocalized(const_cast<char*>(xyXset[i].varInfo->name.c_str()));

  for (i = 0; i < NumberXYYsets; ++i)
    item[cnt++] = XmStringCreateLocalized(const_cast<char*>(xyYset[i].varInfo->name.c_str()));

  for (i = 0; i < 3; ++i)
    if (xyzSet[i].varInfo)
      item[cnt++] = XmStringCreateLocalized(const_cast<char*>(xyzSet[i].varInfo->name.c_str()));

  if (cnt > 0)
    XmListAddItems(list, item, cnt, 1);
 
  for (i = 0; i < cnt; ++i)
    XmStringFree(item[i]);
 
}       /* END SETOUTLIERLIST */

/* -------------------------------------------------------------------- */
static void CreateStatsParmsWindow()
{
  Widget	form, oRC, bRC, RC[4], plRC[2], label, frame[4], title[4];
  Arg		args[8];
  Cardinal	n;

  StatsShell = XtCreatePopupShell("editStatsShell",
               topLevelShellWidgetClass, AppShell, NULL, 0);

  StatsParmsWindow = XmCreateRowColumn(StatsShell, "parmsRC", NULL, 0);

  form = XmCreateForm(StatsParmsWindow, "outlierForm", NULL, 0);
  XtManageChild(form);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  list = XmCreateScrolledList(form, "olVarList", args, n);
  XtManageChild(list);
  XtAddCallback(list, XmNsingleSelectionCallback, setOutlierVar, NULL);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  oRC = XmCreateRowColumn(form, "statsRC", args, n);
  XtManageChild(oRC);


  n = 0;
  frame[0] = XmCreateFrame(oRC, "outlierFrame", args, 0);
  frame[1] = XmCreateFrame(oRC, "averageFrame", args, 0);
  frame[2] = XmCreateFrame(oRC, "regretFrame", args, 0);
  XtManageChildren(frame, 3);

  n = 0;
  title[0] = XmCreateLabel(frame[1], "outlierTitle", args, 0);
  title[1] = XmCreateLabel(frame[0], "averageTitle", args, 0);
  title[2] = XmCreateLabel(frame[2], "regretTitle", args, 0);
  XtManageChild(title[0]); XtManageChild(title[1]);
  XtManageChild(title[2]);

  n = 0;
  RC[0] = XmCreateRowColumn(frame[1], "outlierRC", args, 0);
  RC[1] = XmCreateRowColumn(frame[0], "averageRC", args, 0);
  RC[2] = XmCreateRowColumn(frame[2], "regretRC", args, 0);
  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]);


  /* Outlier frame.
   */
  n = 0;
  plRC[0] = XmCreateRowColumn(RC[0], "plRC", args, n);
  plRC[1] = XmCreateRowColumn(RC[0], "plRC", args, n);
  XtManageChildren(plRC, 2);


  label = XmCreateLabel(plRC[0], "Floor", NULL, 0);
  parmsText[1] = XmCreateTextField(plRC[0], "floorTxt", NULL, 0);
  XtAddCallback(parmsText[1], XmNlosingFocusCallback, ValidateFloat, (XtPointer)"%g");
  XtManageChild(label);
  XtManageChild(parmsText[1]);

  label = XmCreateLabel(plRC[1], "Ceiling", NULL, 0);
  parmsText[2] = XmCreateTextField(plRC[1], "ceilTxt", NULL, 0);
  XtAddCallback(parmsText[2], XmNlosingFocusCallback, ValidateFloat, (XtPointer)"%g");
  XtManageChild(label);
  XtManageChild(parmsText[2]);


  /* Averaging frame.
   */
  n = 0;
  plRC[0] = XmCreateRowColumn(RC[1], "plRC", args, n);
  XtManageChild(plRC[0]);

  n = 0;
  parmsText[0] = XmCreateTextField(plRC[0], "floorTxt", NULL, 0);
  label = XmCreateLabel(plRC[0], ":1 ratio", NULL, 0);
  XtManageChild(parmsText[0]);
  XtManageChild(label);
  XtAddCallback(parmsText[0], XmNlosingFocusCallback, ValidateInteger, (XtPointer)"%d");



  bRC = createARDbuttons(StatsParmsWindow);
  XtManageChild(bRC);

}	/* END CREATESTATSPARMSWINDOW */

/* END ED_STATS.C */
