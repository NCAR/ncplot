/*
-------------------------------------------------------------------------
OBJECT NAME:	dataIO.c

FULL NAME:	Data File IO

ENTRY POINTS:	AddDataFile()
		AddVariable()
		DeleteVariable()
		LoadVariable()
		NewDataFile()
		ReadData()
		ReduceData()
		SetList()

STATIC FNS:	findMinMax()
		freeDataSets()
		getNCattr()
		readSet()

DESCRIPTION:	

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <time.h>
#include <unistd.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>

#define NO_NETCDF_2

#include "netcdf.h"

static void freeDataSets(int), getNCattr(int ncid, char attr[], char **dest);


/* Imported from exp.c */
extern int		NumberExpSets;
extern DATASET_INFO	expSet[];


void	findMinMax();

/* -------------------------------------------------------------------- */
void NewDataFile(Widget w, XtPointer client, XtPointer call)
{
  int		i, j;
  DATAFILE_INFO	*curFile;

  /* Clear out all existing dataFiles and dataSets.  We're starting from
   * scratch.
   */
  for (i = 0; i < NumberDataFiles; ++i)
    {
    curFile = &dataFile[i];

    nc_close(curFile->ncid);

    FreeMemory(curFile->ProjectName);
    FreeMemory(curFile->ProjectNumber);
    FreeMemory(curFile->FlightNumber);
    FreeMemory(curFile->FlightDate);

    for (j = 0; j < curFile->nVariables; ++j)
      FreeMemory((char *)curFile->Variable[j]);

    for (j = 1; curFile->catName[j]; ++j)	/* [0] was not malloced	*/
      FreeMemory(curFile->catName[j]);

    curFile->nVariables = 0;
    }

  freeDataSets(True);
  DataChanged = True;
  CurrentDataFile = NumberDataFiles = NumberDataSets = NumberXYXsets =
	NumberXYYsets = 0;
  curFile = &dataFile[CurrentDataFile];

  AddDataFile(w, client, call);

  if (curFile->ncid == 0)	/* If we failed to open file */
    return;

  SetDataFile(NULL, 0, NULL);


  /* Set default TimeSlice to whole flight.
   */
  if (timeSeg)	/* From command line only	*/
    {
    sscanf(timeSeg, "%02d:%02d:%02d-%02d:%02d:%02d",
        &UserStartTime[0], &UserStartTime[1], &UserStartTime[2],
        &UserEndTime[0], &UserEndTime[1], &UserEndTime[2]);

	if (UserStartTime[0] > UserEndTime[0])
      UserEndTime[0] += 24;

    FreeMemory(timeSeg); timeSeg = NULL;
    }
  else
    {
    memcpy((char *)UserStartTime, (char *)curFile->FileStartTime,4*sizeof(int));
    memcpy((char *)UserEndTime, (char *)curFile->FileEndTime,4*sizeof(int));
    }

  UserEndTime[3] = SecondsSinceMidnite(UserEndTime);
  UserStartTime[3] = SecondsSinceMidnite(UserStartTime);


  if (UserEndTime[3] > curFile->FileEndTime[3] && !RealTime)
    memcpy((char *)UserEndTime, (char *)curFile->FileEndTime, 4*sizeof(int));

  NumberSeconds = UserEndTime[3] - UserStartTime[3];

  /* Set up titles & subtitles.
   */
  strcpy(mainPlot[0].title, curFile->ProjectName);

  if (strlen(curFile->FlightNumber) > 0)
    {
    if (strlen(curFile->ProjectName))
      strcat(mainPlot[0].title, ", ");

    strcat(mainPlot[0].title, "Flight #");
    strcat(mainPlot[0].title, curFile->FlightNumber);
    }

  strcpy(xyyPlot[0].title, mainPlot[0].title);
  strcpy(specPlot.title, mainPlot[0].title);
  strcpy(xyzPlot.title, mainPlot[0].title);
  SetSubtitles();
  SetTimeText();

}	/* END NEWDATAFILE */

