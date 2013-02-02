/*
-------------------------------------------------------------------------
OBJECT NAME:	preferences.c

FULL NAME:	Edit Preferences Window

ENTRY POINTS:	EditPreferences()
		SetPreferences()
		ReadConfigFile()

STATIC FNS:	CreatePreferences()
		ApplyPreferences()
		SavePreferences()

		SetColor()
		SetPrinter()
		SetPrinterColor()
		SetPrintCommand()
		SetTASVariable()

DESCRIPTION:	

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2004
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <ctype.h>

#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

static const int TOTAL_PREFERS = 15;

extern char	*insVariables[], *gpsVariables[], *gpsCorrected[],
		*windVariables[];

extern Widget	AppShell;
static Widget	PreferShell = NULL, PreferWindow, prefText[TOTAL_PREFERS];

static void	CreatePreferences(),
		SavePreferences(Widget w, XtPointer client, XtPointer call),
		ApplyPreferences(Widget w, XtPointer client, XtPointer call);

void SetPreferences();
char *GetTemplateDirectory(), *GetColorName(int indx);


/* -------------------------------------------------------------------- */
void EditPreferences(Widget w, XtPointer client, XtPointer call)
{
  static bool firstTime = true;

  if (firstTime)
    {
    CreatePreferences();
    firstTime = false;
    }

  XtManageChild(PreferWindow);
  XtPopup(XtParent(PreferWindow), XtGrabNone);

  SetPreferences();

}	/* END EDITPREFERENCES */

/* -------------------------------------------------------------------- */
void SetPreferences()
{
  int	i;
  char	temp[100];

  if (!PreferShell)
    return;

  strcpy(buffer, GetColorName(1));
  for (i = 2; i < 8; ++i)
    {
    sprintf(temp, ", %s", GetColorName(i));
    strcat(buffer, temp);
    }

  XmTextFieldSetString(prefText[0], buffer);

  sprintf(buffer, "%zu", LineThickness);
  XmTextFieldSetString(prefText[1], buffer);
  XmTextFieldSetString(prefText[2], GetTemplateDirectory());

  if (printerSetup.color)
    XmTextFieldSetString(prefText[3], (char *)"Color");
  else
    XmTextFieldSetString(prefText[3], (char *)"B&W");

  XmTextFieldSetString(prefText[4], const_cast<char *>(printerSetup.lpCommand.c_str()));

  XmTextFieldSetString(prefText[5], const_cast<char *>(tasVarName.c_str()));
  XmTextFieldSetString(prefText[6], gpsVariables[1]);
  XmTextFieldSetString(prefText[7], gpsVariables[0]);
  XmTextFieldSetString(prefText[8], gpsVariables[2]);
  XmTextFieldSetString(prefText[9], insVariables[1]);
  XmTextFieldSetString(prefText[10], insVariables[0]);
  XmTextFieldSetString(prefText[11], insVariables[2]);
  XmTextFieldSetString(prefText[12], windVariables[0]);
  XmTextFieldSetString(prefText[13], windVariables[1]);
  XmTextFieldSetString(prefText[14], windVariables[2]);

}	/* END SETPREFERENCES */

