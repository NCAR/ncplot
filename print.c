/*
-------------------------------------------------------------------------
OBJECT NAME:	print.c

FULL NAME:	Misc print callbacks & routines

ENTRY POINTS:	PrintPS()
		SetPlotRatios()

STATIC FNS:	print_PS_CB2()
		PrintPostScript()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"
#include <unistd.h>

static char     psDirectory[256] = "*.ps";

static void	PrintPostScript(Widget w, XtPointer client, XtPointer call),
		print_PS_CB2(Widget w, XtPointer client, XtPointer call);


/* -------------------------------------------------------------------- */
void SetPlotRatios(PLOT_INFO *plot)
{
  if (printerSetup.shape == LANDSCAPE)
    {
    printerSetup.widthRatio = printerSetup.height / 11.0;
    printerSetup.heightRatio = printerSetup.width / 8.5;

    plot->ps.windowWidth = (int)(printerSetup.dpi * printerSetup.height);
    plot->ps.windowHeight = (int)(printerSetup.dpi * printerSetup.width);

    plot->ps.titleOffset      = (int)(2250 * printerSetup.heightRatio);
    plot->ps.subTitleOffset   = (int)(2170 * printerSetup.heightRatio);
    }
  else
    {
    printerSetup.widthRatio = printerSetup.width / 8.5;
    printerSetup.heightRatio = printerSetup.height / 11.0;

    plot->ps.windowWidth = (int)(printerSetup.dpi * printerSetup.width);
    plot->ps.windowHeight = (int)(printerSetup.dpi * printerSetup.height);

    plot->ps.titleOffset      = (int)(3000 * printerSetup.heightRatio);
    plot->ps.subTitleOffset   = (int)(2920 * printerSetup.heightRatio);
    }

  printerSetup.fontRatio =
     std::max((printerSetup.heightRatio + printerSetup.widthRatio) / 2, 0.6);

}	/* END SETPLOTRATIOS */

/* -------------------------------------------------------------------- */
void PrintPS(Widget w, XtPointer client, XtPointer call)
{
  if (NumberDataFiles == 0)
    {
    HandleError("No data.", Interactive, RETURN);
    return;
    }

  if (client)
    QueryFile("Enter PostScript output file name:", psDirectory, print_PS_CB2);
  else
    {
    outFile = NULL;
    PrintPostScript((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
    }

}	/* END PRINTPS */

/* -------------------------------------------------------------------- */
static void print_PS_CB2(Widget w, XtPointer client, XtPointer call)
{
  FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
  ExtractFileName(((XmFileSelectionBoxCallbackStruct *)call)->value, &outFile);

  if (access(outFile, F_OK) == 0)
    {
    sprintf(buffer, "Overwrite file %s", outFile);
    WarnUser(buffer, PrintPostScript, NULL);
    }
  else
    PrintPostScript((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);

}	/* END PRINT_PS_CB2 */

/* -------------------------------------------------------------------- */
static void PrintPostScript(Widget w, XtPointer client, XtPointer call)
{
  bool	saveState = Freeze;
  char	*p;

  Freeze = True;
  WaitCursorAll();

  if (call)
    CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

  if (outFile)
    {
    strcpy(psDirectory, outFile);
    if ((p = strrchr(psDirectory, '/')))
      strcpy(p+1, "*.ps");
    }

  switch (PlotType)
    {
    case TIME_SERIES:
      if (NumberDataSets > 0)
        PrintTimeSeries();
      break;

    case XY_PLOT:
      if (NumberXYXsets > 0 && NumberXYYsets > 0)
        PrintXY();
      break;

    case XYZ_PLOT:
      if (xyzSet[0].varInfo && xyzSet[1].varInfo && xyzSet[2].varInfo)
        PrintXYZ();
      break;
    }

  Freeze = saveState;

  PointerCursorAll();

}  /* END PRINTPOSTSCRIPT */

/* END PRINT.C */