/* -------------------------------------------------------------------- */
void AddDataFile(Widget w, XtPointer client, XtPointer call)
{
  int		i, InputFile, nVars, nDims, dimIDs[3], varID;
  char		name[NAMELEN], *data_file, *p;
  DATAFILE_INFO	*curFile;
  VARTBL	*vp;

  if (NumberDataFiles >= MAX_DATAFILES)
    {
    HandleError("Currently at maximum allowed files.", Interactive, IRET);
    return;
    }

  curFile = &dataFile[NumberDataFiles];

  if (w)
    {
    FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

    ExtractFileName(((XmFileSelectionBoxCallbackStruct *)call)->value,
                    &data_file);

    strcpy(curFile->fileName, data_file);
    }


  strcpy(DataPath, curFile->fileName);
  if ((p = strrchr(DataPath, '/')))
    strcpy(p+1, "*.nc");


  /* See if file exists.
   */
  if (access(curFile->fileName, R_OK) != 0)
    {
    sprintf(buffer, "Can't open %s.", curFile->fileName);
    HandleError(buffer, Interactive, IRET);
    curFile->ncid = 0;
    return;
    }

  /* Open Input File
   */
  if (nc_open(curFile->fileName, NC_NOWRITE, &curFile->ncid) != NC_NOERR)
    {
    sprintf(buffer, "Can't open %s.", curFile->fileName);
    HandleError(buffer, Interactive, IRET);
    return;
    }

  InputFile = curFile->ncid;
  ++NumberDataFiles;

  nc_inq_nvars(InputFile, &nVars);

  if (nVars > MAX_VARIABLES)
    {
    sprintf(buffer, "Number of variables in file [%d], exceeds current ncplot maximum [%d],\nclipping, modify define.h and recompile to see all variables from this file.", nVars, MAX_VARIABLES);
    HandleError(buffer, Interactive, IRET);
    nVars = MAX_VARIABLES;
    }


  getNCattr(InputFile, "ProjectName", &curFile->ProjectName);
  getNCattr(InputFile, "ProjectNumber", &curFile->ProjectNumber);
  getNCattr(InputFile, "FlightNumber", &curFile->FlightNumber);
  getNCattr(InputFile, "FlightDate", &curFile->FlightDate);

  GetTimeInterval(InputFile, curFile);


  /* Get Category list.
   */
  curFile->catName[0] = "All variables";
  curFile->catName[1] = NULL;

  if (nc_get_att_text(InputFile, NC_GLOBAL, "Categories", buffer) == NC_NOERR)
    {
    char *p;
    int	nCats;

    p = strtok(buffer, ",");
 
    for (nCats = 1; p; ++nCats)
      {
      curFile->catName[nCats] = (char *)GetMemory(strlen(p)+1);
      strcpy(curFile->catName[nCats], p);
 
      p = strtok(NULL, ",");
      }

    curFile->catName[nCats] = NULL;
    }


  /* Find which file has min start time, and max end time.
   */
  if (NumberDataFiles == 1 || MinStartTime[3] > curFile->FileStartTime[3])
    memcpy((char *)MinStartTime, (char *)curFile->FileStartTime, 4*sizeof(int));

  if (NumberDataFiles == 1 || MaxEndTime[3] < curFile->FileEndTime[3])
    memcpy((char *)MaxEndTime, (char *)curFile->FileEndTime, 4*sizeof(int));


  /* Read in the variables.
   */
  curFile->nVariables = 0;
  for (i = 0; i < nVars; ++i)
    {
    nc_inq_var(InputFile, i, name, NULL, &nDims, dimIDs, NULL);

    if (strcmp(name, "base_time") == 0 || strcmp(name, "time_offset") == 0)
      continue;

    if (nDims > 2)
      continue;


    vp = curFile->Variable[curFile->nVariables++] =
           (VARTBL *)GetMemory(sizeof(VARTBL));

    strcpy(vp->name, name);
    if (nDims == 1)
      vp->OutputRate = 1;
    else
      nc_inq_dimlen(InputFile, dimIDs[1], (size_t *)&vp->OutputRate);

    vp->inVarID	= i;
    }


  if (nc_inq_varid(InputFile, "time_offset", &varID) == NC_NOERR)
    {
    size_t start[2], count[2];
    float tf[2];

    start[0] = 0; start[1] = 0;
    count[0] = 2; count[1] = 1;

    nc_get_vara_float(InputFile, varID, start, count, tf);

    curFile->baseDataRate = (int)(tf[1] - tf[0]);
    }
  else
    curFile->baseDataRate = 1;

  SortTable((char **)curFile->Variable, 0, curFile->nVariables - 1);
  OpenControlWindow(NULL, NULL, NULL);

}	/* END ADDDATAFILE */