/* -------------------------------------------------------------------- */
static void SavePreferences(Widget w, XtPointer client, XtPointer call)
{
  int	i;
  FILE	*fp;
  char	*p, temp[100];

printf("Save Preferences: Writing ~/.ncplotrc\n");

  if ((p = getenv("HOME")) == NULL)
    return;

  sprintf(buffer, "%s/.ncplotrc", p);

  if ((fp = fopen(buffer, "w+")) == NULL)
    return;

  strcpy(buffer, GetColorName(1));
  for (i = 2; i < 8; ++i)
    {
    sprintf(temp, ", %s", GetColorName(i));
    strcat(buffer, temp);
    }

  fprintf(fp, "Colors = %s\n", buffer);
  fprintf(fp, "LineWidth = %zu\n", LineThickness);
  fprintf(fp, "TemplateDirectory = %s\n", GetTemplateDirectory());
  if (printerSetup.color)
    fprintf(fp, "PrintColor = Color\n");
  fprintf(fp, "PrintCommand = %s\n", printerSetup.lpCommand.c_str());
  fprintf(fp, "TrueAirspeed = %s\n", tasVarName.c_str());
  fprintf(fp, "GpsLongitude = %s\n", gpsVariables[0]);
  fprintf(fp, "GpsLatitude = %s\n", gpsVariables[1]);
  fprintf(fp, "GpsAltitude = %s\n", gpsVariables[2]);
  fprintf(fp, "InertialLongitude = %s\n", insVariables[0]);
  fprintf(fp, "InertialLatitude = %s\n", insVariables[1]);
  fprintf(fp, "PressureAltitude = %s\n", insVariables[2]);
  fprintf(fp, "WindU = %s\n", windVariables[0]);
  fprintf(fp, "WindV = %s\n", windVariables[1]);
  fprintf(fp, "WindInterval = %s\n", windVariables[2]);

  fclose(fp);

}	/* SAVEPREFERENCES */

/* -------------------------------------------------------------------- */
static void ApplyPreferences(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

  p = XmTextFieldGetString(prefText[0]);
  SetColorNames(p);
  free(p);

  p = XmTextFieldGetString(prefText[1]);
  if (atoi(p) > 1)
    LineThickness = atoi(p);
  free(p);

  p = XmTextFieldGetString(prefText[2]);
  SetTemplateDirectory(p);
  free(p);

  p = XmTextFieldGetString(prefText[3]);
  if (strncmp(p, "Color", 5) == 0)
    printerSetup.color = true;
  free(p);

  p = XmTextFieldGetString(prefText[4]);
  printerSetup.lpCommand = p;
  free(p);

  p = XmTextFieldGetString(prefText[5]);
  tasVarName = p;
  free(p);

  p = XmTextFieldGetString(prefText[6]);
  strcpy(gpsVariables[1], p);
  free(p);

  p = XmTextFieldGetString(prefText[7]);
  strcpy(gpsVariables[0], p);
  free(p);

  p = XmTextFieldGetString(prefText[8]);
  strcpy(gpsVariables[2], p);
  free(p);

  p = XmTextFieldGetString(prefText[9]);
  strcpy(insVariables[1], p);
  free(p);

  p = XmTextFieldGetString(prefText[10]);
  strcpy(insVariables[0], p);
  free(p);

  p = XmTextFieldGetString(prefText[11]);
  strcpy(insVariables[2], p);
  free(p);

  p = XmTextFieldGetString(prefText[12]);
  strcpy(windVariables[0], p);
  free(p);

  p = XmTextFieldGetString(prefText[13]);
  strcpy(windVariables[1], p);
  free(p);

  p = XmTextFieldGetString(prefText[14]);
  strcpy(windVariables[2], p);
  free(p);

  DrawMainWindow();

}	/* END APPLYPREFERENCES */

