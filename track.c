/*
-------------------------------------------------------------------------
OBJECT NAME:	track.c

FULL NAME:	Track Options window.

ENTRY POINTS:	TrackOptWinControl()

STATIC FNS:	CreateTrackOptWin()
		LoadTrack()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>


#define INS	1
#define GPS	2
#define CORRECTED	3

extern char	*insVariables[], *gpsVariables[], *gpsCorrected[],
		*windVariables[];

extern Widget   AppShell;
static Widget   TrackOptShell = NULL;
Widget   TrackOptWindow = NULL;	// Exported to dataIO.c for Cursor Control
static Widget	wvText[2], tiText, average[2], optButton[8], dirText, tsText;

/* static char *defaultUI = "UIC", *defaultVI = "VIC", *defaultTI = "300"; */

void	findMinMax();
static void CreateTrackOptWin();

/* -------------------------------------------------------------------- */
std::string GetUI()
{
  char *p = XmTextFieldGetString(wvText[0]);
  std::string s(p);
  XtFree(p);
  return(s);

}	/* END GETUI */

/* -------------------------------------------------------------------- */
std::string GetVI()
{
  char *p = XmTextFieldGetString(wvText[0]);
  std::string s(p);
  XtFree(p);
  return(s);

}	/* END GETVI */

/* -------------------------------------------------------------------- */
std::string GetTI()
{
  char *p = XmTextFieldGetString(tiText);
  std::string s(p);
  XtFree(p);
  return(s);

}	/* END GETTI */

/* -------------------------------------------------------------------- */
void SetUI(char name[])
{
  XmTextFieldSetString(wvText[0], name);

}	/* END SETUI */

/* -------------------------------------------------------------------- */
void SetVI(char name[])
{
  XmTextFieldSetString(wvText[1], name);

}	/* END SETVI */

/* -------------------------------------------------------------------- */
void SetTI(char name[])
{
  XmTextFieldSetString(tiText, name);

}	/* END SETTI */

/* -------------------------------------------------------------------- */
int isAverage()
{
  return(XmToggleButtonGetState(average[0]));

}	/* END ISAVERAGE */

/* -------------------------------------------------------------------- */
static void LoadTrack(Widget w, XtPointer client, XtPointer call)
{
  int	set, rc;
  char	**posVar;

  if (NumberDataFiles == 0)
    {
    HandleError("No data file open.\n", Interactive, IRET);
    return;
    }

  if ((int)client == INS)
    posVar = insVariables;
  else
  if ((int)client == GPS)
    posVar = gpsVariables;
  else
    posVar = gpsCorrected;

  if (PlotType == XY_PLOT)
    {
    /* The "GG" thing is to check for GLAT if default GGLAT fails, etc.
     */
    if ((rc = LoadVariable(&xyXset[NumberXYXsets], posVar[0])) == ERR &&
      strncmp(posVar[0], "GG", 2) == 0)
      rc = LoadVariable(&xyXset[NumberXYXsets], &posVar[0][1]);

    if (rc == ERR)
      fprintf(stderr, "Can't locate variable %s\n", posVar[0]);
    else
      {
      sprintf(buffer, "%s (%s)",
		xyXset[NumberXYXsets].varInfo->name.c_str(),
		xyXset[NumberXYXsets].stats.units.c_str());
      xyyPlot[CurrentPanel].Xaxis.label = buffer;

      ++NumberXYXsets;
      }

    if ((rc = LoadVariable(&xyYset[NumberXYYsets], posVar[1])) == ERR &&
      strncmp(posVar[1], "GG", 2) == 0)
      rc = LoadVariable(&xyYset[NumberXYYsets], &posVar[1][1]);

    if (rc == ERR)
      fprintf(stderr, "Can't locate variable %s\n", posVar[1]);
    else
      {
      sprintf(buffer, "%s (%s)",
		xyYset[NumberXYYsets].varInfo->name.c_str(),
		xyYset[NumberXYYsets].stats.units.c_str());
      xyyPlot[CurrentPanel].Yaxis[0].label = buffer;

      ++NumberXYYsets;
      }

    findMinMax();
    }


  if (PlotType == XYZ_PLOT)
    {
    for (set = 0; set < 3; ++set)
      if (xyzSet[set].varInfo)
        {
        xyzSet[set].varInfo = NULL;
        xyzSet[set].nPoints = 0;
        delete [] xyzSet[set].data;
        }

    if (LoadVariable(&xyzSet[0], posVar[0]) == ERR)
      fprintf(stderr, "Can't locate variable %s\n", posVar[0]);
    else
      {
      sprintf(buffer, "%s (%s)",
              xyzSet[0].varInfo->name.c_str(), xyzSet[0].stats.units.c_str());
      xyzPlot.Xaxis.label = buffer;
      }
    if (LoadVariable(&xyzSet[1], posVar[2]) == ERR)
      fprintf(stderr, "Can't locate variable %s\n", posVar[2]);
    else
      {
      sprintf(buffer, "%s (%s)",
              xyzSet[1].varInfo->name.c_str(), xyzSet[1].stats.units.c_str());
      xyzPlot.Yaxis[0].label = buffer;
      }
    if (LoadVariable(&xyzSet[2], posVar[1]) == ERR)
      fprintf(stderr, "Can't locate variable %s\n", posVar[1]);
    else
      {
      sprintf(buffer, "%s (%s)",
              xyzSet[2].varInfo->name.c_str(), xyzSet[2].stats.units.c_str());
      xyzPlot.Zaxis.label = buffer;
      }

    findMinMax();
    }

  DataChanged = true;
  DrawMainWindow();

}	/* END LOADTRACK */

