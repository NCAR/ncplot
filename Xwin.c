/*
-------------------------------------------------------------------------
OBJECT NAME:	Xwin.c

FULL NAME:	X window Stuff

ENTRY POINTS:	CreateMainWindow()
		createParamsTitles()
		createPanelButts()
		createParamsLabels()
		createParamsMinMax()
		createParamsTics()
		createLogInvert()
		createARDbuttons()

STATIC FNS:	createLineItem()
		createLineItemLog()
		createLineItemInvert()
		createLineItemTB()

DESCRIPTION:	This creates the initial X window, menus and all associated
		widgets.  Also calls CreateFile(), CreateError(),
		CreateQuery().

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>


struct menu
	{
	char		*title;
	XtCallbackProc	callback;
	XtPointer	callData;
	} ;

static struct menu	fileMenu[] = {
	{ "newData", GetDataFileName, (XtPointer)NewDataFile },
	{ "addData", GetDataFileName, (XtPointer)AddDataFile },
	{ "seperator", NULL, NULL },
	{ "loadTemplate", LoadTemplate, (XtPointer)NULL },
	{ "saveTemplate", SaveTemplate, (XtPointer)NULL },
	{ "seperator", NULL, NULL },
	{ "printSetup", EditPrintParms, (XtPointer)NULL },
	{ "savePS", PrintPS, (XtPointer)1 },
	{ "print", PrintPS, NULL },
#ifdef PNG
	{ "seperator", NULL, NULL },
	{ "savePNG", SavePNG, NULL },
#endif
	{ "seperator", NULL, NULL },
	{ "quit", Quit, NULL },
	{ NULL, NULL, NULL }};

static struct menu	legendMenu[] = {
	{ "editParms", EditMainParms, NULL },
	{ "editXY", EditXYParms, NULL },
	{ "editXYZ", EditTrackParms, NULL },
	{ "seperator", NULL, NULL },
	{ "editDiff", EditDiffParms, NULL },
	{ "editStats", EditStatsParms, NULL },
	{ "editSpec", EditSpecParms, NULL },
	{ "seperator", NULL, NULL },
	{ "editPrefer", EditPreferences, NULL },
	{ NULL, NULL, NULL }};

static struct menu	viewMenu[] = {
	{ "viewASCII", ViewASCII, NULL },
	{ "viewDiff", DiffWinUp, NULL },
	{ "viewStats", ViewStats, NULL },
	{ "viewTitles", ViewTitles, NULL },
	{ "viewHeader", ViewHeader, NULL },
	{ "seperator", NULL, NULL },
	{ "viewSpectra", SpecWinUp, (XtPointer)SPECTRA },
	{ "viewCoSpectra", SpecWinUp, (XtPointer)COSPECTRA },
	{ "viewQuadrature", SpecWinUp, (XtPointer)QUADRATURE },
	{ "viewCoherence", SpecWinUp, (XtPointer)COHERENCE },
	{ "viewPhase", SpecWinUp, (XtPointer)PHASE },
	{ "viewRatio", SpecWinUp, (XtPointer)RATIO },
	{ NULL, NULL, NULL }};

static struct menu	optionMenu[] = {
	{ "unZoom", (XtCallbackProc)UnZoom, NULL },
	{ "clearPlot", ClearPlot, NULL },
	{ "getExp", GetExpression, NULL },
	{ "seperator", NULL, NULL },
	{ "clearRegret", ClearRegression, NULL },
	{ "linearRegret", LinearRegression, NULL },
	{ "polyRegret", PolyRegression, NULL },
	{ NULL, NULL, NULL }};

static struct menu  helpMenu[] = {
	{ "RAF Homepage", ForkNetscape, (XtPointer)1 },
	{ "RAF Software Page", ForkNetscape, (XtPointer)2 },
	{ "ncplot User's Manual", ForkNetscape, (XtPointer)3 },
	{ NULL, NULL, NULL }};

static struct
	{
	char		*title;
	struct menu	*sub;
	} main_menu[] = {
		{ "File", fileMenu, },
		{ "Edit", legendMenu, },
		{ "View", viewMenu, },
		{ "Options", optionMenu, },
		{ "Help", helpMenu, },
		{ NULL, NULL }};



static char annotateBits[] = {
   0x80, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00,
   0x80, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00,
   0xe0, 0x70, 0x00, 0xe0, 0x70, 0x00, 0x70, 0xe0, 0x00, 0xf0, 0xff, 0x00,
   0xf8, 0xff, 0x01, 0xf8, 0xff, 0x01, 0x3c, 0xc0, 0x03, 0x1c, 0x80, 0x03,
   0x0e, 0x00, 0x07, 0x0e, 0x00, 0x07, 0x1f, 0x80, 0x0f, 0x1f, 0x80, 0x0f};

static char pointerBits[] = {
   0x03, 0x00, 0x00, 0x3f, 0x00, 0x00, 0xfe, 0x03, 0x00, 0xfe, 0x1f, 0x00,
   0xfe, 0x3f, 0x00, 0xfe, 0x1f, 0x00, 0xfc, 0x0f, 0x00, 0xfc, 0x07, 0x00,
   0xfc, 0x07, 0x00, 0xfc, 0x0f, 0x00, 0xf8, 0x1f, 0x00, 0x78, 0x3e, 0x00,
   0x38, 0x7c, 0x00, 0x10, 0xf8, 0x00, 0x00, 0xf0, 0x01, 0x00, 0xe0, 0x03,
   0x00, 0xc0, 0x07, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x06};

static char grabBits[] = {
   0x00, 0x04, 0x00, 0x40, 0x4a, 0x00, 0xa0, 0xaa, 0x04, 0xa0, 0xaa, 0x0a,
   0xa0, 0xaa, 0x0a, 0xa0, 0xaa, 0x0a, 0xa2, 0xaa, 0x0a, 0xa7, 0xaa, 0x0a,
   0xa5, 0xaa, 0x0a, 0xad, 0xaa, 0x0a, 0xa9, 0xbb, 0x0a, 0x19, 0x11, 0x09,
   0x11, 0x00, 0x04, 0x02, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00, 0x04,
   0x38, 0x00, 0x04, 0xc0, 0x00, 0x04, 0x80, 0x00, 0x04, 0x80, 0x00, 0x04};

void	PointerCursorWrap(Widget, XtPointer, XtPointer),
	TextCursorWrap(Widget, XtPointer, XtPointer);

/* -------------------------------------------------------------------- */
Widget CreateMainWindow(Widget parent)
{
  Widget	menubar, form, menu[6], bttn[50], menu_button[6], form1;
  Arg		args[9];
  int		n, i, j, fg, bg;
  Pixmap	pixmap;

  n = 0;
  form = XmCreateForm(parent, "mainForm", args, n);

  n = 0;
  menubar = XmCreateMenuBar(form, "menuBar", args, n);
  XtManageChild(menubar);


  for (i = 0; main_menu[i].title; ++i)
    {
    n = 0;
    menu[i] = XmCreatePulldownMenu(menubar, main_menu[i].title, args, n);

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, menu[i]); ++n;
    menu_button[i] = XmCreateCascadeButton(menubar, main_menu[i].title, args,n);

    for (j = 0; main_menu[i].sub[j].title; ++j)
      {
      n = 0;

      if (main_menu[i].sub[j].callback == NULL)
        {
        bttn[j] = XmCreateSeparator(menu[i], main_menu[i].sub[j].title, args,n);
        continue;
        }

      bttn[j] = XmCreatePushButton(menu[i], main_menu[i].sub[j].title, args, n);
      XtAddCallback(bttn[j], XmNactivateCallback,
              main_menu[i].sub[j].callback, main_menu[i].sub[j].callData);
      }

    XtManageChildren(bttn, j);
    }

  XtManageChildren(menu_button, i);


  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNverticalSpacing, 0); n++;
  XtSetArg(args[n], XmNhorizontalSpacing, 0); n++;
  form1 = XmCreateForm(form, "drawForm", args, n);
  XtManageChild(form1);

  XtVaGetValues(form1, XmNforeground, &fg, XmNbackground, &bg, NULL);


  /* Create Pointer, Annotate, and Grab pixmap buttons.
   */
  if (!RealTime)
    {
    n = 0;
    pixmap = XCreatePixmapFromBitmapData(XtDisplay(form1),
           RootWindowOfScreen(XtScreen(form1)),
           pointerBits, 20, 20, fg, bg, DefaultDepthOfScreen(XtScreen(form1)));

    XtSetArg(args[n], XmNlabelType, XmPIXMAP); n++;
    XtSetArg(args[n], XmNlabelPixmap, pixmap); n++;

    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;

    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    bttn[0] = XmCreatePushButton(form1, "dB", args, n);

    XtSetArg(args[n-1], XmNleftAttachment, XmATTACH_WIDGET);
    XtSetArg(args[n], XmNleftWidget, bttn[0]); ++n;
    pixmap = XCreatePixmapFromBitmapData(XtDisplay(form1),
           RootWindowOfScreen(XtScreen(form1)),
           annotateBits, 20, 20, fg, bg, DefaultDepthOfScreen(XtScreen(form1)));
    XtSetArg(args[1], XmNlabelPixmap, pixmap);
    bttn[1] = XmCreatePushButton(form1, "dB", args, n);

    XtSetArg(args[n-1], XmNleftWidget, bttn[1]);
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    pixmap = XCreatePixmapFromBitmapData(XtDisplay(form1),
           RootWindowOfScreen(XtScreen(form1)),
           grabBits, 20, 20, fg, bg, DefaultDepthOfScreen(XtScreen(form1)));
    XtSetArg(args[1], XmNlabelPixmap, pixmap);
    bttn[2] = XmCreatePushButton(form1, "dB", args, n);
    XtManageChildren(bttn, 2);
    }


  /* Create Graphics Canvas
   */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  mainPlot[0].canvas = XmCreateDrawingArea(form, "canvas", args, n);

  XtAddCallback(mainPlot[0].canvas, XmNexposeCallback,
                (XtCallbackProc)ExposeMainWindow, (XtPointer)NULL);

  XtAddCallback(mainPlot[0].canvas, XmNresizeCallback,
                (XtCallbackProc)ResizeMainWindow, NULL);

  if (!RealTime)
    {
    XtAddCallback(mainPlot[0].canvas, XmNinputCallback,
		(XtCallbackProc)CanvasInput, NULL);

    XtAddCallback(bttn[0], XmNactivateCallback, PointerCursorWrap, mainPlot[0].canvas);
    XtAddCallback(bttn[1], XmNactivateCallback, TextCursorWrap, mainPlot[0].canvas);
/*    XtAddCallback(bttn[2], XmNactivateCallback, GrabCursorWrap, mainPlot[0].canvas); */
    }

  XtManageChild(mainPlot[0].canvas);

  return(form);

}	/* END CREATEMAINWINDOW */

