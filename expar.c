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


static const size_t MAX_EXPRESSIONS = 5;

#define MAX_EXP_VARS	(MAX_DATASETS)

Widget		expText[MAX_EXPRESSIONS];
extern Widget	AppShell, ExpShell, ExpWindow;

static void	CreateExpressionWindow();


/* DataSets used by various expressions.  Export for dataIO.c */
std::vector<DATASET_INFO> expSet;

float	scanit(char *, int);


/* -------------------------------------------------------------------- */
void ComputeExp(DATASET_INFO *set)
{
  bool	saveState = Freeze;
  char  theExpression[BUFFSIZE];

  Freeze = True;
  strcpy(theExpression, set->varInfo->expression.c_str());

  for (size_t i = 0; i < set->nPoints; ++i)
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
  std::string	varName;
  char		*p, *s, *dot;

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

    XmTextFieldSetString(expText[nExps-1], buffer);
    XmTextFieldSetString(expText[i], (char *)"");
    free(p);
    }

  /* Check for .## and replace with 0.##.  i.e. all fractional numerics must
   * have a leading 0.
   */
  for (i = 0; i < nExps; ++i)
    {
    p = s = XmTextFieldGetString(expText[i]);

    dot = strchr(s, '.');
    if (dot)
    {
      buffer[0] = '\0';
      do
      {
        strncat(buffer, s, (size_t)dot-(size_t)s);
        if (!isdigit(dot[-1]))
          strcat(buffer, "0");
        s = dot;
      }
      while ( (dot = strchr(dot+1, '.')) );
      strncat(buffer, s, (size_t)dot-(size_t)s);
    }
    else
      strcpy(buffer, s);

    XmTextFieldSetString(expText[i], buffer);
    free(p);
    }

  for (i = 0; i < MAX_EXPRESSIONS && i <= nExps; ++i)
    XtSetSensitive(expText[i], True);
  for (; i < MAX_EXPRESSIONS; ++i)
    XtSetSensitive(expText[i], False);


  /* Remove all expression varibles from dataFile[0].
   */
  for (	indx = dataFile[0].Variable.size()-1; 
	dataFile[0].Variable[indx]->name.find("USER") != std::string::npos;
	--indx)
    delete dataFile[0].Variable[indx];

  dataFile[0].Variable.resize(indx+1);

  /* Add expression variables to dataFile[0].
   */
  for (i = 0; i < nExps; ++i)
    {
    char tmp[64];
    vi = new VARTBL;
    dataFile[0].Variable.push_back(vi);

    sprintf(tmp, "USER%ld", i+1);
    vi->name = tmp;
    vi->OutputRate = 1;
    vi->inVarID = COMPUTED;
    }

  /* Clean out all data sets required to compute previous expressions.
   */
  for (i = 0; i < expSet.size(); ++i)
    delete [] expSet[i].data;

  expSet.clear();

  /* Ok, parse and validate expressions.
   */
  size_t NumberExpSets = 0;

  for (i = 0; i < nExps; ++i)
    {
    char exp[BUFFSIZE], *e;

    p = XmTextFieldGetString(expText[i]);
    strcpy(exp, p);
    strcat(exp, "\n");
    free(p);

    e = exp;

    std::string theExpression;

    /* Scan expression for use of variables from netCDF file, and validate.
     * Variable names from the netCDF file will be replaced with letters [A-P],
     * for easy access to the data during the lex/yacc parse stage.
     */
    for (; (p = strchr(e, '\"')); ++NumberExpSets)
      {
      *p++ = '\0';
      theExpression += e;
      sprintf(buffer, "%c", 'A' + NumberExpSets);
      theExpression += buffer;

      if ((e = strchr(p, '\"')) == NULL)
        {
        HandleError("Parse error, not enough quotes?", Interactive, IRET);
        return;
        }

      *e = '\0';
      varName = p;
      *e++ = '\"';

      fileIndx = CurrentDataFile;

      if ((indx = SearchTable(dataFile[fileIndx].Variable, varName)) == ERR)
        {
        sprintf(buffer, "Parse error, undefined variable %s.", varName.c_str());
        HandleError(buffer, Interactive, IRET);
        return;
        }

      DATASET_INFO ds;
      ds.fileIndex = fileIndx;
      ds.panelIndex = i;
      ds.varInfo = dataFile[fileIndx].Variable[indx];
      ds.data = 0;
      ds.nPoints = 0;
      expSet.push_back(ds);
      }

    theExpression += e;

    /* Add new variable to the *first* data file.
     */
    sprintf(buffer, "USER%ld", i+1);

    vi = dataFile[0].Variable[SearchTable(dataFile[0].Variable, buffer)];
    vi->expression = theExpression;
    }

  SetList(NULL, NULL, NULL);

}	/* END ACCEPTNEWEXPRESSION */

/* -------------------------------------------------------------------- */
static void CreateExpressionWindow()
{
  Cardinal	n;
  Arg		args[5];
  Widget        RC, frame, plRC, label;

  ExpShell = XtCreatePopupShell("expShell",
                  topLevelShellWidgetClass, AppShell, NULL, 0);

  ExpWindow = XmCreateRowColumn(ExpShell, (char *)"expParRC", NULL, 0);

  n = 0; 
  frame = XmCreateFrame(ExpWindow, (char *)"expFrame", args, n);
  XtManageChild(frame);
  
  n = 0; 
  RC = XmCreateRowColumn(frame, (char *)"expRC", args, n);


  for (size_t i = 0; i < MAX_EXPRESSIONS; ++i)
    {
    plRC = XmCreateRowColumn(RC, (char *)"plRC", args, n);
    XtManageChild(plRC);

    sprintf(buffer, "USER%ld = ", i+1);
    label = XmCreateLabel(plRC, buffer, args, n);
    XtManageChild(label);

    expText[i] = XmCreateTextField(plRC, (char *)"expText", args, n);
    XtManageChild(expText[i]);
    XtAddCallback(expText[i], XmNlosingFocusCallback, AcceptExpressions, NULL);

    if (i > 0)
      XtSetSensitive(expText[i], False);
    }

  XtManageChild(createARDbuttons(ExpWindow));
  XtManageChild(RC);

}	/* END CREATEEXPRESSIONWINDOW */

/* END EXPAR.C */
