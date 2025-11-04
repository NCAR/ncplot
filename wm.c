/*
-------------------------------------------------------------------------
OBJECT NAME:	wm.c

FULL NAME:	Window Manager callbacks

ENTRY POINTS:	SetupTrapWMClose()
		equalScaling()

STATIC FNS:	none

DESCRIPTION:	Trap Window Manater close

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2022
-------------------------------------------------------------------------
*/

#include "define.h"
#include <Xm/Protocols.h>

static Atom	property, protocol;


/* -------------------------------------------------------------------- */
void WMdismiss(Widget w, XtPointer client, XtPointer call)
{
  // Handle the window popdown event
//  printf("Window dismiss requested by window manager! %u %u %u\n", w, client, call);
  XtUnmanageChild((Widget)client);
  XtPopdown(XtParent((Widget)client));
}

/* -------------------------------------------------------------------- */
void WMquit(Widget w, XtPointer client, XtPointer call)
{
  // Handle the window close event
  printf("Window close requested by window manager!\n");
  XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
  Quit(0, 0, 0);
}

/* -------------------------------------------------------------------- */
void SetupTrapWMClose(Widget appShell)
{
  // Trap Window Manager close
  property = XmInternAtom(XtDisplay(appShell), "WM_PROTOCOLS", False);
  protocol = XmInternAtom(XtDisplay(appShell), "WM_DELETE_WINDOW", True);
  XmAddProtocols(appShell, property, &protocol, 1);

  // Quit if any of the primary WM close buttons are pressed.
  XmAddProtocolCallback(appShell, property, protocol, WMquit, (XtPointer)NULL);
}


/* -------------------------------------------------------------------- */
void WindowManagerCloseSetDismiss(Widget shell, Widget win)
{
  XtVaSetValues(shell, XmNdeleteResponse, XmDO_NOTHING, NULL);
  XmAddProtocolCallback(shell, property, protocol, WMdismiss, (XtPointer)win);
}

/* -------------------------------------------------------------------- */
void WindowManagerCloseSetQuit(Widget shell)
{
  XmAddProtocolCallback(shell, property, protocol, WMquit, (XtPointer)NULL);
}

/* END WM.C */
