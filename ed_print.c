/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_print.c

FULL NAME:	Callbacks for Printer Setup

ENTRY POINTS:	CreatePrintWindow()
		EditPrintParms()
		SetPrinterShape()
		SetPrinter()
		GetPRinterList()

STATIC FNS:	ApplyPrintParms()

DESCRIPTION:	This set of procedures Creates & pops up/down the widget
		for "Edit Parameters" menu item.

		CreatePrintWindow - Creates the dialog widget.
		EditPrintParms	- Sets text widgets to current values
			  and pops up dialog window.
		ApplyPrintParms	- Retrieves new values and puts in global
			  variables.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#define TOTAL_PARMS	3
#define MAX_PRINTERS	20

static Widget	PrintShell, PrintWindow, parmsText[TOTAL_PARMS], shapeB[2],
		colorB[2];

static void	CreatePrintWindow(), ApplyPrintParms();

extern Widget   AppShell;

static char	*printerList[MAX_PRINTERS];


/* -------------------------------------------------------------------- */
void EditPrintParms(Widget w, XtPointer clientData, XtPointer callData)
{
  static bool	firstTime = True;

  if (firstTime)
    {
    CreatePrintWindow();
    firstTime = False;
    }

  XmTextFieldSetString(parmsText[0], printerSetup.lpCommand);
  sprintf(buffer, "%.3f", printerSetup.width);
  XmTextFieldSetString(parmsText[1], buffer);
  sprintf(buffer, "%.3f", printerSetup.height);
  XmTextFieldSetString(parmsText[2], buffer);

  SetPrinterShape(printerSetup.shape);

  XtManageChild(PrintWindow);
  XtPopup(XtParent(PrintWindow), XtGrabNone);

}	/* END EDITPRINTPARMS */

/* -------------------------------------------------------------------- */
static void ApplyPrintParms(Widget w, XtPointer clientData, XtPointer callData)
{
  char	*p;

  p = XmTextFieldGetString(parmsText[0]);
  strcpy(printerSetup.lpCommand, p);
  XtFree(p);

  p = XmTextFieldGetString(parmsText[1]);
  printerSetup.width = atof(p);
  XtFree(p);

  p = XmTextFieldGetString(parmsText[2]);
  printerSetup.height = atof(p);
  XtFree(p);

  if (XmToggleButtonGetState(shapeB[0]))
    printerSetup.shape = PORTRAIT;

  if (XmToggleButtonGetState(shapeB[1]))
    printerSetup.shape = LANDSCAPE;

  if (XmToggleButtonGetState(colorB[0]))
    printerSetup.color = True;

  if (XmToggleButtonGetState(colorB[1]))
    printerSetup.color = False;

}	/* END APPLYPRINTPARMS */

/* -------------------------------------------------------------------- */
void SetPrinterShape(int shape)
{
  printerSetup.shape = shape;

  if (PrintWindow)
    switch (shape)
      {
      case LANDSCAPE:
        XmToggleButtonSetState(shapeB[0], False, False);
        XmToggleButtonSetState(shapeB[1], True, False);
        break;

      case PORTRAIT:
        XmToggleButtonSetState(shapeB[0], True, False);
        XmToggleButtonSetState(shapeB[1], False, False);
        break;
      }

}	/* END SETPRINTERSHAPE */

/* -------------------------------------------------------------------- */
void SetPrinter(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

#ifdef SVR4
  p = strstr(printerSetup.lpCommand, "-d");
#else
  p = strstr(printerSetup.lpCommand, "-P");
#endif

  if (p)
    p[-1] = '\0';

  if (strcmp((char *)client, "Default"))
    {
#ifdef SVR4
    strcat(printerSetup.lpCommand, " -d ");
#else
    strcat(printerSetup.lpCommand, " -P ");
#endif
    strcat(printerSetup.lpCommand, (char *)client);
    }

  EditPrintParms(NULL, NULL, NULL);

}

