/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_xyz.c

FULL NAME:	Callbacks for Edit Track Parameteres

ENTRY POINTS:	EditTrackParms()
		SetTrackDefaults()

STATIC FNS:	CreateTrackParmsWindow()
		ApplyTrackParms()
		SetTrackAutoScale()
		SetTrackAutoTics()

DESCRIPTION:	

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1998
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#define TOTAL_PARMS	17

extern Widget	AppShell;
static Widget	TrackShell = NULL, TrackParmsWindow, parmsText[TOTAL_PARMS],
		autoScaleButton, autoTicsButton;

static void	CreateTrackParmsWindow(),
		ApplyTrackParms(Widget w, XtPointer client, XtPointer call);


/* -------------------------------------------------------------------- */
void EditTrackParms(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = True;

  if (firstTime)
    {
    CreateTrackParmsWindow();
    firstTime = False;
    }

  XtManageChild(TrackParmsWindow);
  XtPopup(XtParent(TrackParmsWindow), XtGrabNone);

  SetTrackDefaults();

}	/* END EDITTRACKPARMS */

/* -------------------------------------------------------------------- */
void SetTrackDefaults()
{
  int	i;

  if (!TrackShell)
    return;

  SetDefaults(parmsText, &xyzPlot);

  XmToggleButtonSetState(autoScaleButton, xyzPlot.autoScale, False);
  XmToggleButtonSetState(autoTicsButton, xyzPlot.autoTics, False);

  for (i = 5; i < 11; ++i)
    XtSetSensitive(parmsText[i], 1-xyzPlot.autoScale);

  for (i = 11; i < 17; ++i)
    XtSetSensitive(parmsText[i], 1-xyzPlot.autoTics);

}	/* END SETTRACKDEFAULTS */

/* -------------------------------------------------------------------- */
static void SetTrackAutoScale(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 5; i < 11; ++i)
    XtSetSensitive(parmsText[i], xyzPlot.autoScale);

  xyzPlot.autoScale = 1 - xyzPlot.autoScale;

}	/* END SETTRACKDEFAULTS */

/* -------------------------------------------------------------------- */
static void SetTrackAutoTics(Widget w, XtPointer client, XtPointer call)
{
  int	i;

  for (i = 11; i < 17; ++i)
    XtSetSensitive(parmsText[i], xyzPlot.autoTics);

  xyzPlot.autoTics = 1 - xyzPlot.autoTics;

}	/* END SETTRACKDEFAULTS */

/* -------------------------------------------------------------------- */
static void ApplyTrackParms(Widget w, XtPointer client, XtPointer call)
{
  ApplyParms(parmsText, &xyzPlot);
  DrawMainWindow();

}	/* END APPLYTRACKPARMS */

/* -------------------------------------------------------------------- */
static void CreateTrackParmsWindow()
{
  int		i;
  Widget	RC[5];

  TrackShell = XtCreatePopupShell("editXYZShell",
                  topLevelShellWidgetClass, AppShell, NULL, 0);

  TrackParmsWindow = XmCreateRowColumn(TrackShell, "parmsRC", NULL, 0);

  for (i = 0; i < TOTAL_PARMS; ++i)
    parmsText[i] = NULL;

  RC[0] = createParamsTitles(TrackParmsWindow, parmsText);
  RC[1] = createParamsLabels(TrackParmsWindow, parmsText, &xyzPlot);
  RC[2] = createParamsMinMax(TrackParmsWindow, parmsText, &xyzPlot, &autoScaleButton);
  RC[3] = createParamsTics(TrackParmsWindow, parmsText, &xyzPlot, &autoTicsButton);
  RC[4] = createARDbuttons(TrackParmsWindow);
  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);
  XtManageChild(RC[4]);

  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, ApplyTrackParms, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, ApplyTrackParms, NULL);
  XtAddCallback(autoScaleButton, XmNvalueChangedCallback, SetTrackAutoScale, NULL);
  XtAddCallback(autoTicsButton, XmNvalueChangedCallback, SetTrackAutoTics, NULL);

  for (i = 0; i < TOTAL_PARMS-1; ++i)
    if (parmsText[i])
      XtAddCallback(parmsText[i], XmNlosingFocusCallback, ApplyTrackParms ,NULL);
 
}	/* END CREATETRACKPARMSWINDOW */

/* END ED_XYZ.C */