/* -------------------------------------------------------------------- */
static Widget createLineItem(Widget parent, Widget *parmsText, const char lab[], const char txt[])
{
  Widget	RC, label;

  RC = XmCreateRowColumn(parent, "plRC", NULL, 0);

  label = XmCreateLabel(RC, const_cast<char *>(lab), NULL, 0);
  *parmsText = XmCreateTextField(RC, const_cast<char *>(txt), NULL, 0);
  XtManageChild(label);
  XtManageChild(*parmsText);

  return(RC);

}	/* END CREATELINEITEM */

/* -------------------------------------------------------------------- */
static void createLineItemTB(Widget RC, Widget parms[], int axies)
{
  if (axies & X_AXIS)
    {
    parms[0] = XmCreateToggleButton(RC, "X axis", NULL, 0);
    XtManageChild(parms[0]);
    }

  parms[1] = XmCreateToggleButton(RC, "Y left", NULL, 0);
  XtManageChild(parms[1]);

  parms[2] = XmCreateToggleButton(RC, "Y right", NULL, 0);
  XtManageChild(parms[2]);

}	/* END CREATELINEITEMLOG */

/* -------------------------------------------------------------------- */
static Widget createLineItemLog(Widget parent, Widget *parms, int axies)
{
  Widget	RC, label;

  RC = XmCreateRowColumn(parent, "plRC", NULL, 0);

  label = XmCreateLabel(RC, "Log scale :", NULL, 0);
  XtManageChild(label);

  createLineItemTB(RC, parms, axies);

  return(RC);

}	/* END CREATELINEITEMLOG */

