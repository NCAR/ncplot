/*
-------------------------------------------------------------------------
OBJECT NAME:	ncplot.c

FULL NAME:	NetCDF plot.

TYPE:		X11/R5 Motif 1.2 application

DESCRIPTION:	http://raf.atd.ucar.edu/Software/ncplot.html

INPUT:		Command line options

AUTHOR:		websterc@ncar.ucar.edu

COPYRIGHT:      University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include "fbr.h"

#define APP_CLASS	"XmNCplot"


Widget	AppShell;
Widget	MainShell, MainWindow = NULL;
Widget	ControlShell, ControlWindow = NULL;
Widget	SpecShell, SpectrumWindow = NULL;
Widget	ExpShell, ExpWindow = NULL;
Widget  DiffShell, DifferenceWindow = NULL;

Widget	CreateMainWindow(Widget parent);
void	CreateControlWindow(Widget parent);

static XtResource resources[] = {
	{XtNfont24, XtCFont24, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, font24), XtRString, NULL},

	{XtNfont18, XtCFont18, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, font18), XtRString, NULL},

	{XtNfont14, XtCFont14, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, font14), XtRString, NULL},

	{XtNfont12, XtCFont12, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, font12), XtRString, NULL},

	{XtNfont10, XtCFont10, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, font10), XtRString, NULL},
	};


XtAppContext	appContext;

void UpdateDataRT(XtPointer client, XtIntervalId *id);


/* --------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  int	n;
  Arg	args[6];

  Initialize();
  ProcessArgs(argv);
  ReadConfigFile();

  AutoScale();

  if (!Interactive)
    PrintPS((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
  else
    {
    AppShell = XtAppInitialize(&appContext, APP_CLASS, NULL, 0, &argc, argv,
                                fallback_resources, NULL, 0);

    XtGetApplicationResources(AppShell, (caddr_t) &iv, resources,
                                XtNumber(resources), NULL, 0);

    n = 0;
    MainShell = XtCreatePopupShell("topLevelShell",
                                topLevelShellWidgetClass, AppShell, args, n);

    MainWindow = CreateMainWindow(MainShell);

    n = 0;
    ControlShell = XtCreatePopupShell("controlShell",
                                topLevelShellWidgetClass, AppShell, args, n);

    CreateControlWindow(ControlShell);

    CreateErrorBox(AppShell);
    CreateQueryBox(AppShell);
    CreateWarningBox(AppShell);
    CreateFileSelectionBox(AppShell);

    if (NumberDataFiles > 0)
      NewDataFile((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

    OpenControlWindow(NULL, NULL, NULL);
    XtManageChild(MainWindow);
    XtPopup(XtParent(MainWindow), XtGrabNone);

    if (RealTime)
      XtAppAddTimeOut(appContext, 1000, UpdateDataRT, NULL);

    XtAppMainLoop(appContext);
    }

  return(0);

}	/* END MAIN */

/* END NCPLOT.C */
