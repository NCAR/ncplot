/*
-------------------------------------------------------------------------
OBJECT NAME:	cursor.c

FULL NAME:	Cursors

ENTRY POINTS:	CanvasInput()
		WaitCursor()
		TextCursor()
		PointerCursor()
		CrossHairCursor()
		TextCursorWrap()
		PointerCursorWrap()

STATIC FNS:	none

DESCRIPTION:	Create and remove various cursors.  Stop watch, text
		insert, etc.

REFERENCES:	none

REFERENCED BY:	dataIO.c, annotate.c crosshair.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-9
-------------------------------------------------------------------------
*/

#include "define.h"

#include <X11/cursorfont.h>

static Cursor waitCursor;
static Cursor textCursor;
static Cursor grabCursor;
static Cursor pointerCursor;
static Cursor crosshairCursor;

static int  cursorMode = POINTER;


extern Widget   MainWindow, ControlWindow, SpectrumWindow, DifferenceWindow,
		TrackOptWindow;


/* -------------------------------------------------------------------- */
void WaitCursorAll()
{
  WaitCursor(MainWindow);
  WaitCursor(ControlWindow);
  WaitCursor(SpectrumWindow);
  WaitCursor(DifferenceWindow);
  WaitCursor(TrackOptWindow);

}

void PointerCursorAll()
{
  PointerCursor(MainWindow);
  PointerCursor(ControlWindow);
  PointerCursor(SpectrumWindow);
  PointerCursor(DifferenceWindow);
  PointerCursor(TrackOptWindow);

}

/* -------------------------------------------------------------------- */
void CanvasInput(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
  XAnyEvent *xe = (XAnyEvent *)evt->event;

  switch (cursorMode)
    {
    case POINTER:
      if (xe->type == 4 || xe->type == 5)
        Zoom(w, client, evt);

      break;

    case ANNOTATE:
      if (xe->type == 5)
        SetCursorXY(w, client, evt);

      if (xe->type == 3)
        ProcessText(w, client, evt);

      break;
    }

}	/* END CANVASINPUT */

/* -------------------------------------------------------------------- */
void WaitCursor(Widget w)
{
  Display	*dpy;
  static bool	firstTime = True;

  if (w == NULL)
    return;

  dpy = XtDisplay(w);

  if (firstTime)
    {
    waitCursor = XCreateFontCursor(dpy, XC_watch);
    firstTime = False;
    }

  XDefineCursor(dpy, XtWindow(w), waitCursor);
  XFlush(dpy);

}	/* END WAITCURSOR */

/* -------------------------------------------------------------------- */
void TextCursor(Widget w)
{
  Display	*dpy;
  static bool	firstTime = True;

  if (w == NULL)
    return;

  dpy = XtDisplay(w);

  if (firstTime)
    {
    textCursor = XCreateFontCursor(dpy, XC_xterm);
    firstTime = False;
    }

  XDefineCursor(dpy, XtWindow(w), textCursor);
  XFlush(dpy);
  cursorMode = ANNOTATE;

}	/* END TEXTCURSOR */

/* -------------------------------------------------------------------- */
void GrabCursor(Widget w)
{
  Display	*dpy;
  static bool	firstTime = True;

  if (w == NULL)
    return;

  dpy = XtDisplay(w);

  if (firstTime)
    {
    grabCursor = XCreateFontCursor(dpy, XC_xterm);
    firstTime = False;
    }

  XDefineCursor(dpy, XtWindow(w), grabCursor);
  XFlush(dpy);
  cursorMode = GRAB;

}	/* END GRABCURSOR */

/* -------------------------------------------------------------------- */
void PointerCursor(Widget w)
{
  Display	*dpy;
  static bool	firstTime = True;

  if (w == NULL)
    return;

  dpy = XtDisplay(w);

  if (firstTime)
    {
    pointerCursor = XCreateFontCursor(dpy, XC_left_ptr);
    firstTime = False;
    }

  XDefineCursor(dpy, XtWindow(w), pointerCursor);
  XFlush(dpy);
  cursorMode = POINTER;

}	/* END POINTERCURSOR */

/* -------------------------------------------------------------------- */
void CrossHairCursor(Widget w)
{
  Display	*dpy;
  static bool	firstTime = True;

  if (w == NULL)
    return;

  dpy = XtDisplay(w);

  if (firstTime)
    {
    crosshairCursor = XCreateFontCursor(dpy, XC_crosshair);
    firstTime = False;
    }

  XDefineCursor(dpy, XtWindow(w), crosshairCursor);
  XFlush(dpy);
  cursorMode = CROSSHAIR;

}	/* END CROSSHAIRCURSOR */

/* -------------------------------------------------------------------- */
void TextCursorWrap(Widget w, XtPointer client, XtPointer call)
{
  TextCursor((Widget)client);

}	/* END TEXTCURSORWRAP */

/* -------------------------------------------------------------------- */
void PointerCursorWrap(Widget w, XtPointer client, XtPointer call)
{
  PointerCursor((Widget)client);

}	/* END POINTERCURSORWRAP */

/* END CURSOR.C */
