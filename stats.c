/*
-------------------------------------------------------------------------
OBJECT NAME:	stats.c

FULL NAME:	Statistics Window & callbacks

ENTRY POINTS:	ViewStats()
		SetStatsData()
		ComputeStats()

STATIC FNS:	CreateStatsWindow()
		DismissStats()
		PrintStats()
		formatLine()

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	Menu button.

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-9
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#define NO_NETCDF_2

#include <netcdf.h>

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

static Widget StatsShell, StatsWindow, statsText;

static void
	CreateStatsWindow(),
	PrintStats(Widget w, XtPointer client, XtPointer call),
	DismissStats(Widget w, XtPointer client, XtPointer call);
static char *formatLine(char *, DATASET_INFO *);

static char *statTitle = "Variable              nPoints         Min         Max        Mean       Sigma         Var\n";

extern Widget	AppShell;


/* -------------------------------------------------------------------- */
static void DismissStats(Widget w, XtPointer client, XtPointer call)
{
  StatsWinOpen = False;

  if (StatsWindow)
    {
    XtUnmanageChild(StatsWindow);
    XtPopdown(XtParent(StatsWindow));
    }

}	/* END DISMISSSTATS */

/* -------------------------------------------------------------------- */
void ViewStats(Widget w, XtPointer client, XtPointer call)
{
  SetStatsData();

  if (StatsWindow)
    {
    XtManageChild(StatsWindow);
    XtPopup(XtParent(StatsWindow), XtGrabNone);
    StatsWinOpen = True;
    }

}	/* END VIEWSTATS */

/* -------------------------------------------------------------------- */
void SetStatsData()
{
  int	i;

  static bool	firstTime = True;

  if (firstTime) {
    CreateStatsWindow();
    firstTime = False;
    }

  XmTextSetString(statsText, statTitle);

  for (i = 0; i < NumberDataSets; ++i)
    {
    formatLine(buffer, &dataSet[i]);
    XmTextInsert(statsText, XmTextGetLastPosition(statsText), buffer);
    }

  for (i = 0; i < NumberXYXsets; ++i)
    {
    formatLine(buffer, &xyXset[i]);
    XmTextInsert(statsText, XmTextGetLastPosition(statsText), buffer);
    }

  for (i = 0; i < NumberXYYsets; ++i)
    {
    formatLine(buffer, &xyYset[i]);
    XmTextInsert(statsText, XmTextGetLastPosition(statsText), buffer);
    }

  for (i = 0; i < 3; ++i)
    if (xyzSet[i].varInfo)
      {
      formatLine(buffer, &xyzSet[i]);
      XmTextInsert(statsText, XmTextGetLastPosition(statsText), buffer);
      }

  if (WindBarbs)
    {
    formatLine(buffer, &ui);
    XmTextInsert(statsText, XmTextGetLastPosition(statsText), buffer);

    formatLine(buffer, &vi);
    XmTextInsert(statsText, XmTextGetLastPosition(statsText), buffer);
    }

}	/* END SETSTATSDATA */

/* -------------------------------------------------------------------- */
void ComputeStats(DATASET_INFO *set)
{
  int       i, missCnt, len = 0;
  NR_TYPE   y;
  double    sum, sigma;
 
  /* Read in variables units from file.
   */
  buffer[0] = '\0';

  if (set->varInfo->inVarID != COMPUTED)
    {
    nc_inq_attlen(dataFile[set->fileIndex].ncid, set->varInfo->inVarID,
						"units", (size_t *)&len);
    nc_get_att_text(dataFile[set->fileIndex].ncid, set->varInfo->inVarID,
						"units", buffer);
    if (strcmp(buffer, "C") == 0)
      {
      buffer[0] = 0xb0;
      buffer[1] = 'C';
      buffer[2] = '\0';
      }
    }
  else
    strcpy(buffer, "Unk");

  strncpy(set->stats.units, buffer, UNITS_LEN);

  if (len < UNITS_LEN)
    set->stats.units[len] = '\0';
  else
    set->stats.units[UNITS_LEN-1] = '\0';

  missCnt = 0;
  sum = sigma = 0.0;
 
  set->stats.min = FLT_MAX;
  set->stats.max = -FLT_MAX;
 
  for (i = 0; i < set->nPoints; ++i)
    {
    if ((y = set->data[i]) == set->missingValue ||
        y < set->stats.outlierMin || y > set->stats.outlierMax)
      {
      ++missCnt;
      continue;
      }
 
    set->stats.min = MIN(set->stats.min, y);
    set->stats.max = MAX(set->stats.max, y);

    sum += y;
    }
 
  if (set->nPoints == missCnt)
    {
    set->stats.mean = 0.0;
    set->stats.min = 0.0;
    set->stats.max = 0.0;
    }
  else
    {
    set->stats.nPoints = set->nPoints - missCnt;
    set->stats.mean = sum / set->stats.nPoints;
    }

  for (i = 0; i < set->nPoints; ++i)
    if ((y = set->data[i]) != set->missingValue ||
        y < set->stats.outlierMin || y > set->stats.outlierMax)
      sigma += pow(y - set->stats.mean, 2.0);
 
  set->stats.variance = sigma / (set->stats.nPoints - 1);
  set->stats.sigma = (float)sqrt(sigma / (set->stats.nPoints - 1));

}   /* END COMPUTESTATS */

