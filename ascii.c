/*
-------------------------------------------------------------------------
OBJECT NAME:	ascii.c

FULL NAME:	ASCII Window & callbacks

ENTRY POINTS:	ViewASCII()
		SetASCIIdata()

STATIC FNS:	CreateASCIIwindow()
		DismissASCII()
		SaveASCII()
		Save_OK()
		PrintASCII()
		formatTitle()
		formatLine()
		specASCII()

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	Callback, Xplot.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"
#include "ps.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

static Widget ASCIIshell, ASCIIwindow, asciiText;

static void
  CreateASCIIwindow(),
  SaveASCII(Widget w, XtPointer client, XtPointer call),
  Save_OK(Widget w, XtPointer client, XmFileSelectionBoxCallbackStruct *call),
  PrintASCII(Widget w, XtPointer client, XtPointer call),
  DismissASCII(Widget w, XtPointer client, XtPointer call),
  timeDomainASCII(FILE *fp, int n), freqDomainASCII(FILE *fp, int n);

static char *formatTitle(char *);
static char *formatLine(char *, int, int, int, int, int);

extern Widget	AppShell;


/* -------------------------------------------------------------------- */
static void DismissASCII(Widget w, XtPointer client, XtPointer call)
{
  AsciiWinOpen = False;

  if (ASCIIwindow)
    {
    XtUnmanageChild(ASCIIwindow);
    XtPopdown(XtParent(ASCIIwindow));
    }

}	/* END DISMISSASCII */

/* -------------------------------------------------------------------- */
void ViewASCII(Widget w, XtPointer client, XtPointer call)
{
  SetASCIIdata(NULL, NULL, NULL);

  if (ASCIIwindow)
    {
    XtManageChild(ASCIIwindow);
    XtPopup(XtParent(ASCIIwindow), XtGrabNone);
    AsciiWinOpen = True;
    }

}	/* END VIEWASCII */

/* -------------------------------------------------------------------- */
void SetASCIIdata(Widget w, XtPointer client, XtPointer call)
{
  static bool	firstTime = True;

  if (NumberDataSets == 0)
    {
    if (AsciiWinOpen)
      DismissASCII(NULL, NULL, NULL);

    return;
    }

  if (firstTime) {
    CreateASCIIwindow();
    firstTime = False;
    }


  /* If spectral window is open, then we display frequency domain ASCII else
   * time domain output.
   */
  if (specPlot.windowOpen)
    freqDomainASCII(NULL, psd[0].M);
  else
    timeDomainASCII(NULL, std::min(nASCIIpoints, dataSet[0].nPoints));

}	/* END SETASCIIDATA */

/* -------------------------------------------------------------------- */
static void CreateASCIIwindow()
{
  Arg         args[32];
  Cardinal    n;
  Widget      drFrame, drRC, b[3];

  n = 0;
  ASCIIshell = XtCreatePopupShell("asciiShell",
                topLevelShellWidgetClass, AppShell, args, n);

  n = 0;
  ASCIIwindow = XmCreateForm(ASCIIshell, "asciiForm", args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  drFrame = XmCreateFrame(ASCIIwindow, "buttonFrame", args, n);
  XtManageChild(drFrame);

  n = 0;
  drRC = XmCreateRowColumn(drFrame, "buttonRC", args, n);
  XtManageChild(drRC);


  n = 0;
  b[0] = XmCreatePushButton(drRC, "saveButton", args, n);
  XtAddCallback(b[0], XmNactivateCallback, SaveASCII, NULL);

  n = 0;
  b[1] = XmCreatePushButton(drRC, "printButton", args, n);
  XtAddCallback(b[1], XmNactivateCallback, PrintASCII, NULL);

  n = 0;
  b[2] = XmCreatePushButton(drRC, "dismissButton", args, n);
  XtAddCallback(b[2], XmNactivateCallback, DismissASCII, ASCIIwindow);

  XtManageChildren(b, 3);


  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, drFrame); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  asciiText = XmCreateScrolledText(ASCIIwindow, "asciiText", args, n);
  XtManageChild(asciiText);

}	/* END CREATEASCIIWINDOW */

/* -------------------------------------------------------------------- */
static void SaveASCII(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

  if ((p = getenv("DATA_DIR")) == NULL)
    if ((p = getenv("HOME")) == NULL)
      p = "";

  sprintf(buffer, "%s/*", p);
  QueryFile("Enter file name to save:", buffer, (XtCallbackProc)Save_OK);

}

/* -------------------------------------------------------------------- */
static void Save_OK(Widget w, XtPointer client, XmFileSelectionBoxCallbackStruct *call)
{
  FILE	*fp;
  char	*file;

  ExtractFileName(call->value, &file);
  FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

  if ((fp = fopen(file, "w")) == NULL) {
    sprintf(buffer, "Save: can't open %s.", file);
    ShowError(buffer);
    return;
    }

  /* If spectral window is open, then we display frequency domain ASCII else
   * time domain output.
   */
  if (specPlot.windowOpen)
    freqDomainASCII(fp, psd[0].M);
  else
    timeDomainASCII(fp, dataSet[0].nPoints);

  fclose(fp);

}	/* END SAVE_OK */

