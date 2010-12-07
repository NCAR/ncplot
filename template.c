/*
-------------------------------------------------------------------------
OBJECT NAME:	template.c

FULL NAME:	Save/Load templates

ENTRY POINTS:	SaveTemplate()
		LoadTemplate()
		SetTemplateDirectory()
		SetTemplateFile()

STATIC FNS:	load_CB2()
		save_CB2()
		saveTemplate()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1998-2001
-------------------------------------------------------------------------
*/

#include "define.h"
#include <unistd.h>

#include <Xm/List.h>

static char	templateDir[256] = "";
static char	templateFile[256] = "";

static void	save_CB2(Widget w, XtPointer client, XtPointer call),
		load_CB2(Widget w, XtPointer client, XtPointer call),
		saveTemplate(Widget w, XtPointer client, XtPointer call);

void    ChangePlotType(Widget, XtPointer, XtPointer),
	AddPanel(Widget, XtPointer, XtPointer),
	DeletePanel(Widget, XtPointer, XtPointer),
	findMinMax();

/* -------------------------------------------------------------------- */
char *GetTemplateDirectory()
{
  strcpy(buffer, templateDir);
  buffer[strlen(templateDir)-2] = '\0';

  return(buffer);

}

/* -------------------------------------------------------------------- */
void SetTemplateDirectory(char s[])
{
  strcpy(templateDir, s);
  strcat(templateDir, "/*");

}

/* -------------------------------------------------------------------- */
void SetTemplateFile(char s[])
{
  strcpy(templateFile, s);

}

/* -------------------------------------------------------------------- */
void CheckForTemplateFile()
{
  if (templateFile[0])
    load_CB2(NULL, NULL, NULL);

}

/* -------------------------------------------------------------------- */
void SaveTemplate(Widget w, XtPointer client, XtPointer call)
{
  if (NumberDataFiles == 0)
    {
    HandleError("No data.", Interactive, RETURN);
    return;
    }

  QueryFile("Enter template file to save:", templateDir, save_CB2);

}	/* END SAVETEMPLATE */

/* -------------------------------------------------------------------- */
static void save_CB2(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

  FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
  ExtractFileName(((XmFileSelectionBoxCallbackStruct *)call)->value, &p);
  strcpy(templateFile, p);
  strcpy(templateDir, p);
  if ((p = strrchr(templateDir, '/')))
    strcpy(p+1, "*");

  if (access(templateFile, F_OK) == 0)
    {
    sprintf(buffer, "Overwrite file %s?", templateFile);
    WarnUser(buffer, saveTemplate, NULL);
    }
  else
    saveTemplate(NULL, NULL, NULL);

}	/* END SAVE_CB2 */