/* -------------------------------------------------------------------- */
static void CreateStatsWindow()
{
  Arg         args[8];
  Cardinal    n;
  Widget      drFrame, drRC, b[3];

  n = 0;
  StatsShell = XtCreatePopupShell("statsShell",
                  topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  StatsWindow = XmCreateForm(StatsShell, "statsForm", args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  drFrame = XmCreateFrame(StatsWindow, "buttonFrame", args, n);
  XtManageChild(drFrame);

  n = 0;
  drRC = XmCreateRowColumn(drFrame, "buttonRC", args, n);
  XtManageChild(drRC);


  n = 0;
  b[0] = XmCreatePushButton(drRC, "dismissButton", args, n);
  b[1] = XmCreatePushButton(drRC, "printButton", args, n);
  b[2] = XmCreatePushButton(drRC, "parmsButton", args, n);
  XtManageChildren(b, 3);
  XtAddCallback(b[0], XmNactivateCallback, DismissStats, StatsWindow);
  XtAddCallback(b[1], XmNactivateCallback, PrintStats, NULL);
  XtAddCallback(b[2], XmNactivateCallback, EditStatsParms, NULL);


  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, drFrame); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  statsText = XmCreateScrolledText(StatsWindow, "statsText", args, n);
  XtManageChild(statsText);

}	/* END CREATESTATSWINDOW */

/* -------------------------------------------------------------------- */
static void PrintStats(Widget w, XtPointer client, XtPointer call)
{
  FILE    *fp;
  int     i;
  char	*p;

  if ((p = getenv("LPDEST")) != NULL)
    printf("Output being sent to %s.\n", p);

  if ((fp = popen(printerSetup.lpCommand, "w")) == NULL)
    {
    ShowError("PrintStats: can't open pipe to 'lp'");
    return;
    }


  fprintf(fp, "%s, %s\n\n", mainPlot[0].title, mainPlot[0].subTitle);
  fprintf(fp, statTitle);

  for (i = 0; i < NumberDataSets; ++i)
    {
    formatLine(buffer, &dataSet[i]);
    fprintf(fp, buffer);
    }

  for (i = 0; i < NumberXYXsets; ++i)
    {
    formatLine(buffer, &xyXset[i]);
    fprintf(fp, buffer);
    }

  for (i = 0; i < NumberXYYsets; ++i)
    {
    formatLine(buffer, &xyYset[i]);
    fprintf(fp, buffer);
    }

  for (i = 0; i < 3; ++i)
    if (xyzSet[i].varInfo)
      {
      formatLine(buffer, &xyzSet[i]);
      fprintf(fp, buffer);
      }

  if (WindBarbs)
    {
    formatLine(buffer, &ui);
    fprintf(fp, buffer);

    formatLine(buffer, &vi);
    fprintf(fp, buffer);
    }

  pclose(fp);

}	/* END PRINTSTATS */

/* -------------------------------------------------------------------- */
static char *formatLine(char buff[], DATASET_INFO *set)
{
  char	temp[32];

  memset(buff, ' ', 256);

  sprintf(temp, "%s (%s)", set->varInfo->name, set->stats.units);
  memcpy(buff, temp, strlen(temp));

  sprintf(temp, "%5ld/%ld", set->stats.nPoints, set->nPoints);
  memcpy(&buff[20], temp, strlen(temp));

  sprintf(&buff[32], "%11.3e %11.3e %11.3e %11.3e %11.3e\n",
      set->stats.min,
      set->stats.max,
      set->stats.mean,
      set->stats.sigma,
      set->stats.variance);

  return(buff);

}

/* END STATS.C */