/* -------------------------------------------------------------------- */
static void CreatePreferences()
{
  int		i;
  Cardinal	n;
  Arg		args[2];
  Widget	frame, RC, plRC, label, b[3];

  PreferShell = XtCreatePopupShell("prefParmsShell",
                   topLevelShellWidgetClass, AppShell, NULL, 0);

  PreferWindow = XmCreateRowColumn(PreferShell, (char *)"prefRC", NULL, 0);

  n = 0;
  frame = XmCreateFrame(PreferWindow, (char *)"prefFrame", args, n);
  XtManageChild(frame);

  n = 0;
  RC = XmCreateRowColumn(frame, (char *)"prefRC", args, n);

  for (i = 0; i < TOTAL_PREFERS; ++i)
    {
    plRC = XmCreateRowColumn(RC, (char *)"plRC", args, n);
    XtManageChild(plRC);

    sprintf(buffer, "prefer%d", i);
    label = XmCreateLabel(plRC, buffer, args, n);
    XtManageChild(label);

    prefText[i] = XmCreateTextField(plRC, (char *)"prefText", args, n);
    XtManageChild(prefText[i]);
    XtAddCallback(prefText[i], XmNlosingFocusCallback, ApplyPreferences, NULL);
    }


  /* Command buttons.
   */
  n = 0;
  frame = XmCreateFrame(PreferWindow, (char *)"buttonFrame", args, 0);
  XtManageChild(frame);

  n = 0;
  plRC = XmCreateForm(frame, (char *)"buttonRC", args, n);

  n = 0;
  b[0] = XmCreatePushButton(plRC, (char *)"dismissButton", args, n);
  XtAddCallback(b[0], XmNactivateCallback, DismissWindow, PreferWindow);

  n = 0;
  b[1] = XmCreatePushButton(plRC, (char *)"saveButton", args, n);
  XtAddCallback(b[1], XmNactivateCallback, SavePreferences, NULL);

  XtManageChildren(b, 2);

  XtManageChild(plRC);
  XtManageChild(RC);

}	/* END CREATEPREFERENCES */

/* --------------------------------------------------------------------- */
void ReadConfigFile()
{
  FILE	*fp;
  char	*p;

  if ((p = getenv("HOME")) == NULL)
    return;

  sprintf(buffer, "%s/.ncplotrc", p);

  if ((fp = fopen(buffer, "r")) == NULL)
    return;

  printf("Reading config file %s.\n", buffer);

  while (fgets(buffer, 1024, fp) > 0)
    {
    if (buffer[0] == '#' || strlen(buffer) < 3)
      continue;

    p = strchr(buffer, '=') + 1;

    if (p == (char *)1)
       continue;

    while (isspace(*p))
      ++p;

    p[strlen(p)-1] = '\0'; /* ditch newline */

    if (strncmp(buffer, "Colors", 6) == 0) {
      SetColorNames(p);
      }
    if (strncmp(buffer, "TrueAirspeed", 12) == 0) {
      tasVarName = p;
      }
    if (strncmp(buffer, "GpsLatitude", 11) == 0) {
      strcpy(gpsVariables[1], p);
      }
    if (strncmp(buffer, "GpsLongitude", 12) == 0) {
      strcpy(gpsVariables[0], p);
      }
    if (strncmp(buffer, "GpsAltitude", 11) == 0) {
      strcpy(gpsVariables[2], p);
      }
    if (strncmp(buffer, "InertialLatitude", 16) == 0) {
      strcpy(insVariables[1], p);
      }
    if (strncmp(buffer, "InertialLongitude", 17) == 0) {
      strcpy(insVariables[0], p);
      }
    if (strncmp(buffer, "PressureAltitude", 16) == 0) {
      strcpy(insVariables[2], p);
      }
    if (strncmp(buffer, "WindU", 5) == 0) {
      strcpy(windVariables[0], p);
      }
    if (strncmp(buffer, "WindV", 5) == 0) {
      strcpy(windVariables[1], p);
      }
    if (strncmp(buffer, "WindInterval", 12) == 0) {
      strcpy(windVariables[2], p);
      }
    if (strncmp(buffer, "PrintCommand", 12) == 0) {
      printerSetup.lpCommand = p;
      }
    if (strncmp(buffer, "TemplateDir", 11) == 0) {
      SetTemplateDirectory(p);
      }
    if (strncmp(buffer, "PrintColor", 10) == 0) {
      if (strncmp(p, "Color", 5) == 0)
        printerSetup.color = true;
      }
    if (strncmp(buffer, "LineWidth", 9) == 0) {
      if (atoi(p) > 1)
        LineThickness = atoi(p);
      }
    }

  fclose(fp);

}	/* END READCONFIGFILE */

/* END PREFERENCES.C */
