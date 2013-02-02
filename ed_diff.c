/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_diff.c

FULL NAME:	Callbacks for Edit Difference Parameteres

ENTRY POINTS:	EditDiffParms()
		SetDiffDefaults()

STATIC FNS:	CreateDiffParmsWindow()
		ApplyDiffParms()
		SetDiffAutoScale()
		SetDiffAutoTics()

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

static const int TOTAL_PARMS = 17;

extern Widget	AppShell;
static Widget	DiffShell = NULL, DiffParmsWindow, parmsText[TOTAL_PARMS],
		autoScaleButton, autoTicsButton;

static void	CreateDiffParmsWindow(), 
		ApplyDiffParms(Widget w, XtPointer client, XtPointer call);


/* -------------------------------------------------------------------- */
void SetDiffDefaults()
{
  int	i;

  if (DiffShell == NULL)
    return;

  SetDefaults(parmsText, &diffPlot);

  XmToggleButtonSetState(autoScaleButton, diffPlot.autoScale, False);
  XmToggleButtonSetState(autoTicsButton, diffPlot.autoTics, False);

  for (i = 7; i < 9; ++i)
    XtSetSensitive(parmsText[i], 1-diffPlot.autoScale);

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], 1-diffPlot.autoTics);

}	/* END SETDIFFDEFAULTS */

/* -------------------------------------------------------------------- */
void EditDiffParms(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = True;

  if (firstTime)
    {
    CreateDiffParmsWindow();
    firstTime = False;
    }

  XtManageChild(DiffParmsWindow);
  XtPopup(XtParent(DiffParmsWindow), XtGrabNone);

  SetDiffDefaults();

}	/* END EDITDIFFPARMS */

/* -------------------------------------------------------------------- */
static void SetDiffAutoScale(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 7; i < 9; ++i)
    XtSetSensitive(parmsText[i], diffPlot.autoScale);

  diffPlot.autoScale = 1 - diffPlot.autoScale;

}	/* END SETDIFFAUTOSCALE */

/* -------------------------------------------------------------------- */
static void SetDiffAutoTics(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 11; i < 15; ++i)
    XtSetSensitive(parmsText[i], diffPlot.autoTics);

  diffPlot.autoTics = 1 - diffPlot.autoTics;

}	/* END SETDIFFAUTOTICS */

/* -------------------------------------------------------------------- */
static void ApplyDiffParms(Widget w, XtPointer client, XtPointer call)
{
  ApplyParms(parmsText, &diffPlot);

  if (diffPlot.windowOpen)
    {
    DiffWinUp(NULL, NULL, NULL);
    PlotDifference(NULL, NULL, NULL);
    }

  SetDiffDefaults();

}	/* END APPLYDIFFPARMS */

/* -------------------------------------------------------------------- */
static void CreateDiffParmsWindow()
{
  int		i;
  Widget	RC[5];

  DiffShell = XtCreatePopupShell("editDiffShell",
                  topLevelShellWidgetClass, AppShell, NULL, 0);

  DiffParmsWindow = XmCreateRowColumn(DiffShell, (char *)"parmsRC", NULL, 0);

  for (i = 0; i < TOTAL_PARMS; ++i)
    parmsText[i] = NULL;

  RC[0] = createParamsTitles(DiffParmsWindow, parmsText);
  RC[1] = createParamsLabels(DiffParmsWindow, parmsText, &diffPlot);
  RC[2] = createParamsMinMax(DiffParmsWindow, parmsText, &diffPlot, &autoScaleButton);
  RC[3] = createParamsTics(DiffParmsWindow, parmsText, &diffPlot, &autoTicsButton);
  RC[4] = createARDbuttons(DiffParmsWindow);
  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]);

  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, SetDiffAutoScale, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, ApplyDiffParms, NULL);
  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, ApplyDiffParms, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, SetDiffAutoTics, NULL);

  for (i = 0; i < TOTAL_PARMS-1; ++i)
    if (parmsText[i])
      XtAddCallback(parmsText[i], XmNlosingFocusCallback, ApplyDiffParms, NULL);

}	/* END CREATEDIFFPARMSWINDOW */

/* END ED_DIFF.C */