/* -------------------------------------------------------------------- */
void SetList()
{
  int		i;
  XmString	item[MAX_VARIABLES];
  DATAFILE_INFO	*curFile = &dataFile[CurrentDataFile];

  XmListDeleteAllItems(varList);

  for (i = 0; i < curFile->nVariables; ++i)
    item[i] = XmStringCreateLocalized(curFile->Variable[i]->name);

  XmListAddItems(varList, item, curFile->nVariables, 1);

  for (i = 0; i < curFile->nVariables; ++i)
    XmStringFree(item[i]);

}	/* END SETLIST */

/* -------------------------------------------------------------------- */
static void readSet(DATASET_INFO *set)
{
  size_t	start[3], count[3];
  int		i, frontPad, endPad;
  DATAFILE_INFO *file;

  WaitCursorAll();

  if (set->varInfo->inVarID == COMPUTED)
    {
    int	whichExp = set->varInfo->name[4] - '1';;

    set->nPoints = NumberSeconds;

    for (i = 0; i < NumberExpSets; ++i) 
      if (expSet[i].panelIndex == whichExp && expSet[i].nPoints == 0)
{printf("  reading expSet %s, nPts=%d\n", expSet[i].varInfo->name, expSet[i].nPoints);
        {
        readSet(&expSet[i]);
        set->nPoints = expSet[i].nPoints;
        }
}

    set->missingValue = MISSING_VALUE;
    set->data = (NR_TYPE *)GetMemory(set->nPoints * sizeof(NR_TYPE));
    ComputeExp(set);
    goto bottom;
    }

  file = &dataFile[set->fileIndex];

  if (UserStartTime[3] > file->FileStartTime[3])
    {
    frontPad = 0;
    start[0] = (UserStartTime[3] - file->FileStartTime[3]) / file->baseDataRate;
    }
  else
    {
    frontPad = (file->FileStartTime[3] - UserStartTime[3]) / file->baseDataRate;
    start[0] = 0;
    }

  start[1] = start[2] = 0;

  count[0] = set->nPoints = NumberSeconds / file->baseDataRate;
  count[1] = set->varInfo->OutputRate;
  count[2] = 1;

  if (UserEndTime[3] < file->FileEndTime[3])
    endPad = 0;
  else
    endPad = (UserEndTime[3] - file->FileEndTime[3]) / file->baseDataRate;

  count[0] -= frontPad + endPad;

  if (UserEndTime[3] < file->FileStartTime[3])
    {
    frontPad = NumberSeconds / file->baseDataRate;
    count[0] = endPad = 0;
    }

  if (UserStartTime[3] > file->FileEndTime[3])
    {
    endPad = NumberSeconds / file->baseDataRate;
    count[0] = frontPad = 0;
    }

  frontPad *= set->varInfo->OutputRate;
  endPad *= set->varInfo->OutputRate;
  set->nPoints *= set->varInfo->OutputRate;

  set->data = (NR_TYPE *)GetMemory(set->nPoints * sizeof(NR_TYPE));

  if (nc_get_att_float(file->ncid, set->varInfo->inVarID, "missing_value",
	&set->missingValue) != NC_NOERR)
    if (nc_get_att_float(file->ncid, set->varInfo->inVarID, "MissingValue",
	&set->missingValue) != NC_NOERR)
      set->missingValue = MISSING_VALUE;

  nc_get_vara_float(file->ncid, set->varInfo->inVarID, start, count,
           &set->data[frontPad]);

  for (i = 0; i < frontPad; ++i)
    set->data[i] = set->missingValue;

  for (i = (count[0] * count[1]) + frontPad; i < set->nPoints; ++i)
    set->data[i] = set->missingValue;

  if (RealTime)
    set->head = frontPad + (count[0] * count[1]);

bottom:
  ComputeStats(set);

  PointerCursorAll();

}	/* END READSET */