/* -------------------------------------------------------------------- */
static Widget createLineItemInvert(Widget parent, Widget *parms, int axies)
{
  Widget	RC, label;

  RC = XmCreateRowColumn(parent, "plRC", NULL, 0);

  label = XmCreateLabel(RC, "Invert scale :", NULL, 0);
  XtManageChild(label);

  createLineItemTB(RC, parms, axies);

  return(RC);

}	/* END CREATELINEITEMINVERT */

/* -------------------------------------------------------------------- */
Widget createParamsTitles(Widget parent, Widget parmsText[])
{
  int		n;
  Arg		args[3];
  Widget	frame, RC, plRC[2];

  n = 0;
  frame = XmCreateFrame(parent, "titlesFrame", args, n);
  XtManageChild(frame);
 
  n = 0;
  RC = XmCreateRowColumn(frame, "titlesRC", args, n);
 

  plRC[0] = createLineItem(RC, &parmsText[0], "Title", "titleTxt");
  plRC[1] = createLineItem(RC, &parmsText[1], "Subtitle", "subTitleTxt");

  XtManageChildren(plRC, 2);

  return(RC);	/* In case we want to add more stuff	*/

}	/* END CREATEPARAMSTITLES */

/* -------------------------------------------------------------------- */
void createPanelButts(Widget parent, std::vector<Widget>& panelB, XtCallbackProc set)
{
  Widget	plRC, label;
  Arg		args[2];
  Cardinal	n;

  /* Panel stuff.
   */
  n = 0;
  plRC = XmCreateRowColumn(parent, "plRC", args, n);
  XtManageChild(plRC);
 
  label = XmCreateLabel(plRC, "Panel", args, n);
  plRC = XmCreateRadioBox(plRC, "pnRC", args, n);
  XtManageChild(label);
  XtManageChild(plRC);
 
  for (size_t i = 0; i < MAX_PANELS; ++i)
    {
    sprintf(buffer, "%d", i+1);
    panelB.push_back(XmCreateToggleButton(plRC, buffer, NULL, 0));
 
    XtAddCallback(panelB[i], XmNvalueChangedCallback, set, (XtPointer)i);
    }
 
  XtManageChildren(&panelB[0], MAX_PANELS);
  XmToggleButtonSetState(panelB[0], True, False);

}	/* END CREATEPANELBUTTS */

