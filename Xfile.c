/*
-------------------------------------------------------------------------
OBJECT NAME:	Xfile.c

FULL NAME:	FileSelectionBox

DESCRIPTION:	This pops up a Fileselection box for choosing a file name
		Call CreateFile where you init X stuff.  Then just call
		QueryFile(Prompt, Default Directory, OK_callBack);  Then your
		OKcallBack procedure calls ExtractFileName()

INPUT:		String to Display.

OUTPUT:		Error message in its own tidy little window.
-------------------------------------------------------------------------
*/

#include <Xm/Xm.h>
#include <Xm/FileSB.h>
#include <Xm/TextF.h>

#include "define.h"

static Widget	fileBox;


/* -------------------------------------------------------------------- */
void QueryFile(char *prompt, char *directory, XtCallbackProc callBack)
{
  XmString	xmdir, xmprompt;
  Arg		args[4];
  int		n = 0;

  if (prompt)
    {
    xmprompt = XmStringCreate(prompt, XmSTRING_DEFAULT_CHARSET);
 
    XtSetArg(args[n], XmNselectionLabelString, xmprompt); ++n;
    XtSetValues(fileBox, args, n);
    XmStringFree(xmprompt);
    }
 
  if (directory)
    {
    xmdir = XmStringCreate(directory, XmSTRING_DEFAULT_CHARSET);
    XmFileSelectionDoSearch(fileBox, xmdir);
    XmStringFree(xmdir);
    }
  else
    XmFileSelectionDoSearch(fileBox, NULL);

  XtRemoveAllCallbacks(fileBox, XmNokCallback);
  XtAddCallback(fileBox, XmNokCallback, (XtCallbackProc)callBack, NULL);

  XtAddGrab(fileBox, True, False);
  XtManageChild(fileBox);

}	/* END QUERYFILE */

/* -------------------------------------------------------------------- */
void FileCancel(Widget w, XtPointer clientData, XtPointer callData)
{
  XtUnmanageChild(fileBox);
  XtRemoveGrab(fileBox);

}	/* END FILECANCEL */

/* -------------------------------------------------------------------- */
void CreateFileSelectionBox(Widget parent)
{
  fileBox = XmCreateFileSelectionDialog(parent, "fileBox", NULL, 0);
  XtSetSensitive(XmFileSelectionBoxGetChild(fileBox, XmDIALOG_HELP_BUTTON), False);

  XtAddCallback(fileBox, XmNcancelCallback, FileCancel, (XtPointer)False);

}	/* END CREATEFILESELECTIONBOX */

/* -------------------------------------------------------------------- */
char *ExtractFileDialogFilter()
{
  Widget fltr = XmFileSelectionBoxGetChild(fileBox, XmDIALOG_FILTER_TEXT);

  if (fltr)
    return XmTextFieldGetString(fltr);

}	/* END EXTRACTFILTER */

/* -------------------------------------------------------------------- */
void ExtractFileName(XmString str, char **text)
{
  XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, text);

}	/* END EXTRACTFILENAME */

/* END XFILE.C */
