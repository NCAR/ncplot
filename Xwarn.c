/*
-------------------------------------------------------------------------
OBJECT NAME:	Xwarn.c

FULL NAME:	Show Warning Box with Message

DESCRIPTION:	CreateWarn should be called once where you initialize
		your X stuff.  To use just call WarnUser(ErrMsg, callBack)

INPUT:		String to Display.  Callback to call if user hits OK.

OUTPUT:		Warning message in its own tidy little window.

AUTHOR:		websterc@ncar
-------------------------------------------------------------------------
*/

#include <cstdio>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>

static const int nWarnings = 3;

static Widget	warnBox[nWarnings];
static int	inUse[nWarnings];

void CancelWarning(Widget, XtPointer, XtPointer);

/* -------------------------------------------------------------------- */
void WarnUser(char str[], XtCallbackProc okCB, XtCallbackProc cancelCB)
{
  int		i;
  Widget	label;
  Arg		args[5];
  XmString	xStr;

  for (i = 0; i < nWarnings; ++i)
    if (inUse[i] == False)
      break;

  if (i == nWarnings) {
    fprintf(stderr, "WarnUser: Out of warning boxes.\n");
    i = 0;
    }

  inUse[i] = True;

  label = XmMessageBoxGetChild(warnBox[i], XmDIALOG_MESSAGE_LABEL);
  xStr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[0], XmNlabelString, xStr);
  XtSetValues(label, args, 1);
  XmStringFree(xStr);

  XtRemoveAllCallbacks(warnBox[i], XmNokCallback);
  XtAddCallback(warnBox[i], XmNokCallback, CancelWarning, (XtPointer)i);

  XtRemoveAllCallbacks(warnBox[i], XmNcancelCallback);
  XtAddCallback(warnBox[i],XmNcancelCallback,CancelWarning,(XtPointer)i);

  if (okCB)
    XtAddCallback(warnBox[i], XmNokCallback, (XtCallbackProc)okCB,(XtPointer)i);
  if (cancelCB)
    XtAddCallback(warnBox[i], XmNcancelCallback, cancelCB, (XtPointer)i);

  XtManageChild(warnBox[i]);
  XtPopup(XtParent(warnBox[i]), XtGrabNone);

}	/* END WARNUSER */

/* -------------------------------------------------------------------- */
void CancelWarning(Widget w, XtPointer clientData, XtPointer callData)
{
  XtUnmanageChild(warnBox[(int)clientData]);
  XtPopdown(XtParent(warnBox[(int)clientData]));
  inUse[(int)clientData] = False;

}	/* END CANCELWARNING */

/* -------------------------------------------------------------------- */
void CreateWarningBox(Widget parent)
{
  int	i;

  for (i = 0; i < nWarnings; ++i) {
    inUse[i] = False;
    warnBox[i] = XmCreateWarningDialog(parent, "warnBox", NULL, 0);
    XtSetSensitive(XmMessageBoxGetChild(warnBox[i],
                   XmDIALOG_HELP_BUTTON), False);
    }

}	/* END CREATEWARNINGBOX */

/* END XWARN.C */
