/*
-------------------------------------------------------------------------
OBJECT NAME:	page.c

FULL NAME:	Command CallBacks

ENTRY POINTS:	PageForward()
		PageBackward()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	Callback

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"


void UTStoHHMMSS(int time[]);

static int EOFreached = False, SavedNumberSeconds;

/* -------------------------------------------------------------------- */
void PageForward(Widget w, XtPointer client, XtPointer call)
{
  if (UserEndTime[3] >= MaxEndTime[3])
    return;

  UserStartTime[3] = UserEndTime[3];
  UserEndTime[3] += NumberSeconds;

  if (UserEndTime[3] > MaxEndTime[3])
    {
    EOFreached = True;
    SavedNumberSeconds = NumberSeconds;
    UserEndTime[3] = MaxEndTime[3];
    }
  else
    EOFreached = False;

  UTStoHHMMSS(UserStartTime);
  UTStoHHMMSS(UserEndTime);

  SetTimeText();
  ReadData();
  DrawMainWindow();

}	/* END PAGEFORWARD */

/* -------------------------------------------------------------------- */
void PageBackward(Widget w, XtPointer client, XtPointer call)
{
  if (UserStartTime[3] <= MinStartTime[3])
    return;

  if (EOFreached)
    {
    EOFreached = False;
    NumberSeconds = SavedNumberSeconds;
    }

  UserEndTime[3] = UserStartTime[3];
  UserStartTime[3] -= NumberSeconds;

  if (UserStartTime[3] < MinStartTime[3])
    {
    UserStartTime[3] = MinStartTime[3];
    UserEndTime[3] = UserStartTime[3] + NumberSeconds;
    }

  UTStoHHMMSS(UserStartTime);
  UTStoHHMMSS(UserEndTime);

  SetTimeText();
  ReadData();
  DrawMainWindow();

}	/* END PAGEBACKWARD */

/* -------------------------------------------------------------------- */
void UTStoHHMMSS(int time[])
{
  int		t = time[3];

  time[0] = t / 3600; t -= time[0] * 3600;
  time[1] = t / 60; t -= time[1] * 60;
  time[2] = t;

/*  if (time[0] >= 24)
    time[0] -= 24;
*/
}	/* END HHMMSSTOUTS */

/* END PAGE.C */