/* -------------------------------------------------------------------- */
static void CreatePrintWindow()
{
  Widget	plRC[TOTAL_PARMS], label, b[3], frame[5], RC[5],
		slPD, slOpMenu, slButts[MAX_PRINTERS];
  Arg		args[10];
  XmString	name;
  int		i, n, cnt;

  n = 0;
  PrintShell = XtCreatePopupShell("printShell",
                topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  PrintWindow = XmCreateRowColumn(PrintShell, "parmsRC", args, n);

  n = 0;
  frame[0] = XmCreateFrame(PrintWindow, "printParmsFrame", args, 0);
  frame[1] = XmCreateFrame(PrintWindow, "shapeFrame", args, 0);
  frame[2] = XmCreateFrame(PrintWindow, "colorFrame", args, 0);
  frame[3] = XmCreateFrame(PrintWindow, "buttonFrame", args, 0);
  XtManageChildren(frame, 4);

  n = 0;
  RC[0] = XmCreateRowColumn(frame[0], "printParmsRC", args, 0);
  RC[1] = XmCreateRadioBox(frame[1], "plRC", args, 0);
  RC[2] = XmCreateRadioBox(frame[2], "plRC", args, 0);
  RC[3] = XmCreateRowColumn(frame[3], "buttonRC", args, 0);
  XtManageChild(RC[0]); XtManageChild(RC[1]);
  XtManageChild(RC[2]); XtManageChild(RC[3]);


  /* Printer Op Menu
   */
  n = 0;
  slPD = XmCreatePulldownMenu(RC[0], "slPullDown", args, n);

  n = 0;
  XtSetArg(args[n], XmNsubMenuId, slPD); ++n;
  slOpMenu = XmCreateOptionMenu(RC[0], "prOpMenu", args, n);
  XtManageChild(slOpMenu);


  for (i = 0; printerList[i]; ++i)
    {
    name = XmStringCreateLocalized(printerList[i]);

    n = 0;
    XtSetArg(args[n], XmNlabelString, name); ++n;
    slButts[i] = XmCreatePushButton(slPD, "opMenB", args, n);
    XtAddCallback(slButts[i], XmNactivateCallback, SetPrinter,
                  (XtPointer)printerList[i]);

    XmStringFree(name);
    }

  XtManageChildren(slButts, i);


  /* Plot Parameters.
   */
  for (i = cnt = 0; i < TOTAL_PARMS; ++i, ++cnt)
    {
    n = 0;
    plRC[i] = XmCreateRowColumn(RC[0], "plRC", args, n);

    n = 0;
    sprintf(buffer, "lbl%d", cnt);
    label = XmCreateLabel(plRC[i], buffer, args, n);

    n = 0;
    sprintf(buffer, "txt%d", cnt);
    parmsText[cnt] = XmCreateTextField(plRC[i], buffer, args, n);

    XtManageChild(label);
    XtManageChild(parmsText[cnt]);

    if (i == 1 || i == 2)
      {
      XtAddCallback(parmsText[cnt],XmNlosingFocusCallback, ValidateFloat,
		(XtPointer)"%.3f");

      label = XmCreateLabel(plRC[i], "inches", NULL, 0);
      XtManageChild(label);
      }
    }

  XtManageChildren(plRC, TOTAL_PARMS);


  n = 0;
  shapeB[0] = XmCreateToggleButton(RC[1], "Portrait     ", args, n);
  shapeB[1] = XmCreateToggleButton(RC[1], "Landscape    ", args, n);
  XtManageChildren(shapeB, 2);

  n = 0;
  colorB[0] = XmCreateToggleButton(RC[2], "Color        ", args, n);
  colorB[1] = XmCreateToggleButton(RC[2], "Black & White", args, n);
  XtManageChildren(colorB, 2);

  if (printerSetup.color)
    XmToggleButtonSetState(colorB[0], True, False);
  else
    XmToggleButtonSetState(colorB[1], True, False);

  /* Command buttons.
   */
  n = 0;
  b[0] = XmCreatePushButton(RC[3], "applyButton", args, n);
  b[1] = XmCreatePushButton(RC[3], "resetButton", args, n);
  b[2] = XmCreatePushButton(RC[3], "dismissButton", args, n);
  XtAddCallback(b[0], XmNactivateCallback, ApplyPrintParms, NULL);
  XtAddCallback(b[1], XmNactivateCallback, EditPrintParms, NULL);
  XtAddCallback(b[2], XmNactivateCallback, DismissWindow, PrintWindow);
  XtManageChildren(b, 3);

}	/* END CREATEPRINTWINDOW */

/* -------------------------------------------------------------------- */
void *GetPrinterList(void *arg)
{
  FILE	*in;
  char	*p;
  int	i;

  memset(printerList, 0, sizeof(printerList));

  if ((in = popen("lpstat -v", "r")) == NULL)
    {
    fprintf(stderr, "lpstat command failed.\n");
    return(NULL);
    }

  printerList[0] = (char *)GetMemory(8);
  strcpy(printerList[0], "Default");

  for (i = 1; fgets(buffer, 1024, in) > 0; ++i)
    {
    if (i >= MAX_PRINTERS-1)
      {
      fprintf(stderr, "GetPrinterList: Maximum number of printers reached, list will be incomplete.\n");
      break;
      }

    p = strtok(buffer, " ");
    p = strtok(NULL, " ");
    p = strtok(NULL, ":");

    printerList[i] = (char *)GetMemory(strlen(p)+1);
    strcpy(printerList[i], p);
//printf("%s\n", printerList[i]);
    }

  pclose(in);

}	/* END GETPRINTERLIST */

/* END ED_PRINT.C */