/* -------------------------------------------------------------------- */
static void saveTemplate(Widget w, XtPointer client, XtPointer call)
{
  size_t i, x;
  FILE	*fp;
  bool	saveState = Freeze;

  if (call)
    CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

  if ((fp = fopen(templateFile, "w+")) == NULL)
    {
    sprintf(buffer, "Can't destroy %s.", templateFile);
    HandleError(buffer, Interactive, RETURN);
    return;
    }


  Freeze = true;
  WaitCursorAll();


  fprintf(fp, "ncplot template file\n");
  fprintf(fp, "Version=4\n");
  fprintf(fp, "PlotType=%d\n", PlotType);

  switch (PlotType)
    {
    case TIME_SERIES:
      fprintf(fp, "nPanels=%ld\n", NumberOfPanels);

      for (i = 0; i < NumberOfPanels; ++i)
        {
        fprintf(fp, "Grid=%d, AutoScale=%d, AutoTics=%d\n", mainPlot[i].grid,
		mainPlot[i].autoScale, mainPlot[i].autoTics);

        fprintf(fp, "Yaxis - Invert=%d %d, log=%d %d, bounds=%e %e %e %e, xTics=%d %d, yTics=%d %d\n", 
		mainPlot[i].Yaxis[0].invertAxis,
		mainPlot[i].Yaxis[1].invertAxis,
		mainPlot[i].Yaxis[0].logScale,
		mainPlot[i].Yaxis[1].logScale,
		mainPlot[i].Yaxis[0].min, mainPlot[i].Yaxis[0].max,
		mainPlot[i].Yaxis[1].min, mainPlot[i].Yaxis[1].max,
		mainPlot[i].Xaxis.nMajorTics, mainPlot[i].Xaxis.nMinorTics,
		mainPlot[i].Yaxis[0].nMajorTics, mainPlot[i].Yaxis[0].nMinorTics);
        }

      fprintf(fp, "nSets=%ld\n", NumberDataSets);

      for (i = 0; i < NumberDataSets; ++i)
        {
        fprintf(fp, "VarName=%s Panel#=%d, ScaleLoc=%d\n",
		dataSet[i].varInfo->name.c_str(), dataSet[i].panelIndex,
		dataSet[i].scaleLocation);
        }

      for (i = 0; i < NumberOfPanels; ++i)
        {
        fprintf(fp, "xLabel%ld=%s\n", i, mainPlot[i].Xaxis.label.c_str());
        fprintf(fp, "yLabel%ldL=%s\n", i, mainPlot[i].Yaxis[0].label.c_str());
        fprintf(fp, "yLabel%ldR=%s\n", i, mainPlot[i].Yaxis[1].label.c_str());
        }

      break;


    case XY_PLOT:
      fprintf(fp, "nPanels=%ld\n", NumberOfXYpanels);

      for (i = 0; i < NumberOfXYpanels; ++i)
        {
        fprintf(fp, "Grid=%d, AutoScale=%d, AutoTics=%d\n",
		xyyPlot[i].grid, xyyPlot[i].autoScale, xyyPlot[i].autoTics);

        fprintf(fp, "Xaxis - Invert=%d, log=%d, bounds=%e %e, tics=%d %d\n", 
		xyyPlot[i].Xaxis.invertAxis, xyyPlot[i].Xaxis.logScale,
		xyyPlot[i].Xaxis.min, xyyPlot[i].Xaxis.max,
		xyyPlot[i].Xaxis.nMajorTics, xyyPlot[i].Xaxis.nMinorTics);

        fprintf(fp, "Yaxis - Invert=%d %d, log=%d %d, bounds=%e %e %e %e, tics=%d %d\n", 
		xyyPlot[i].Yaxis[0].invertAxis, xyyPlot[i].Yaxis[1].invertAxis,
		xyyPlot[i].Yaxis[0].logScale, xyyPlot[i].Yaxis[1].logScale,
		xyyPlot[i].Yaxis[0].min, xyyPlot[i].Yaxis[0].max,
		xyyPlot[i].Yaxis[1].min, xyyPlot[i].Yaxis[1].max,
		xyyPlot[i].Yaxis[0].nMajorTics, xyyPlot[i].Yaxis[0].nMinorTics);

        }

      fprintf(fp, "nSets=%ld\n", NumberXYXsets + NumberXYYsets);

      for (i = 0; i < NumberXYXsets; ++i)
        fprintf(fp, "Panel#=%d, Axis=%d, ScaleLoc=%d, VarName=%s\n",
		xyXset[i].panelIndex, X_AXIS, xyXset[i].scaleLocation,
		xyXset[i].varInfo->name.c_str());

      for (i = 0; i < NumberXYYsets; ++i)
        fprintf(fp, "Panel#=%d, Axis=%d, ScaleLoc=%d, VarName=%s\n",
		xyYset[i].panelIndex, Y_AXIS, xyYset[i].scaleLocation,
		xyYset[i].varInfo->name.c_str());

      for (i = 0; i < NumberOfXYpanels; ++i)
        {
        fprintf(fp, "xLabel%ld=%s\n", i, xyyPlot[i].Xaxis.label.c_str());
        fprintf(fp, "yLabel%ldL=%s\n", i, xyyPlot[i].Yaxis[0].label.c_str());
        fprintf(fp, "yLabel%ldR=%s\n", i, xyyPlot[i].Yaxis[1].label.c_str());
        }

      break;


    case XYZ_PLOT:
      fprintf(fp, "nPanels=1\n");
      fprintf(fp, "AutoScale=%d, AutoTics=%d\n", xyzPlot.autoScale, xyzPlot.autoTics);

      fprintf(fp, "Xaxis - Invert=%d, log=%d, bounds=%e %e, tics=%d %d\n", 
		xyzPlot.Xaxis.invertAxis, xyzPlot.Xaxis.logScale,
		xyzPlot.Xaxis.min, xyzPlot.Xaxis.max,
		xyzPlot.Xaxis.nMajorTics, xyzPlot.Xaxis.nMinorTics);

      fprintf(fp, "Yaxis - Invert=%d, log=%d, bounds=%e %e, tics=%d %d\n", 
		xyzPlot.Yaxis[0].invertAxis, xyzPlot.Yaxis[0].logScale,
		xyzPlot.Yaxis[0].min, xyzPlot.Yaxis[0].max,
		xyzPlot.Yaxis[0].nMajorTics, xyzPlot.Yaxis[0].nMinorTics);

      fprintf(fp, "Zaxis - Invert=%d, log=%d, bounds=%e %e, tics=%d %d\n", 
		xyzPlot.Zaxis.invertAxis, xyzPlot.Zaxis.logScale,
		xyzPlot.Zaxis.min, xyzPlot.Zaxis.max,
		xyzPlot.Zaxis.nMajorTics, xyzPlot.Zaxis.nMinorTics);

      for (i = 0, x = 0; i < 3; ++i)
        if (xyzSet[i].varInfo)
          ++x;

      fprintf(fp, "nSets=%ld\n", x);

      for (i = 0; i < 3; ++i)
        {
        if (xyzSet[i].varInfo)
          fprintf(fp, "Axis=%ld, VarName=%s\n", i, xyzSet[i].varInfo->name.c_str());
        }

      fprintf(fp, "xLabel=%s\n", xyzPlot.Xaxis.label.c_str());
      fprintf(fp, "yLabel=%s\n", xyzPlot.Yaxis[0].label.c_str());
      fprintf(fp, "zLabel=%s\n", xyzPlot.Zaxis.label.c_str());

      break;
    }


  fclose(fp);

  Freeze = saveState;

  PointerCursorAll();

}  /* END SAVETEMPLATE */

