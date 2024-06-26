/*
-------------------------------------------------------------------------
OBJECT NAME:	titles.c

FULL NAME:	netCDF Titles/Category Window & callbacks

ENTRY POINTS:	ViewTitles()

STATIC FNS:	CreateTitleWindow()
		SetTitles()
		DismissTitles()
		PrintTitles()

DESCRIPTION:	Currently hardwired to only use the Category list from
		file[0].

COPYRIGHT:	University Corporation for Atmospheric Research, 1998-05
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"
#include <netcdf.h>

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

static Widget	TitleShell, TitleWindow = NULL, titleText;
static int	currentCategory;

static void
	CreateTitleWindow(), SetTitles(),
	PrintTitles(Widget w, XtPointer client, XtPointer call),
	DismissTitles(Widget w, XtPointer client, XtPointer call);

extern Widget	AppShell;


/* -------------------------------------------------------------------- */
void ViewTitles(Widget w, XtPointer client, XtPointer call)
{
  if (NumberDataFiles == 0)
    return;

  if (!TitleWindow)
    CreateTitleWindow();

  SetTitles();

  if (TitleWindow) {
    XtManageChild(TitleWindow);
    XtPopup(XtParent(TitleWindow), XtGrabNone);
    }

}	/* END VIEWTITLES */

/* -------------------------------------------------------------------- */
static void DismissTitles(Widget w, XtPointer client, XtPointer call)
{
  if (TitleWindow)
    {
    XtUnmanageChild(TitleWindow);
    XtPopdown(XtParent(TitleWindow));
    XmTextSetString(titleText, (char *)"");
    }

}	/* END DISMISSHEADER */

/* -------------------------------------------------------------------- */
static void SetCategory(Widget w, XtPointer client, XtPointer call)
{
  currentCategory = (long)client;
  SetTitles();

}	/* END SETCATEGORY */

/* -------------------------------------------------------------------- */
static void SetTitles()
{
  DATAFILE_INFO	*curFile = &dataFile[CurrentDataFile];

  XmTextSetString(titleText, const_cast<char *>(curFile->fileName.c_str()));
  XmTextInsert(titleText, XmTextGetLastPosition(titleText), (char *)"\n\n");

  for (size_t i = 0; i < curFile->Variable.size(); ++i)
    {
    VARTBL *vp = curFile->Variable[i];
    bool ok = false;

    if (currentCategory == 0)
       ok = true;
    else
      {
      std::set<std::string>::iterator it = curFile->categories.begin();
      std::advance (it, currentCategory - 1);

      if (find(vp->categories.begin(), vp->categories.end(), *it) != vp->categories.end())
        ok = true;
      }

    if (ok)
      {
      snprintf(buffer, BUFFSIZE, "%-20s %-10s %s\n",
		curFile->Variable[i]->name.c_str(),
		curFile->Variable[i]->units.c_str(),
		curFile->Variable[i]->long_name.c_str());
      XmTextInsert(titleText, XmTextGetLastPosition(titleText), buffer);
      }
    }

}	/* END SETTITLES */

/* -------------------------------------------------------------------- */
static void CreateTitleWindow()
{
  Arg		args[8];
  size_t	i, n;
  Widget	drFrame, drRC, b[3], catPD, catOpMenu, catButts[32];
  XmString	name;

  n = 0;
  TitleShell = XtCreatePopupShell("headerShell",
                  topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  TitleWindow = XmCreateForm(TitleShell, (char *)"headerForm", args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  drFrame = XmCreateFrame(TitleWindow, (char *)"buttonFrame", args, n);
  XtManageChild(drFrame);

  n = 0;
  drRC = XmCreateRowColumn(drFrame, (char *)"buttonRC", args, n);
  XtManageChild(drRC);

  n = 0;
  b[0] = XmCreatePushButton(drRC, (char *)"dismissButton", args, n);
  XtAddCallback(b[0], XmNactivateCallback, DismissTitles, TitleWindow);

  n = 0;
  b[1] = XmCreatePushButton(drRC, (char *)"printButton", args, n);
  XtAddCallback(b[1], XmNactivateCallback, PrintTitles, NULL);

  XtManageChildren(b, 2);



  /* Category Op Menu.
   */
  n = 0;
  catPD = XmCreatePulldownMenu(drRC, (char *)"catPullDown", args, n);

  n = 0;
  XtSetArg(args[n], XmNsubMenuId, catPD); ++n;
  catOpMenu = XmCreateOptionMenu(drRC, (char *)"catOpMenu", args, n);
  XtManageChild(catOpMenu);

  name = XmStringCreateLocalized((char *)"All Variables");
  n = 0;
  XtSetArg(args[n], XmNlabelString, name); ++n;
  catButts[0] = XmCreatePushButton(catPD, (char *)"opMenB", args, n);
  XtAddCallback(catButts[0], XmNactivateCallback, SetCategory, (XtPointer)0);

  XmStringFree(name);

  std::set<std::string>::iterator it = dataFile[0].categories.begin();
  for (i = 1; it != dataFile[0].categories.end(); ++it, ++i)
    {
    name = XmStringCreateLocalized(const_cast<char *>((*it).c_str()));

    n = 0;
    XtSetArg(args[n], XmNlabelString, name); ++n;
    catButts[i] = XmCreatePushButton(catPD, (char *)"opMenB", args, n);
    XtAddCallback(catButts[i], XmNactivateCallback, SetCategory, (XtPointer)i);

    XmStringFree(name);
    }

  XtManageChildren(catButts, i);



  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, drFrame); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  titleText = XmCreateScrolledText(TitleWindow, (char *)"headerText", args, n);
  XtManageChild(titleText);

}	/* END CREATETITLEWINDOW */

/* -------------------------------------------------------------------- */
static void PrintTitles(Widget w, XtPointer client, XtPointer call)
{
  FILE	*fp;
  char	*p;

  if ((p = getenv("LPDEST")) != NULL)
    printf("Output being sent to %s.\n", p);

  if ((fp = popen(const_cast<char *>(printerSetup.lpCommand.c_str()), "w")) == NULL)
    {
    ShowError("PrintTitles: can't open pipe to 'lp'");
    return;
    }

  fprintf(fp, "%s, %s\n\n",
	mainPlot[0].title.c_str(), mainPlot[0].subTitle.c_str());

  p = XmTextGetString(titleText);
  fprintf(fp, "%s\n", p);
  XtFree(p);

  pclose(fp);

}	/* END PRINTTITLES */

/* END TITLES.C */