/* -------------------------------------------------------------------- */
void ReadData()
{
  int	i;

  Freeze = True;

  /* Perform time computations.
   */
  if (RealTime)
    {
    memcpy((char*)UserStartTime, (char*)dataFile[0].FileEndTime, 4*sizeof(int));
    memcpy((char*)UserEndTime,  (char *)dataFile[0].FileEndTime, 4*sizeof(int));

    UserStartTime[3] -= NumberSeconds;
    FromSecondsSinceMidnite(UserStartTime);
    }
  else
    {
    if (UserStartTime[3] < MinStartTime[3])
      memcpy((char *)UserStartTime, (char *)MinStartTime, 4*sizeof(int));
 
    if (UserEndTime[3] > MaxEndTime[3])
      memcpy((char *)UserEndTime, (char *)MaxEndTime, 4*sizeof(int));
 
    NumberSeconds = UserEndTime[3] - UserStartTime[3];
    }

  freeDataSets(False);

  for (i = 0; i < NumberExpSets; ++i)
    readSet(&expSet[i]);

  for (i = 0; i < NumberDataSets; ++i)
    readSet(&dataSet[i]);

  for (i = 0; i < NumberXYXsets; ++i)
    if (xyXset[i].varInfo)
      readSet(&xyXset[i]);

  for (i = 0; i < NumberXYYsets; ++i)
    if (xyYset[i].varInfo)
      readSet(&xyYset[i]);

  for (i = 0; i < 3; ++i)
    if (xyzSet[i].varInfo)
      readSet(&xyzSet[i]);


  if (WindBarbs && PlotType == XY_PLOT)
    {
    readSet(&ui);
    readSet(&vi);
    }

  if (plotWaveNumber() || plotWaveLength())
    readSet(&tas);

  findMinMax();
  SetSubtitles();
  SetTimeText();

  Freeze = False;
  DataChanged = True;

}	/* END READDATA */

/* -------------------------------------------------------------------- */
void findMinMax()
{
  int	plot, set;

  for (plot = 0; plot < NumberOfPanels; ++plot)
    {
    mainPlot[plot].Xaxis.biggestValue = mainPlot[plot].Yaxis[0].biggestValue =
	mainPlot[plot].Yaxis[1].biggestValue = -FLT_MAX;

    mainPlot[plot].Xaxis.smallestValue = mainPlot[plot].Yaxis[0].smallestValue =
	mainPlot[plot].Yaxis[1].smallestValue = FLT_MAX;

    for (set = 0; set < NumberDataSets; ++set)
      if (dataSet[set].panelIndex == plot)
        {
        mainPlot[plot].Yaxis[dataSet[set].scaleLocation].smallestValue =
            MIN(mainPlot[plot].Yaxis[dataSet[set].scaleLocation].smallestValue,
                dataSet[set].stats.min);

        mainPlot[plot].Yaxis[dataSet[set].scaleLocation].biggestValue =
            MAX(mainPlot[plot].Yaxis[dataSet[set].scaleLocation].biggestValue,
                dataSet[set].stats.max);
        }
    }


  for (plot = 0; plot < NumberOfXYpanels; ++plot)
    {
    xyyPlot[plot].Xaxis.biggestValue = xyyPlot[plot].Yaxis[0].biggestValue =
	xyyPlot[plot].Yaxis[1].biggestValue = -FLT_MAX;
    xyyPlot[plot].Xaxis.smallestValue = xyyPlot[plot].Yaxis[0].smallestValue =
	xyyPlot[plot].Yaxis[1].smallestValue = FLT_MAX;

    for (set = 0; set < NumberXYXsets; ++set)
      {
      if (xyXset[set].panelIndex == plot)
        {
        xyyPlot[plot].Xaxis.smallestValue =
           MIN(xyyPlot[plot].Xaxis.smallestValue, xyXset[set].stats.min);

        xyyPlot[plot].Xaxis.biggestValue =
           MAX(xyyPlot[plot].Xaxis.biggestValue, xyXset[set].stats.max);
        }
      }

    for (set = 0; set < NumberXYYsets; ++set)
      {
      if (xyYset[set].panelIndex == plot)
        {
        xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].smallestValue =
           MIN(xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].smallestValue,
           xyYset[set].stats.min);

        xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].biggestValue =
           MAX(xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].biggestValue,
           xyYset[set].stats.max);
        }
      }
    }


  if (PlotType == XYZ_PLOT)
    {
    xyzPlot.Xaxis.smallestValue = xyzSet[0].stats.min;
    xyzPlot.Xaxis.biggestValue  = xyzSet[0].stats.max;
    xyzPlot.Yaxis[0].smallestValue	= xyzSet[1].stats.min;
    xyzPlot.Yaxis[0].biggestValue	= xyzSet[1].stats.max;
    xyzPlot.Zaxis.smallestValue	= xyzSet[2].stats.min;
    xyzPlot.Zaxis.biggestValue	= xyzSet[2].stats.max;
    }

}	/* END FINDMINMAX */