/* -------------------------------------------------------------------- */
void LoadTemplate(Widget w, XtPointer client, XtPointer call)
{
  if (w)
    QueryFile("Enter template file to load:", templateDir, load_CB2);

}	/* END LOADTEMPLATE */

/* -------------------------------------------------------------------- */
static void load_CB2(Widget w, XtPointer client, XtPointer call)
{
  FILE	*fp;
  size_t i, x, x1, x2, x3, x4, x5, x6, x7, x8, version;
  float	min0, min1, max0, max1;
  char	s[32], *p;
  XmString      name;

  if (w)
    {
    FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
    ExtractFileName(((XmFileSelectionBoxCallbackStruct *)call)->value, &p);
    strcpy(templateFile, p);
    strcpy(templateDir, p);
    if ((p = strrchr(templateDir, '/')))
      strcpy(p+1, "*");
    }

  if ((fp = fopen(templateFile, "r")) == NULL)
    {
    sprintf(buffer, "Can't open %s.", templateFile);
    HandleError(buffer, Interactive, RETURN);
    return;
    }


  fgets(buffer, 512, fp);
  if (strcmp(buffer, "ncplot template file\n") != 0)
    {
    HandleError("Not a valid template file.", Interactive, RETURN);
    return;
    }


  fgets(buffer, 512, fp);
  sscanf(buffer, "Version=%ld", &version);

  fgets(buffer, 512, fp);
  sscanf(buffer, "PlotType=%ld", &x);

  ChangePlotType(NULL, (XtPointer)x, NULL);
  ClearPlot(NULL, NULL, NULL);

  switch (PlotType)
    {
    case TIME_SERIES:
      fgets(buffer, 512, fp);
      sscanf(buffer, "nPanels=%ld", &x);

      while (x > NumberOfPanels)
        AddPanel(NULL, NULL, NULL);

      while (x < NumberOfPanels)
        DeletePanel(NULL, NULL, NULL);

      for (i = 0; i < NumberOfPanels; ++i)
        {
        fgets(buffer, 512, fp);
        sscanf(buffer, "Grid=%ld, AutoScale=%ld, AutoTics=%ld", &x1, &x2, &x3);
        mainPlot[i].grid = x1;
        mainPlot[i].autoScale = x2;
        mainPlot[i].autoTics = x3;

        fgets(buffer, 512, fp);
        if (version < 2)
          sscanf(buffer, "Yaxis - Invert=%ld %ld, log=%ld %ld", &x1, &x2, &x3, &x4);
        else
        if (version == 3)
          sscanf(buffer, "Yaxis - Invert=%ld %ld, log=%ld %ld, bounds=%e %e %e %e",
		&x1, &x2, &x3, &x4, &min0, &max0, &min1, &max1);
        else
          sscanf(buffer, "Yaxis - Invert=%ld %ld, log=%ld %ld, bounds=%e %e %e %e, xTics=%ld %ld, yTics=%ld %ld",
		&x1, &x2, &x3, &x4, &min0, &max0, &min1, &max1, &x5, &x6, &x7, &x8);

        mainPlot[i].Yaxis[0].invertAxis = x1;
        mainPlot[i].Yaxis[1].invertAxis = x2;
        mainPlot[i].Yaxis[0].logScale = x3;
        mainPlot[i].Yaxis[1].logScale = x4;

        if (version >= 2)
          {
          mainPlot[i].Yaxis[0].min = min0;
          mainPlot[i].Yaxis[0].max = max0;
          mainPlot[i].Yaxis[1].min = min1;
          mainPlot[i].Yaxis[1].max = max1;
          }

        if (version >= 4)
          {
          mainPlot[i].Xaxis.nMajorTics = x5;
          mainPlot[i].Xaxis.nMinorTics = x6;
          mainPlot[i].Yaxis[0].nMajorTics = x7;
          mainPlot[i].Yaxis[0].nMinorTics = x8;
          mainPlot[i].Yaxis[1].nMajorTics = x7;
          mainPlot[i].Yaxis[1].nMinorTics = x8;
          }
        }

      fgets(buffer, 512, fp);
      sscanf(buffer, "nSets=%ld", &x);

      for (i = 0; i < x; ++i)
        {
        fgets(buffer, 512, fp);
        sscanf(buffer, "VarName=%s Panel#=%ld, ScaleLoc=%ld", s, &CurrentPanel, &x1);
        name = XmStringCreateLocalized(s);
        if ((x2 = XmListItemPos(varList, name)) > 0)
          {
          DataChanged = true;
          AddVariable(&dataSet[NumberDataSets++], s);
          mainPlot[CurrentPanel].Yaxis[x1].label =
			dataSet[NumberDataSets-1].stats.units;

          dataSet[NumberDataSets-1].scaleLocation = x1;
          }
        else
          {
          sprintf(buffer, "Variable %s not found, continuing.", s);
          HandleError(buffer, Interactive, RETURN);
          }

        XmStringFree(name);
        }

      if (version > 2)
        {
        for (i = 0; i < NumberOfPanels; ++i)
          {
          fgets(buffer, 512, fp);
          buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
          mainPlot[i].Xaxis.label = strchr(buffer, '=')+1;

          fgets(buffer, 512, fp);
          buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
          mainPlot[i].Yaxis[0].label = strchr(buffer, '=')+1;

          fgets(buffer, 512, fp);
          buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
          mainPlot[i].Yaxis[1].label = strchr(buffer, '=')+1;
          }
        }

      break;


    case XY_PLOT:
      fgets(buffer, 512, fp);
      sscanf(buffer, "nPanels=%ld", &x);

      while (x > NumberOfXYpanels)
        AddPanel(NULL, NULL, NULL);
 
      while (x < NumberOfXYpanels)
        DeletePanel(NULL, NULL, NULL);

      for (i = 0; i < NumberOfXYpanels; ++i)
        {
        fgets(buffer, 512, fp);
        sscanf(buffer, "Grid=%ld, AutoScale=%ld", &x1, &x2);
        xyyPlot[i].grid = x1;
        xyyPlot[i].autoScale = x2;
 
        fgets(buffer, 512, fp);
        if (version < 2)
          sscanf(buffer, "Xaxis - Invert=%ld, log=%ld", &x1, &x2);
        else
        if (version == 3)
          sscanf(buffer, "Xaxis - Invert=%ld, log=%ld, bounds=%e %e",
                &x1, &x2, &min0, &max0);
        else
          sscanf(buffer, "Xaxis - Invert=%ld, log=%ld, bounds=%e %e, tics=%ld %ld",
                &x1, &x2, &min0, &max0, &x3, &x4);

        xyyPlot[i].Xaxis.invertAxis = x1;
        xyyPlot[i].Xaxis.logScale = x2;

        if (version >= 2)
          {
          xyyPlot[i].Xaxis.min = min0;
          xyyPlot[i].Xaxis.max = max0;
          }

        if (version >= 4)
          {
          xyyPlot[i].Xaxis.nMajorTics = x3;
          xyyPlot[i].Xaxis.nMinorTics = x4;
          }

        fgets(buffer, 512, fp);
        if (version < 2)
          sscanf(buffer, "Yaxis - Invert=%ld %ld, log=%ld %ld", &x1, &x2, &x3, &x4);
        else
        if (version == 3)
          sscanf(buffer, "Yaxis - Invert=%ld %ld, log=%ld %ld, bounds=%e %e %e %e",
                &x1, &x2, &x3, &x4, &min0, &max0, &min1, &max1);
        else
          sscanf(buffer, "Yaxis - Invert=%ld %ld, log=%ld %ld, bounds=%e %e %e %e, tics=%ld %ld",
                &x1, &x2, &x3, &x4, &min0, &max0, &min1, &max1, &x5, &x6);

        xyyPlot[i].Yaxis[0].invertAxis = x1;
        xyyPlot[i].Yaxis[1].invertAxis = x2;
        xyyPlot[i].Yaxis[0].logScale = x3;
        xyyPlot[i].Yaxis[1].logScale = x4;

        if (version >= 2)
          {
          xyyPlot[i].Yaxis[0].min = min0;
          xyyPlot[i].Yaxis[0].max = max0;
          xyyPlot[i].Yaxis[1].min = min1;
          xyyPlot[i].Yaxis[1].max = max1;
          }

        if (version >= 4)
          {
          xyyPlot[i].Yaxis[0].nMajorTics = x5;
          xyyPlot[i].Yaxis[0].nMinorTics = x6;
          xyyPlot[i].Yaxis[1].nMajorTics = x5;
          xyyPlot[i].Yaxis[1].nMinorTics = x6;
          }
        }

      fgets(buffer, 512, fp);
      sscanf(buffer, "nSets=%ld", &x);

      for (i = 0; i < x; ++i)
        {
        fgets(buffer, 512, fp);
        sscanf(buffer, "Panel#=%ld, Axis=%ld, ScaleLoc=%ld, VarName=%s",
			&CurrentPanel, &x3, &x1, s);

        name = XmStringCreateLocalized(s);
        if ((x2 = XmListItemPos(varList, name)) > 0)
          {
          if (x3 == X_AXIS)
            {
            DataChanged = true;
            AddVariable(&xyXset[NumberXYXsets++], s);
            xyyPlot[CurrentPanel].Xaxis.label =
			xyXset[NumberXYXsets-1].stats.units;
            }
          else
            {
            DataChanged = true;
            AddVariable(&xyYset[NumberXYYsets++], s);
            xyyPlot[CurrentPanel].Yaxis[x1].label =
			xyYset[NumberXYYsets-1].stats.units;
            xyYset[NumberXYYsets].scaleLocation = x1;
            }
          }
        else
          {
          sprintf(buffer, "Variable %s not found, continuing.", s);
          HandleError(buffer, Interactive, RETURN);
          }

        XmStringFree(name);
        }

      if (version > 2)
        {
        for (i = 0; i < NumberOfXYpanels; ++i)
          {
          fgets(buffer, 512, fp);
          buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
          xyyPlot[i].Xaxis.label = strchr(buffer, '=')+1;

          fgets(buffer, 512, fp);
          buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
          xyyPlot[i].Yaxis[0].label = strchr(buffer, '=')+1;

          fgets(buffer, 512, fp);
          buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
          xyyPlot[i].Yaxis[1].label = strchr(buffer, '=')+1;
          }
        }

      break;


    case XYZ_PLOT:
      fgets(buffer, 512, fp);
      fgets(buffer, 512, fp);
      sscanf(buffer, "AutoScale=%ld", &x1);
      xyzPlot.autoScale = x1;

      fgets(buffer, 512, fp);
      if (version < 2)
        sscanf(buffer, "Xaxis - Invert=%ld, log=%ld", &x1, &x2);
      else
      if (version == 3)
        sscanf(buffer, "Xaxis - Invert=%ld, log=%ld, bounds=%e %e",
                &x1, &x2, &min0, &max0);
      else
        sscanf(buffer, "Xaxis - Invert=%ld, log=%ld, bounds=%e %e, tics=%ld %ld",
                &x1, &x2, &min0, &max0, &x3, &x4);

      xyzPlot.Xaxis.invertAxis = x1;
      xyzPlot.Xaxis.logScale = x2;

      if (version >= 2)
        {
        xyzPlot.Xaxis.min = min0;
        xyzPlot.Xaxis.max = max0;
        }

      if (version >= 4)
        {
        xyzPlot.Xaxis.nMajorTics = x3;
        xyzPlot.Xaxis.nMinorTics = x4;
        }

      fgets(buffer, 512, fp);
      if (version < 2)
        sscanf(buffer, "Yaxis - Invert=%ld, log=%ld", &x1, &x2);
      else
      if (version == 3)
        sscanf(buffer, "Yaxis - Invert=%ld, log=%ld, bounds=%e %e",
                &x1, &x2, &min0, &max0);
      else
        sscanf(buffer, "Yaxis - Invert=%ld, log=%ld, bounds=%e %e, tics=%ld %ld",
                &x1, &x2, &min0, &max0, &x5, &x6);

      xyzPlot.Yaxis[0].invertAxis = x1;
      xyzPlot.Yaxis[0].logScale = x2;

      if (version >= 2)
        {
        xyzPlot.Yaxis[0].min = min0;
        xyzPlot.Yaxis[0].max = max0;
        }

      if (version >= 4)
        {
        xyzPlot.Yaxis[0].nMajorTics = x5;
        xyzPlot.Yaxis[0].nMinorTics = x6;
        }

      fgets(buffer, 512, fp);
      if (version < 2)
        sscanf(buffer, "Zaxis - Invert=%ld, log=%ld", &x1, &x2);
      else
      if (version == 3)
        sscanf(buffer, "Zaxis - Invert=%ld, log=%ld, bounds=%e %e",
                &x1, &x2, &min0, &max0);
      else
        sscanf(buffer, "Zaxis - Invert=%ld, log=%ld, bounds=%e %e, tics=%ld %ld",
                &x1, &x2, &min0, &max0, &x3, &x4);

      xyzPlot.Zaxis.invertAxis = x1;
      xyzPlot.Zaxis.logScale = x2;

      if (version >= 2)
        {
        xyzPlot.Zaxis.min = min0;
        xyzPlot.Zaxis.max = max0;
        }

      if (version >= 4)
        {
        xyzPlot.Zaxis.nMajorTics = x3;
        xyzPlot.Zaxis.nMinorTics = x4;
        }

      fgets(buffer, 512, fp);
      sscanf(buffer, "nSets=%ld", &x);

      for (i = 0; i < x; ++i)
        {
        fgets(buffer, 512, fp);
        sscanf(buffer, "Axis=%ld, VarName=%s", &x1, s);

        name = XmStringCreateLocalized(s);
        if ((x2 = XmListItemPos(varList, name)) > 0)
          {
          DataChanged = true;
          AddVariable(&xyzSet[x1], s);

          switch (x1)
            {
            case X_AXIS >> 1:
              sprintf(buffer, "%s (%s)",
		xyzSet[x1].varInfo->name.c_str(), xyzSet[x1].stats.units.c_str());
              xyzPlot.Xaxis.label = buffer;
              break;

            case Y_AXIS >> 1:
              sprintf(buffer, "%s (%s)",
		xyzSet[x1].varInfo->name.c_str(), xyzSet[x1].stats.units.c_str());
              xyzPlot.Yaxis[0].label = buffer;
              break;

            case Z_AXIS >> 1:
              sprintf(buffer, "%s (%s)",
		xyzSet[x1].varInfo->name.c_str(), xyzSet[x1].stats.units.c_str());
              xyzPlot.Zaxis.label = buffer;
              break;
            }
          }
        else
          {
          sprintf(buffer, "Variable %s not found, continuing.", s);
          HandleError(buffer, Interactive, RETURN);
          }

        XmStringFree(name);
        }

      if (version > 2)
        {
        fgets(buffer, 512, fp);
        buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
        xyzPlot.Xaxis.label = strchr(buffer, '=')+1;

        fgets(buffer, 512, fp);
        buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
        xyzPlot.Yaxis[0].label = strchr(buffer, '=')+1;

        fgets(buffer, 512, fp);
        buffer[strlen(buffer)-1] = '\0';  /* Remove newline */
        xyzPlot.Zaxis.label = strchr(buffer, '=')+1;
        }

      break;
    }

  fclose(fp);

  findMinMax();
  DrawMainWindow();

}	/* END LOAD_CB2 */

/* END TEMPLATE.C */
