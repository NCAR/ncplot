/*
-------------------------------------------------------------------------
OBJECT NAME:	header.c

FULL NAME:	netCDF Header Window & callbacks

ENTRY POINTS:	ViewHeader()

STATIC FNS:	CreateHeaderWindow()
		SetHeaderData()
		DismissHeader()
		PrintHeader()

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	Menu button.

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"
#include "netcdf.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

static Widget HeaderShell, HeaderWindow = NULL, headerText;

static void
	CreateHeaderWindow(), SetHeaderData(),
	PrintHeader(Widget w, XtPointer client, XtPointer call),
	DismissHeader(Widget w, XtPointer client, XtPointer call);

extern Widget	AppShell;


/* -------------------------------------------------------------------- */
static void DismissHeader(Widget w, XtPointer client, XtPointer call)
{
  if (HeaderWindow)
    {
    XtUnmanageChild(HeaderWindow);
    XtPopdown(XtParent(HeaderWindow));
    XmTextSetString(headerText, "");
    }

}	/* END DISMISSHEADER */

/* -------------------------------------------------------------------- */
void ViewHeader(Widget w, XtPointer client, XtPointer call)
{
  if (NumberDataFiles == 0)
    return;

  if (!HeaderWindow)
    CreateHeaderWindow();

  SetHeaderData();

  if (HeaderWindow) {
    XtManageChild(HeaderWindow);
    XtPopup(XtParent(HeaderWindow), XtGrabNone);
    }

}	/* END VIEWHEADER */

/* -------------------------------------------------------------------- */
static void SetHeaderData()
{
  FILE	*pp;

  XmTextSetString(headerText, dataFile[CurrentDataFile].fileName);
  XmTextInsert(headerText, XmTextGetLastPosition(headerText), "\n\n");

  sprintf(buffer, "ncdump -h %s", dataFile[CurrentDataFile].fileName);

  if ((pp = popen(buffer, "r")) == NULL)
    {
    char	msg[128];

    sprintf(msg, "Can't open pipe [%s]", buffer);
    HandleError(msg, Interactive, IRET);
    return;
    }

  while (fread(buffer, BUFFSIZE, 1, pp) > 0)
    XmTextInsert(headerText, XmTextGetLastPosition(headerText), buffer);

  if (strncmp(buffer, "ncdump", 6) == 0)
    {
    HandleError("Can't locate netCDF utility ncdump.", Interactive, IRET);
    return;
    }
  else
    {
    strcpy(strchr(buffer, '}')+1, "\n\n");
    XmTextInsert(headerText, XmTextGetLastPosition(headerText), buffer);
    }

  pclose(pp);

}	/* END SETHEADERDATA */

/* -------------------------------------------------------------------- */
static void CreateHeaderWindow()
{
  Arg         args[8];
  Cardinal    n;
  Widget      drFrame, drRC, b[3];

  n = 0;
  HeaderShell = XtCreatePopupShell("headerShell",
                  topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  HeaderWindow = XmCreateForm(HeaderShell, "headerForm", args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  drFrame = XmCreateFrame(HeaderWindow, "buttonFrame", args, n);
  XtManageChild(drFrame);

  n = 0;
  drRC = XmCreateRowColumn(drFrame, "buttonRC", args, n);
  XtManageChild(drRC);

  n = 0;
  b[0] = XmCreatePushButton(drRC, "dismissButton", args, n);
  XtAddCallback(b[0], XmNactivateCallback, DismissHeader, HeaderWindow);

  n = 0;
  b[1] = XmCreatePushButton(drRC, "printButton", args, n);
  XtAddCallback(b[1], XmNactivateCallback, PrintHeader, NULL);

  XtManageChildren(b, 2);


  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, drFrame); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  headerText = XmCreateScrolledText(HeaderWindow, "headerText", args, n);
  XtManageChild(headerText);

}	/* END CREATEHEADERWINDOW */

/* -------------------------------------------------------------------- */
static void PrintHeader(Widget w, XtPointer client, XtPointer call)
{
  FILE	*fp;
  char	*p;

  if ((p = getenv("LPDEST")) != NULL)
    printf("Output being sent to %s.\n", p);

  if ((fp = popen(printerSetup.lpCommand, "w")) == NULL)
    {
    ShowError("PrintHeader: can't open pipe to 'lp'");
    return;
    }

  fprintf(fp, "%s, %s\n\n", mainPlot[0].title, mainPlot[0].subTitle);

  p = XmTextGetString(headerText);
  fprintf(fp, "%s\n", p);
  XtFree(p);

  pclose(fp);

}	/* END PRINTHEADER */

/* END HEADER.C */
