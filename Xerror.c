/*
-------------------------------------------------------------------------
OBJECT NAME:	Xerror.c

FULL NAME:	Show Error Box with Message

DESCRIPTION:	CreateError should be called once where you initialize
		your X stuff.  To use just call ShowError(ErrMsg)

INPUT:		String to Display.

OUTPUT:		Error message in its own tidy little window.

COPYRIGHT:	University Corporation for Atmospheric Research, 1991
-------------------------------------------------------------------------
*/

#include <Xm/Xm.h>
#include <Xm/MessageB.h>

static Widget	errorBox;


/* -------------------------------------------------------------------- */
void ShowError(const char str[])
{
  Widget	label;
  Arg		args[5];
  XmString	xStr;

  label = XmMessageBoxGetChild(errorBox, XmDIALOG_MESSAGE_LABEL);
  xStr = XmStringCreateLtoR(const_cast<char *>(str), XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[0], XmNlabelString, xStr);
  XtSetValues(label, args, 1);
  XmStringFree(xStr);

  XtManageChild(errorBox);
  XtPopup(XtParent(errorBox), XtGrabNone);

}	/* END SHOWERROR */

/* -------------------------------------------------------------------- */
void ErrorOK(Widget w, XtPointer clientData, XtPointer callData)
{
  XtPopdown(XtParent(errorBox));
  XtUnmanageChild(errorBox);

}	/* END ERROROK */

/* -------------------------------------------------------------------- */
void CreateErrorBox(Widget parent)
{
  errorBox = XmCreateErrorDialog(parent, "errorBox", NULL, 0);
  XtSetSensitive(XmMessageBoxGetChild(errorBox, XmDIALOG_CANCEL_BUTTON), False);
  XtSetSensitive(XmMessageBoxGetChild(errorBox, XmDIALOG_HELP_BUTTON), False);

  XtAddCallback(errorBox, XmNokCallback, ErrorOK, NULL);

}	/* END CREATEERRORBOX */

/* END XERROR.C */
