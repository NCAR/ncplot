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
		isMissingValue()

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

static void freeDataSets(int), getNCattr(int ncid, char attr[], std::string& dest);
static bool VarCompareLT(const VARTBL *x, const VARTBL *y);


/* Imported from exp.c */
extern size_t		NumberExpSets;
extern DATASET_INFO	expSet[];


void	findMinMax();

/* -------------------------------------------------------------------- */
void NewDataFile(Widget w, XtPointer client, XtPointer call)
{
  size_t	i, j;
  DATAFILE_INFO	*curFile;

  /* Clear out all existing dataFiles and dataSets.  We're starting from
   * scratch.
   */
  for (i = 0; i < NumberDataFiles; ++i)
    {
    curFile = &dataFile[i];

    nc_close(curFile->ncid);

    for (j = 0; j < curFile->Variable.size(); ++j)
      delete curFile->Variable[j];

    for (j = 1; curFile->catName[j]; ++j)	/* [0] was not malloced	*/
      delete [] curFile->catName[j];

    curFile->Variable.clear();
    }

  freeDataSets(true);
  DataChanged = true;
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

    delete [] timeSeg;
    timeSeg = NULL;
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
  mainPlot[0].title = curFile->ProjectName;

  if (curFile->FlightNumber.length() > 0)
    {
    if (curFile->ProjectName.length() > 0)
      mainPlot[0].title += ", ";

    mainPlot[0].title += "Flight #";
    mainPlot[0].title += curFile->FlightNumber;
    }

  xyyPlot[0].title = mainPlot[0].title;
  specPlot.title = mainPlot[0].title;
  xyzPlot.title = mainPlot[0].title;
  SetSubtitles();
  SetTimeText();

}	/* END NEWDATAFILE */

/* -------------------------------------------------------------------- */
void AddDataFile(Widget w, XtPointer client, XtPointer call)
{
  int		i, InputFile, nVars, nDims, dimIDs[3], varID;
  char		name[NAMELEN], *data_file;
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

    curFile->fileName = data_file;
    }


  size_t indx = curFile->fileName.find_last_of('/');
  if (indx != std::string::npos)
    {
    DataPath = curFile->fileName.substr(0, indx+1);
    DataPath += "*.nc";
    }


  /* See if file exists.
   */
  if (access(curFile->fileName.c_str(), R_OK) != 0)
    {
    sprintf(buffer, "Can't open %s.", curFile->fileName.c_str());
    HandleError(buffer, Interactive, IRET);
    curFile->ncid = 0;
    return;
    }

  /* Open Input File
   */
  if (nc_open(curFile->fileName.c_str(), NC_NOWRITE, &curFile->ncid) != NC_NOERR)
    {
    sprintf(buffer, "Can't open %s.", curFile->fileName.c_str());
    HandleError(buffer, Interactive, IRET);
    return;
    }

  InputFile = curFile->ncid;
  ++NumberDataFiles;

  nc_inq_nvars(InputFile, &nVars);


  getNCattr(InputFile, "ProjectName", curFile->ProjectName);
  getNCattr(InputFile, "ProjectNumber", curFile->ProjectNumber);
  getNCattr(InputFile, "FlightNumber", curFile->FlightNumber);
  getNCattr(InputFile, "FlightDate", curFile->FlightDate);

  std::string warning;
  getNCattr(InputFile, "WARNING", warning);

  if (warning.size() > 0)
    curFile->ShowPrelimDataWarning = true;
  else
    curFile->ShowPrelimDataWarning = false;


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
      curFile->catName[nCats] = new char[strlen(p)+1];
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
  curFile->Variable.clear();
  for (i = 0; i < nVars; ++i)
    {
    nc_inq_var(InputFile, i, name, NULL, &nDims, dimIDs, NULL);

    if (strcmp(name, "base_time") == 0 || strcmp(name, "time_offset") == 0)
      continue;

    if (nDims > 2)
      continue;


    vp = new VARTBL;
    curFile->Variable.push_back(vp);

    strcpy(vp->name, name);
    if (nDims == 1)
      vp->OutputRate = 1;
    else
      nc_inq_dimlen(InputFile, dimIDs[1], (size_t *)&vp->OutputRate);

    if (nc_get_att_float(InputFile, i, "_FillValue", &vp->MissingValue) != NC_NOERR)
      if (nc_get_att_float(InputFile, i, "missing_value", &vp->MissingValue) != NC_NOERR)
        if (nc_get_att_float(InputFile, i, "MissingValue", &vp->MissingValue) != NC_NOERR)
          vp->MissingValue = DEFAULT_MISSING_VALUE;

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

  std::sort(curFile->Variable.begin(), curFile->Variable.end(), VarCompareLT);
  OpenControlWindow(NULL, NULL, NULL);

}	/* END ADDDATAFILE */

/* -------------------------------------------------------------------- */
void SetList()
{
  size_t	i;
  XmString	item[MAX_VARIABLES];
  DATAFILE_INFO	*curFile = &dataFile[CurrentDataFile];

  XmListDeleteAllItems(varList);

  for (i = 0; i < curFile->Variable.size(); ++i)
    item[i] = XmStringCreateLocalized(curFile->Variable[i]->name);

  XmListAddItems(varList, item, curFile->Variable.size(), 1);

  for (i = 0; i < curFile->Variable.size(); ++i)
    XmStringFree(item[i]);

}	/* END SETLIST */