/* -------------------------------------------------------------------- */
Widget createParamsLabels(Widget parent, Widget parmsText[], PLOT_INFO *plot)
{
  int		n, i;
  Arg		args[2];
  Widget	frame, RC, plRC[3];

  n = 0;
  frame = XmCreateFrame(parent, "AxiesFrame", args, n);
  XtManageChild(frame);

  n = 0;
  RC = XmCreateRowColumn(frame, "AxiesRC", args, n);

  i = 0;

  /* Axies labels.
   */
  plRC[i++] = createLineItem(RC, &parmsText[2], "xLabel", "labelTxt");
  plRC[i++] = createLineItem(RC, &parmsText[3], "yLabel0", "labelTxt");

  if (plot->plotType == XYZ_PLOT)
    plRC[i++] = createLineItem(RC, &parmsText[4], "zLabel", "labelTxt");
  else
    plRC[i++] = createLineItem(RC, &parmsText[4], "yLabel1", "labelTxt");

  XtManageChildren(plRC, i);

  return(RC);

}	/* END CREATEPARAMSLABELS */

/* -------------------------------------------------------------------- */
Widget createParamsMinMax(Widget parent, Widget parmsText[], PLOT_INFO *plot, Widget *autoBut)
{
  int		i, n;
  Arg		args[2];
  Widget	frame, RC, plRC[3], label;

  n = 0;
  frame = XmCreateFrame(parent, "AxiesFrame", args, n);
  XtManageChild(frame);

  n = 0;
  RC = XmCreateRowColumn(frame, "AxiesRC", args, n);

  i = 0;

  /* Auto button.
   */
  n = 0;
  *autoBut = XmCreateToggleButton(RC, "Auto Scale", args, n);
  XtManageChild(*autoBut);
  XmToggleButtonSetState(*autoBut, True, False);

  /* Mins & Maxs
   */
  if (plot->plotType != TIME_SERIES)
    {
    n = 0;
    plRC[i] = createLineItem(RC, &parmsText[5], "xMin", "minMaxTxt");

    label = XmCreateLabel(plRC[i], "xMax", args, n);
    XtManageChild(label);
    parmsText[6] = XmCreateTextField(plRC[i], "minMaxTxt", args, n);
    XtManageChild(parmsText[6]);
    ++i;
    }

  n = 0;
  plRC[i] = createLineItem(RC, &parmsText[7], "yMin", "minMaxTxt");

  label = XmCreateLabel(plRC[i], "yMax", args, n);
  XtManageChild(label);
  parmsText[8] = XmCreateTextField(plRC[i], "minMaxTxt", args, n);
  XtManageChild(parmsText[8]);
  ++i;

  if (plot->plotType == XYZ_PLOT)
    {
    n = 0;
    plRC[i] = createLineItem(RC, &parmsText[9], "zMin", "minMaxTxt");

    label = XmCreateLabel(plRC[i], "zMax", args, n);
    XtManageChild(label);
    parmsText[10] = XmCreateTextField(plRC[i], "minMaxTxt", args, n);
    XtManageChild(parmsText[10]);
    }
  else
    {
    n = 0;
    plRC[i] = createLineItem(RC, &parmsText[9], "yMin", "minMaxTxt");

    label = XmCreateLabel(plRC[i], "yMax", args, n);
    XtManageChild(label);
    parmsText[10] = XmCreateTextField(plRC[i], "minMaxTxt", args, n);
    XtManageChild(parmsText[10]);
    }

  ++i;


  XtManageChildren(plRC, i);

  i = (plot->plotType == TIME_SERIES) ? 7 : 5;
  n = 11;
  for (; i < n; ++i)
    {
    XtSetSensitive(parmsText[i], False);
    XtAddCallback(parmsText[i], XmNlosingFocusCallback, ValidateFloat, (XtPointer)"%g");
    }

  return(RC);

}	/* END CREATEPARAMSMINMAX */