/* -------------------------------------------------------------------- */
void TrackOptWinControl()
{
  static bool firstTime = true;

  if (firstTime)
    {
    if (PlotType == TIME_SERIES)	/* weird bug, if you remove */
      return;

    CreateTrackOptWin();
    firstTime = false;
    }

  switch (PlotType)
    {
    case TIME_SERIES:
      XtUnmanageChild(TrackOptWindow);
      XtPopdown(XtParent(TrackOptWindow));
      break;

    case XY_PLOT:
      XtSetSensitive(optButton[0], true);
      XtSetSensitive(optButton[1], false);
      XtSetSensitive(optButton[2], false);
      XtManageChild(TrackOptWindow);
      XtPopup(XtParent(TrackOptWindow), XtGrabNone);
      break;


    case XYZ_PLOT:
      XtSetSensitive(optButton[0], false);
      XtSetSensitive(optButton[1], true);
      XtSetSensitive(optButton[2], true);
      XtManageChild(TrackOptWindow);
      XtPopup(XtParent(TrackOptWindow), XtGrabNone);
      break;
    }

}  /* END TRACKOPTWINCONTROL */

/* -------------------------------------------------------------------- */
static void CreateTrackOptWin()
{
  Cardinal	n;
  Arg		args[5];
  Widget	trackOptRC, frame[5], RC[5], b[8], plRC[3], label;

  n = 0;
  TrackOptShell = XtCreatePopupShell("trackOptShell",
                        topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  TrackOptWindow = XmCreateForm(TrackOptShell, "trackOptWindow", args, n);

  n = 0;
  trackOptRC = XmCreateRowColumn(TrackOptWindow, "trackOptRC", args, n);
  XtManageChild(trackOptRC);


  n = 0;
  frame[0] = XmCreateFrame(trackOptRC, "trackLoadFrame", args, 0);
  frame[1] = XmCreateFrame(trackOptRC, "windFrame", args, 0);
  frame[2] = XmCreateFrame(trackOptRC, "miscOptFrame", args, 0);
  XtManageChildren(frame, 3);

  n = 0;
  RC[0] = XmCreateRowColumn(frame[0], "trackLoadRC", args, 0);
  RC[1] = XmCreateRowColumn(frame[1], "windRC", args, 0);
  RC[2] = XmCreateRowColumn(frame[2], "miscRC", args, 0);
  XtManageChild(RC[0]);
  XtManageChild(RC[1]);
  XtManageChild(RC[2]);


  /* INS/GPS Track.
   */
  n = 0;
  b[0] = XmCreatePushButton(RC[0], "Load INS Track", args, n);
  b[1] = XmCreatePushButton(RC[0], "Load GPS Track", args, n);
  b[2] = XmCreatePushButton(RC[0], "Load GPS Corrected", args, n);
  b[3] = XmCreateToggleButton(RC[0], "Landmarks", args, n);
  b[4] = XmCreateToggleButton(RC[0], "Geo-political map", args, n);
  b[5] = XmCreateToggleButton(RC[0], "Track scaling", args, n);

  XtManageChildren(b, 6);

  XtAddCallback(b[0], XmNactivateCallback, LoadTrack, (void *)INS);
  XtAddCallback(b[1], XmNactivateCallback, LoadTrack, (void *)GPS);
  XtAddCallback(b[2], XmNactivateCallback, LoadTrack, (void *)CORRECTED);
  XtAddCallback(b[3], XmNvalueChangedCallback, ToggleLandMarks, NULL);
  XtAddCallback(b[4], XmNvalueChangedCallback, ToggleGeoPolMap, NULL);
  XtAddCallback(b[5], XmNvalueChangedCallback, ToggleEqualScaling, NULL);

  if (getenv("GMTHOME") == NULL)
    XtSetSensitive(b[3], false);


  /* Wind vector options.
   */
  n = 0;
  b[0] = XmCreateToggleButton(RC[1], "Wind vectors", args, n);
  XtManageChildren(b, 1);

  XtAddCallback(b[0], XmNvalueChangedCallback, ToggleWindBarbs, NULL);

  n = 0;
  plRC[0] = XmCreateRadioBox(RC[1], "plRC", args, n);
  plRC[1] = XmCreateRowColumn(RC[1], "plRC", args, n);
  plRC[2] = XmCreateRowColumn(RC[1], "plRC", args, n);
  XtManageChildren(plRC, 3);

  n = 0;
  average[0] = XmCreateToggleButton(plRC[0], "Averaged", args, n);
  average[1] = XmCreateToggleButton(plRC[0], "Instant", args, n);
  XtManageChildren(average, 2);
  XmToggleButtonSetState(average[0], true, false);

  /* Time interval. */
  n = 0;
  label = XmCreateLabel(plRC[1], "Interval (secs)", args, n);
  tiText = XmCreateTextField(plRC[1], "tiText", args, n);
  XtManageChild(label);
  XtManageChild(tiText);
  XmTextFieldSetString(tiText, windVariables[2]);

  /* UI & VI Text widgets. */
  n = 0;
  wvText[0] = XmCreateTextField(plRC[2], "uiText", args, n);
  wvText[1] = XmCreateTextField(plRC[2], "uiText", args, n);
  XtManageChildren(wvText, 2);
  XmTextFieldSetString(wvText[0], windVariables[0]);
  XmTextFieldSetString(wvText[1], windVariables[1]);


  /* Other Options. */
  n = 0;
  plRC[0] = XmCreateRowColumn(RC[2], "pnRC", args, n); XtManageChild(plRC[0]);
  dirText = XmCreateTextField(plRC[0], "dirText", args, n); XtManageChild(dirText);
  label = XmCreateLabel(plRC[0], "Direction arrows", args, n); XtManageChild(label);

  plRC[0] = XmCreateRowColumn(RC[2], "pnRC", args, n); XtManageChild(plRC[0]);
  tsText = XmCreateTextField(plRC[0], "tsText", args, n); XtManageChild(tsText);
  label = XmCreateLabel(plRC[0], "Time Stamps", args, n); XtManageChild(label);

  optButton[0] = XmCreateToggleButton(RC[2], "Scatter plot (on/off)", args,n);
  optButton[1] = XmCreateToggleButton(RC[2], "Project to XY-plane", args, n);
  optButton[2] = XmCreateToggleButton(RC[2], "Project to back planes", args, n);
  XtManageChildren(optButton, 3);

  XtAddCallback(dirText, XmNvalueChangedCallback, ToggleArrows, NULL);
  XtAddCallback(tsText, XmNvalueChangedCallback, ToggleTimeStamps, NULL);
  XtAddCallback(optButton[0], XmNvalueChangedCallback, ToggleScatter, NULL);
  XtAddCallback(optButton[1], XmNvalueChangedCallback, ToggleProject, NULL);
  XtAddCallback(optButton[2], XmNvalueChangedCallback, ToggleProject, (XtPointer)1);

  n = 0;
  XtSetArg(args[n], XmNy,
	HeightOfScreen(XtScreen(xyyPlot[0].canvas)) - 250); ++n;
  XtSetValues(TrackOptShell, args, n);

}  /* END CREATETRACKOPTWIN */

/* END TRACK.C */