/* -------------------------------------------------------------------- */
int LoadVariable(DATASET_INFO *set, char varName[])
{
  int	indx;

  set->fileIndex = CurrentDataFile;
  set->panelIndex = CurrentPanel;
  set->data = NULL;
  set->head = 0;

  set->stats.outlierMin = -FLT_MAX;
  set->stats.outlierMax = FLT_MAX;

  /* Search variable, return error if not found.
   */
  if ((indx = SearchTable((char **)dataFile[CurrentDataFile].Variable,
              dataFile[CurrentDataFile].nVariables, varName)) == ERR)
    return(ERR);

  set->varInfo = dataFile[CurrentDataFile].Variable[indx];

  readSet(set);

  return(OK);

}	/* END LOADVARIABLE */

/* -------------------------------------------------------------------- */
void AddVariable(DATASET_INFO *set, int indx)
{
  set->fileIndex = CurrentDataFile;
  set->panelIndex = CurrentPanel;
  set->scaleLocation = whichSide();

  if (indx != COMPUTED)
    set->varInfo = dataFile[CurrentDataFile].Variable[indx];
  else
    {
    static int	expCnt = 0;

    indx = dataFile[CurrentDataFile].nVariables++;
    dataFile[CurrentDataFile].Variable[indx]
		= (VARTBL *)GetMemory(sizeof(VARTBL));
    SetList();

    set->varInfo = dataFile[CurrentDataFile].Variable[indx];

    sprintf(set->varInfo->name, "USER%d", expCnt);
    set->varInfo->OutputRate = 1;
    set->varInfo->inVarID = COMPUTED;
    }

  set->data = NULL;
  set->head = 0;

  set->stats.outlierMin = -FLT_MAX;
  set->stats.outlierMax = FLT_MAX;

  readSet(set);

}	/* END ADDVARIABLE */

/* -------------------------------------------------------------------- */
void ReduceData(int start, int newNumberSeconds)
{
  int	i, rate, set;
  int	hours, mins, sex;

  for (set = 0; set < NumberDataSets; ++set)
    {
    rate = dataSet[set].nPoints / NumberSeconds;
    dataSet[set].nPoints = newNumberSeconds * rate;

    memcpy((char *)dataSet[set].data, (char *)&dataSet[set].data[start*rate],
			dataSet[set].nPoints * sizeof(NR_TYPE));

    dataSet[set].data = (NR_TYPE *)realloc((void *)dataSet[set].data,
                dataSet[set].nPoints * sizeof(NR_TYPE));
    }

  for (set = 0; set < NumberExpSets; ++set)
    {
    if (expSet[set].nPoints == 0)
      continue;

    rate = expSet[set].nPoints / NumberSeconds;
    expSet[set].nPoints = newNumberSeconds * rate;

    memcpy((char *)expSet[set].data, (char *)&expSet[set].data[start*rate],
			expSet[set].nPoints * sizeof(NR_TYPE));

    expSet[set].data = (NR_TYPE *)realloc((void *)expSet[set].data,
                expSet[set].nPoints * sizeof(NR_TYPE));
    }

  NumberSeconds = newNumberSeconds;


  /* Increment User Start Time accordingly.
   */
  hours = start / 3600; start -= hours * 3600;
  mins = start / 60; start -= mins * 60;
  sex = start;

  if ((UserStartTime[2] += sex) > 59) {
    UserStartTime[2] -= 60;
    ++UserStartTime[1];
    }

  if ((UserStartTime[1] += mins) > 59) {
    UserStartTime[1] -= 60;
    ++UserStartTime[0];
    }

  UserStartTime[0] += hours;


  /* Set User End Time accordingly.
   */
  hours = newNumberSeconds / 3600; newNumberSeconds -= hours * 3600;
  mins = newNumberSeconds / 60; newNumberSeconds -= mins * 60;
  sex = newNumberSeconds;

  UserEndTime[0] = UserStartTime[0] + hours;
  UserEndTime[1] = UserStartTime[1] + mins;
  UserEndTime[2] = UserStartTime[2] + sex;

  if (UserEndTime[2] > 59) {
    UserEndTime[2] -= 60;
    ++UserEndTime[1];
    }

  if (UserEndTime[1] > 59) {
    UserEndTime[1] -= 60;
    ++UserEndTime[0];
    }

  UserStartTime[3] = SecondsSinceMidnite(UserStartTime);
  UserEndTime[3] = SecondsSinceMidnite(UserEndTime);

  SetSubtitles();
  SetTimeText();
  DataChanged = True;

  for (i = 0; i < NumberDataSets; ++i)
    ComputeStats(&dataSet[i]);

  findMinMax();

}	/* END REDUCEDATA */