/* -------------------------------------------------------------------- */
static void readSet(DATASET_INFO *set)
{
  size_t	start[3], count[3];
  size_t	i, frontPad, endPad;
  DATAFILE_INFO *file;

  WaitCursorAll();

  if (set->varInfo->inVarID == COMPUTED)
    {
    size_t whichExp = set->varInfo->name[4] - '1';;

    set->nPoints = NumberSeconds;

    for (i = 0; i < NumberExpSets; ++i) 
      if (expSet[i].panelIndex == whichExp && expSet[i].nPoints == 0)
        {
        readSet(&expSet[i]);
        set->nPoints = expSet[i].nPoints;
        }

    set->missingValue = DEFAULT_MISSING_VALUE;
    set->data = new NR_TYPE[set->nPoints];
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

  set->data = new NR_TYPE[set->nPoints];

  set->missingValue = set->varInfo->MissingValue;

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
  size_t i;

  Freeze = true;

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

  freeDataSets(false);

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

  Freeze = false;
  DataChanged = true;

}	/* END READDATA */

/* -------------------------------------------------------------------- */
void findMinMax()
{
  size_t plot, set;

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
            std::min(mainPlot[plot].Yaxis[dataSet[set].scaleLocation].smallestValue, dataSet[set].stats.min);

        mainPlot[plot].Yaxis[dataSet[set].scaleLocation].biggestValue =
            std::max(mainPlot[plot].Yaxis[dataSet[set].scaleLocation].biggestValue, dataSet[set].stats.max);
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
           std::min(xyyPlot[plot].Xaxis.smallestValue, xyXset[set].stats.min);

        xyyPlot[plot].Xaxis.biggestValue =
           std::max(xyyPlot[plot].Xaxis.biggestValue, xyXset[set].stats.max);
        }
      }

    for (set = 0; set < NumberXYYsets; ++set)
      {
      if (xyYset[set].panelIndex == plot)
        {
        xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].smallestValue =
           std::min(xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].smallestValue,
           xyYset[set].stats.min);

        xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].biggestValue =
           std::max(xyyPlot[plot].Yaxis[xyYset[set].scaleLocation].biggestValue,
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
  if ((indx = SearchTable(dataFile[CurrentDataFile].Variable, varName)) == ERR)
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
    VARTBL	*vp;
    static int	expCnt = 0;

    vp = new VARTBL;
    dataFile[CurrentDataFile].Variable.push_back(vp);
    SetList();

    set->varInfo = vp;

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
  size_t i, rate, set;
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
  DataChanged = true;

  for (i = 0; i < NumberDataSets; ++i)
    ComputeStats(&dataSet[i]);

  findMinMax();

}	/* END REDUCEDATA */

/* -------------------------------------------------------------------- */
int DeleteVariable(DATASET_INFO *sets, size_t nSets, int indx)
{
  size_t set;
  bool rc = false;

  for (set = 0; set < nSets; ++set)
    if (sets[set].fileIndex == CurrentDataFile &&
        sets[set].panelIndex == CurrentPanel &&
        sets[set].varInfo == dataFile[sets[set].fileIndex].Variable[indx])
      {
      delete [] sets[set].data;

      for (++set; set < nSets; ++set)
        sets[set-1] = sets[set];

      --nSets;

      rc = true;
      break;
      }

  sets[nSets].varInfo = NULL;
  sets[nSets].nPoints = 0;

  return(rc);

}	/* END DELETEVARIABLE */

/* -------------------------------------------------------------------- */
static void freeDataSets(int mode)
{
  size_t set;

  for (set = 0; set < NumberExpSets; ++set)
    {
    if (mode)
      delete expSet[set].varInfo;

    if (expSet[set].nPoints > 0)
      delete [] expSet[set].data;

    expSet[set].nPoints = 0;
    }

  for (set = 0; set < NumberDataSets; ++set)
    {
    if (mode)
      dataSet[set].varInfo = NULL;

    dataSet[set].nPoints = 0;
    delete [] dataSet[set].data;
    }


  for (set = 0; set < NumberXYXsets; ++set)
    {
    if (mode)
      xyXset[set].varInfo = NULL;

    xyXset[set].nPoints = 0;
    delete [] xyXset[set].data;
    }

  for (set = 0; set < NumberXYYsets; ++set)
    {
    if (mode)
      xyYset[set].varInfo = NULL;

    xyYset[set].nPoints = 0;
    delete [] xyYset[set].data;
    }


  for (set = 0; set < 3; ++set)
    if (xyzSet[set].varInfo)
      {
      if (mode)
        xyzSet[set].varInfo = NULL;

      xyzSet[set].nPoints = 0;
      delete [] xyzSet[set].data;
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
static void getNCattr(int ncid, char attr[], std::string& dest)
{
  size_t len;

  if (nc_inq_attlen(ncid, NC_GLOBAL, attr, &len) == NC_NOERR)
    {
    nc_get_att_text(ncid, NC_GLOBAL, attr, buffer);
    buffer[len] = '\0';

    while (buffer[--len] < 0x20)	/* Remove extraneous CR/LF, etc */
      buffer[len] = '\0';

    dest = buffer;
    }

}	/* END GETNCATTR */

/* -------------------------------------------------------------------- */
bool isMissingValue(float target, float fillValue)
{
  if (isnan(fillValue) && isnan(target))
    return(true);
  else
    if (fillValue == target)
      return(true);

  return(false);

}	/* END ISMISSINGVALUE */

/* -------------------------------------------------------------------- */
static bool VarCompareLT(const VARTBL *x, const VARTBL *y)
{
    return(strcmp(x->name, y->name) < 0);
}

/* END DATAIO.C */