/* -------------------------------------------------------------------- */
static void PrintASCII(Widget w, XtPointer client, XtPointer call)
{
  FILE	*fp;
  char	*p;

#ifdef SVR4
  if ((p = getenv("LPDEST")) != NULL)
#else
  if ((p = getenv("PRINTER")) != NULL)
#endif
    printf("Output being sent to %s.\n", p);

  if ((fp = popen(printerSetup.lpCommand.c_str(), "w")) == NULL)
    {
    sprintf(buffer, "PrintASCII: can't open pipe to '%s'", printerSetup.lpCommand.c_str());
    ShowError(buffer);
    return;
    }


  /* If spectral window is open, then we display frequency domain ASCII else
   * time domain output.
   */
  if (specPlot.windowOpen)
    freqDomainASCII(fp, psd[0].M);
  else
    timeDomainASCII(fp, std::min((size_t)60, dataSet[0].nPoints));

  pclose(fp);

}	/* END PRINTASCII */


/* -------------------------------------------------------------------- */
static void timeDomainASCII(FILE *fp, int nPoints)
{
  int	i, hour, min, sec, msec;
  int	msecCnt;

  if (fp)
    fprintf(fp, formatTitle(buffer));
  else
    XmTextSetString(asciiText, formatTitle(buffer));


  hour = UserStartTime[0];
  min = UserStartTime[1];
  sec = UserStartTime[2];
  msec = 0;

  if (hour > 23)
    hour = 0;

  if (dataSet[0].nPoints < NumberSeconds)
    msecCnt = 1000;
  else
    msecCnt = 1000 / (dataSet[0].nPoints / NumberSeconds);

  for (i = 0; i < nPoints; ++i)
    {
    formatLine(buffer, i, hour, min, sec, msec);

    if (fp)
      fprintf(fp, buffer);
    else
      XmTextInsert(asciiText, XmTextGetLastPosition(asciiText), buffer);

    if ((msec += msecCnt) > 999) {
      msec = 0;
      if (++sec > 59) {
        sec = 0;
        if (++min > 59) {
          min = 0;
          if (++hour > 23)
            hour = 0;
          }
        }
      }
    }

}	/* END TIMEDOMAINASCII */

/* -------------------------------------------------------------------- */
static void freqDomainASCII(FILE *fp, int nPoints)
{
  int		i, set, nSets = 1;
  double	*dataP = 0;

  if (!fp)
    XmTextSetString(asciiText, "");

  if (psd[0].display == SPECTRA)
    nSets = std::min(NumberDataSets, MAX_PSD);

  for (set = 0; set <= nSets; ++set)
    {
    switch (psd[0].display)
      {
      case SPECTRA:
      case COSPECTRA:
        dataP = psd[set].Pxx;
        break;

      case QUADRATURE:
        dataP = psd[set].Qxx;
        break;

      case COHERENCE:
      case PHASE:
      case RATIO:
        dataP = psd[set].Special;
        break;
      }


    for (i = 0; i <= nPoints; ++i)
      {
      if (equalLogInterval())
        sprintf(buffer, "%14f %14e\n", psd[set].ELIAx[i], psd[set].ELIAy[i]);
      else
        sprintf(buffer, "%14f %14e\n", psd[set].freqPerBin * i, dataP[i]);

      if (fp)
        fprintf(fp, buffer);
      else
        XmTextInsert(asciiText, XmTextGetLastPosition(asciiText), buffer);
      }
    }

}	/* END FREQDOMAINASCII */

/* -------------------------------------------------------------------- */
static char *formatTitle(char buff[]) 
{
  int		lrOffset, varCnt = 0;
  VARTBL	*vp;

  lrOffset = (dataSet[0].nPoints == NumberSeconds) ? 16 : 19;
  memset(buff, ' ', 20 * (NumberDataSets+1));
  memcpy(buff, "UTC", 3);
 
  for (size_t i = 0; i < NumberDataSets; ++i)
    {
    vp = dataSet[i].varInfo;

    if (dataSet[i].nPoints == dataSet[0].nPoints)
      memcpy(&buff[lrOffset+(14*varCnt++)], vp->name.c_str(), vp->name.size());
    }
 
  strcpy(&buff[lrOffset+(varCnt*14)], "\n");

  return(buff);

}	/* END FORMATTITLE */

/* -------------------------------------------------------------------- */
static char *formatLine(
	char	buff[],				/* Output buffer	*/
	int	indx,				/* Record index		*/
	int	hour, int min, int sec, int msec) /* Time stamp		*/
{
  size_t i;
  char	tempBuff[32];
  bool	intData;

  if (UTCseconds)
    {
    if (dataSet[0].nPoints <= NumberSeconds)
      sprintf(buff, "%05d   ", hour*3600 + min*60 + sec);
    else
      sprintf(buff, "%05d.%03d   ", hour*3600 + min*60 + sec, msec);
    }
  else
    {
    if (dataSet[0].nPoints <= NumberSeconds)
      sprintf(buff, "%02d:%02d:%02d   ", hour, min, sec);
    else
      sprintf(buff, "%02d:%02d:%02d.%03d   ", hour, min, sec, msec);
    }

  intData =	strchr(asciiFormat, 'd') || strchr(asciiFormat, 'x') ||
		strchr(asciiFormat, 'u');

  for (i = 0; i < NumberDataSets; ++i)
    {
    if (dataSet[i].nPoints == dataSet[0].nPoints)
      {
      if (intData)
        sprintf(tempBuff, asciiFormat, (int)dataSet[i].data[(dataSet[i].head + indx) % dataSet[i].nPoints]);
      else
        sprintf(tempBuff, asciiFormat, dataSet[i].data[(dataSet[i].head + indx) % dataSet[i].nPoints]);
      strcat(buff, tempBuff);
      }
    }

  strcat(buff, "\n");

  return(buff);

}	/* FORMATLINE */

/* END ASCII.C */