/* -------------------------------------------------------------------- */
int DeleteVariable(DATASET_INFO *sets, int nSets, int indx)
{
  int	set;
  bool	rc = False;

  for (set = 0; set < nSets; ++set)
    if (sets[set].fileIndex == CurrentDataFile &&
        sets[set].panelIndex == CurrentPanel &&
        sets[set].varInfo == dataFile[sets[set].fileIndex].Variable[indx])
      {
      FreeMemory((char *)sets[set].data);

      for (++set; set < nSets; ++set)
        sets[set-1] = sets[set];

      --nSets;

      rc = True;
      break;
      }

  sets[nSets].varInfo = NULL;
  sets[nSets].nPoints = 0;

  return(rc);

}	/* END DELETEVARIABLE */

/* -------------------------------------------------------------------- */
static void freeDataSets(int mode)
{
  int	set;

  for (set = 0; set < NumberExpSets; ++set)
    {
    if (mode)
      free(expSet[set].varInfo);

    if (expSet[set].nPoints > 0)
      FreeMemory((char *)expSet[set].data);

    expSet[set].nPoints = 0;
    }

  for (set = 0; set < NumberDataSets; ++set)
    {
    if (mode)
      dataSet[set].varInfo = NULL;

    dataSet[set].nPoints = 0;
    FreeMemory((char *)dataSet[set].data);
    }


  for (set = 0; set < NumberXYXsets; ++set)
    {
    if (mode)
      xyXset[set].varInfo = NULL;

    xyXset[set].nPoints = 0;
    FreeMemory((char *)xyXset[set].data);
    }

  for (set = 0; set < NumberXYYsets; ++set)
    {
    if (mode)
      xyYset[set].varInfo = NULL;

    xyYset[set].nPoints = 0;
    FreeMemory((char *)xyYset[set].data);
    }


  for (set = 0; set < 3; ++set)
    if (xyzSet[set].varInfo)
      {
      if (mode)
        xyzSet[set].varInfo = NULL;

      xyzSet[set].nPoints = 0;
      FreeMemory((char *)xyzSet[set].data);
      }

}	/* END FREEDATASETS */

/* -------------------------------------------------------------------- */
void GetTimeInterval(int InputFile, DATAFILE_INFO *curFile)
{
  /* Perform time computations.
   */
  nc_get_att_text(InputFile, NC_GLOBAL, "TimeInterval", buffer);
  sscanf(buffer, "%02d:%02d:%02d-%02d:%02d:%02d",
         &curFile->FileStartTime[0], &curFile->FileStartTime[1],
         &curFile->FileStartTime[2],
         &curFile->FileEndTime[0], &curFile->FileEndTime[1],
         &curFile->FileEndTime[2]);

  if (curFile->FileStartTime[0] > curFile->FileEndTime[0])
    curFile->FileEndTime[0] += 24;
 
  /* Seconds since midnight.
   */
  curFile->FileStartTime[3] = SecondsSinceMidnite(curFile->FileStartTime);
  curFile->FileEndTime[3] = SecondsSinceMidnite(curFile->FileEndTime);

}	/* END GETTIMEINTERVAL */

/* -------------------------------------------------------------------- */
static void getNCattr(int ncid, char attr[], char **dest)
{
  size_t len;

  if (nc_inq_attlen(ncid, NC_GLOBAL, attr, &len) == NC_NOERR)
    {
    *dest = (char *)GetMemory(len+1);
    nc_get_att_text(ncid, NC_GLOBAL, attr, *dest);
    (*dest)[len] = '\0';

    while ((*dest)[--len] < 0x20)	/* Remove extraneous CR/LF, etc */
      (*dest)[len] = '\0';
    }
  else
    {
    *dest = (char *)GetMemory(1);
    *dest[0] = '\0';
    }

}	/* END GETNCATTR */

/* END DATAIO.C */
