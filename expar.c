/*
-------------------------------------------------------------------------
OBJECT NAME:	expar.c

FULL NAME:	Expression Parser

ENTRY POINTS:	ComputeExp()
		AcceptExpressions()

STATIC FNS:	CreateExpressionWindow()

DESCRIPTION:	User defined variables.

REFERENCES:	dataIO.c, exp.l, exp.y

REFERENCED BY:	Callback, dataIO.c

COPYRIGHT:	University Corporation for Atmospheric Research, 2001-02
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>


#define MAX_EXPRESSIONS	5
#define MAX_EXP_VARS	(MAX_DATASETS)


Widget		expText[MAX_EXPRESSIONS];
extern Widget	AppShell, ExpShell, ExpWindow;

static void	CreateExpressionWindow();


/* DataSets used by variaous expressions.  Export for dataIO.c */
size_t		NumberExpSets = 0;
DATASET_INFO	expSet[MAX_EXP_VARS];

float	scanit(char *, int);


/* -------------------------------------------------------------------- */
void ComputeExp(DATASET_INFO *set)
{
  size_t i;
  bool	saveState = Freeze;
  char	theExpression[256];

  Freeze = True;
  strcpy(theExpression, set->varInfo->expression);

printf("ComputeExp = %d, exp=%s", set->nPoints, theExpression);

  for (i = 0; i < set->nPoints; ++i)
    set->data[i] = scanit(theExpression, i);

  Freeze = saveState;

}	/* END COMPUTEEXP */

/* -------------------------------------------------------------------- */
void GetExpression(Widget w, XtPointer client, XtPointer call)
{
  static bool	firstTime = True;

  if (firstTime)
    {
    CreateExpressionWindow();
    firstTime = False;
    }

  XtManageChild(ExpWindow);
  XtPopup(XtParent(ExpWindow), XtGrabNone);

}	/* END GETEXPRESSION */

/* -------------------------------------------------------------------- */
void AcceptExpressions(Widget w, XtPointer client, XtPointer call)
{
  int		indx;
  size_t	i, nExps, fileIndx;
  VARTBL	*vi;
  char		exp[256], *e, *p, varName[NAMELEN], theExpression[256];

  if (NumberDataFiles == 0)
    return;

  /* Make sure no blank/deleted expressions exist in the middle.
   */
  for (nExps = i = 0; i < MAX_EXPRESSIONS; ++i)
    {
    p = XmTextFieldGetString(expText[i]);

    if (strlen(p) <= 0 || nExps++ == i) {
      free(p);
      continue;
      }

    XmTextFieldSetString(expText[nExps-1], p);
    XmTextFieldSetString(expText[i], "");
    free(p);
    }

  for (i = 0; i <= nExps; ++i)
    XtSetSensitive(expText[i], True);
  for (; i < MAX_EXPRESSIONS; ++i)
    XtSetSensitive(expText[i], False);



  /* Allocate memory for the new variables and/or add them to datafile zero.
   */
  for (	indx = dataFile[0].Variable.size()-1;
	strncmp(dataFile[0].Variable[indx]->name, "USER", 4) == 0; --indx)
    delete [] dataFile[0].Variable[indx]->expression;


  for (i = 0; i < nExps; ++i)
    if (++indx >= (int)dataFile[0].Variable.size())
      {
      vi = new VARTBL;
      dataFile[0].Variable.push_back(vi);

      sprintf(vi->name, "USER%d", i+1);
      vi->OutputRate = 1;
      vi->inVarID = COMPUTED;
      }



  /* Clean out all data sets required to compute previous expressions.
   */
  for (i = 0; i < NumberExpSets; ++i) {
    if (expSet[i].nPoints > 0) {
      delete [] expSet[i].data;
      expSet[i].nPoints = 0;
      }
    }


  /* Ok, parse and validate expressions.
   */
  NumberExpSets = 0;

  for (i = 0; i < nExps; ++i)
    {
    p = XmTextFieldGetString(expText[i]);
    strcpy(exp, p);
    strcat(exp, "\n");
    free(p);

    e = exp;
    theExpression[0] = '\0';


    /* Scan expression for use of variables from netCDF file, and validate.
     * Variable names from the netCDF file will be replaced with letters [A-P],
     * for easy access to the data during the lex/yacc parse stage.
     */
    for (; (p = strchr(e, '\"')); ++NumberExpSets)
      {
      *p++ = '\0';
      strcat(theExpression, e);
      sprintf(buffer, "%c", 'A' + NumberExpSets);
      strcat(theExpression, buffer);

      if ((e = strchr(p, '\"')) == NULL)
        {
        HandleError("Parse error, not enough quotes?", Interactive, IRET);
        return;
        }

      if (NumberExpSets >= MAX_EXP_VARS)
        {
        HandleError("Parse error, too many variables used.", Interactive, IRET);
        return;
        }

      *e = '\0';
      strcpy(varName, p);
      *e++ = '\"';

      fileIndx = CurrentDataFile;

      if ((indx = SearchTable(dataFile[fileIndx].Variable, varName)) == ERR)
        {
        sprintf(buffer, "Parse error, undefined variable %s.", varName);
        HandleError(buffer, Interactive, IRET);
        return;
        }

      expSet[NumberExpSets].fileIndex = fileIndx;
      expSet[NumberExpSets].panelIndex = i;	/* Whic Exp this belongs to */
      expSet[NumberExpSets].varInfo = dataFile[fileIndx].Variable[indx];
      }


    strcat(theExpression, e);
//printf("New exp = %s", theExpression);


    /* Add new variable to the *first* data file.
     */
    sprintf(buffer, "USER%d", i+1);
//printf("  >> %d\n", SearchTable((char **)dataFile[0].Variable,dataFile[0].Variable.size(), buffer));

    vi = dataFile[0].Variable[SearchTable(dataFile[0].Variable, buffer)];

    vi->expression = new char[256];
    strcpy(vi->expression, theExpression);
    }

  SetList();

}	/* END ACCEPTNEWEXPRESSION */

/* -------------------------------------------------------------------- */
static void CreateExpressionWindow()
{
  int           i;
  Cardinal	n;
  Arg		args[5];
  Widget        RC, frame, plRC, label;

  ExpShell = XtCreatePopupShell("expShell",
                  topLevelShellWidgetClass, AppShell, NULL, 0);

  ExpWindow = XmCreateRowColumn(ExpShell, "expParRC", NULL, 0);

  n = 0; 
  frame = XmCreateFrame(ExpWindow, "expFrame", args, n);
  XtManageChild(frame);
  
  n = 0; 
  RC = XmCreateRowColumn(frame, "expRC", args, n);


  for (i = 0; i < MAX_EXPRESSIONS; ++i)
    {
    plRC = XmCreateRowColumn(RC, "plRC", args, n);
    XtManageChild(plRC);

    sprintf(buffer, "USER%d = ", i+1);
    label = XmCreateLabel(plRC, buffer, args, n);
    XtManageChild(label);

    expText[i] = XmCreateTextField(plRC, "expText", args, n);
    XtManageChild(expText[i]);
    XtAddCallback(expText[i], XmNlosingFocusCallback, AcceptExpressions, NULL);

    if (i > 0)
      XtSetSensitive(expText[i], False);
    }

  XtManageChild(createARDbuttons(ExpWindow));
  XtManageChild(RC);

}	/* END CREATEEXPRESSIONWINDOW */

/* END EXPAR.C */