/* -------------------------------------------------------------------- */
Widget createParamsTics(Widget parent, Widget parmsText[], PLOT_INFO *plot, Widget *autoBut)
{
  int		i, n;
  Arg		args[2];
  Widget	frame, RC, plRC[3], label;

  n = 0;
  frame = XmCreateFrame(parent, "AxiesFrame", args, n);
  XtManageChild(frame);

  n = 0;
  RC = XmCreateRowColumn(frame, "AxiesRC", args, n);

  i = 0;

  /* Auto button.
   */
  n = 0;
  *autoBut = XmCreateToggleButton(RC, "Auto Tics", args, n);
  XtManageChild(*autoBut);
  XmToggleButtonSetState(*autoBut, True, False);

  /* # Tic marks.
   */
  n = 0;
  plRC[i] = createLineItem(RC, &parmsText[11], "xTicMaj", "ticTxt");

  label = XmCreateLabel(plRC[i], "xTicMin", args, n);
  XtManageChild(label);
  parmsText[12] = XmCreateTextField(plRC[i], "ticTxt", args, n);
  XtManageChild(parmsText[12]);
  ++i;

  n = 0;
  plRC[i] = createLineItem(RC, &parmsText[13], "yTicMaj", "ticTxt");

  label = XmCreateLabel(plRC[i], "yTicMin", args, n);
  XtManageChild(label);
  parmsText[14] = XmCreateTextField(plRC[i], "ticTxt", args, n);
  XtManageChild(parmsText[14]);
  ++i;

  if (plot->plotType == XYZ_PLOT)
    {
    n = 0;
    plRC[i] = createLineItem(RC, &parmsText[15], "zTicMaj", "ticTxt");

    label = XmCreateLabel(plRC[i], "zTicMin", args, n);
    XtManageChild(label);
    parmsText[16] = XmCreateTextField(plRC[i], "ticTxt", args, n);
    XtManageChild(parmsText[16]);
    ++i;
    }

  XtManageChildren(plRC, i);

  n = (plot->plotType == XYZ_PLOT) ? 17 : 15;
  for (i = 11; i < n; ++i)
    XtAddCallback(parmsText[i], XmNlosingFocusCallback, ValidateInteger, NULL);

  return(RC);

}	/* END CREATEPARAMSTICS */

/* -------------------------------------------------------------------- */
Widget createLogInvert(Widget parent, Widget parmsTB[], XtCallbackProc apply,
	PLOT_INFO *plot, int axies)
{
  int		n, i;
  Arg		args[2];
  Widget	frame, RC, plRC[2];

  n = 0;
  frame = XmCreateFrame(parent, "AxiesFrame", args, n);
  XtManageChild(frame);

  n = 0;
  RC = XmCreateRowColumn(frame, "AxiesRC", args, n);

  for (i = 0; i < 6; ++i)
    parmsTB[i] = NULL;

  n = 0;
  plRC[0] = createLineItemLog(RC, parmsTB, axies);
  plRC[1] = createLineItemInvert(RC, &parmsTB[3], axies);

  XtManageChildren(plRC, 2);

  for (i = 0; i < 6; ++i)
    if (parmsTB[i])
      XtAddCallback(parmsTB[i], XmNvalueChangedCallback, apply, NULL);

  return(RC);

}	/* END CREATELOGINVERT */

/* -------------------------------------------------------------------- */
Widget createARDbuttons(Widget parent)
{
  int		n;
  Arg		args[3];
  Widget	frame, RC, b[5];

  /* Command buttons.
   */
  n = 0;
  frame = XmCreateFrame(parent, "buttonFrame", args, 0);
  XtManageChild(frame);

  n = 0;
  RC = XmCreateForm(frame, "buttonRC", args, n);

  n = 0;
  b[0] = XmCreatePushButton(RC, "dismissButton", args, n);
  XtAddCallback(b[0], XmNactivateCallback, DismissWindow, parent);

  XtManageChildren(b, 1);

  return(RC);

}	/* END CREATEARDBUTTONS */

/* END XWIN.C */
