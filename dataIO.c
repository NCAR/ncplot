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

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2007
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <ctime>
#include <unistd.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/TextF.h>

#define NO_NETCDF_2

#include <netcdf.h>

static void freeDataSets(int);
bool getNCattr(int ncid, int varID, const char attr[], std::string & dest);
static bool VarCompareLT(const VARTBL *x, const VARTBL *y);
static int baseRate(const float tf[], int n);


/* Imported from exp.c */
extern std::vector<DATASET_INFO> expSet;

void	performSanityChecks(int InputFile, DATAFILE_INFO * curFile);
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

    curFile->Variable.clear();
    curFile->categories.clear();
    }

  freeDataSets(true);
  DataChanged = true;
  void ClearLandmarks();
  ClearLandmarks();
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
  int		i, InputFile, nVars, nDims, dimIDs[3];
  int		rc;
  char		name[100], *data_file;
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
    DataPath = ExtractFileDialogFilter();
    }


  /* See if file exists.
   */
  curFile->ncid = 0;
  if (access(curFile->fileName.c_str(), R_OK) != 0)
    {
    sprintf(buffer, "Can't open %s.", curFile->fileName.c_str());
    HandleError(buffer, Interactive, IRET);
    return;
    }

  /* Open Input File
   */
  rc = nc_open(curFile->fileName.c_str(), NC_NOWRITE, &InputFile);
  if (rc != NC_NOERR)
    {
    sprintf(buffer, "Can't open %s.\n\n%s", curFile->fileName.c_str(), nc_strerror(rc));
    HandleError(buffer, Interactive, IRET);
    return;
    }

  /* Get Time dimension.
   */
  int time_dim;
  if (nc_inq_dimid(InputFile, "Time", &time_dim) != NC_NOERR)
    {
    if (nc_inq_dimid(InputFile, "time", &time_dim) != NC_NOERR)
      {
      HandleError("File does not contain the Time dimension.", Interactive, IRET);
      nc_close(InputFile);
      return;
      }
    }

  /* Get Time dimension.
   */
  int timeVarID;
  if (nc_inq_varid(InputFile, "Time", &timeVarID) != NC_NOERR)
    if (nc_inq_varid(InputFile, "time", &timeVarID) != NC_NOERR)
      if (nc_inq_varid(InputFile, "time_offset", &timeVarID) != NC_NOERR)
        {
        HandleError("File does not contain the Time variable.", Interactive, IRET);
        nc_close(InputFile);
        return;
        }


  curFile->ncid = InputFile;
  ++NumberDataFiles;

  nc_inq_nvars(InputFile, &nVars);


  if (getNCattr(InputFile, NC_GLOBAL, "ProjectName", curFile->ProjectName) == false)
    getNCattr(InputFile, NC_GLOBAL, "project", curFile->ProjectName);	// Unidata DataDiscovery name.

  getNCattr(InputFile, NC_GLOBAL, "FlightNumber", curFile->FlightNumber);
  getNCattr(InputFile, NC_GLOBAL, "FlightDate", curFile->FlightDate);
  if (getNCattr(InputFile, NC_GLOBAL, "Platform", curFile->TailNumber) == false)
    getNCattr(InputFile, NC_GLOBAL, "Aircraft", curFile->TailNumber);

  std::string warning;
  getNCattr(InputFile, NC_GLOBAL, "WARNING", warning);

  if (warning.size() > 0)
    curFile->ShowPrelimDataWarning = true;
  else
    curFile->ShowPrelimDataWarning = false;

  std::string landmarks;
  getNCattr(InputFile, NC_GLOBAL, "landmarks", landmarks);

  if (landmarks.size() > 0)
  {
    void setLandmarks(const std::string);
    setLandmarks(landmarks);
  }


  GetTimeInterval(InputFile, curFile);

  /* Find which file has min start time, and max end time.
   */
  if (NumberDataFiles == 1 || MinStartTime[3] > curFile->FileStartTime[3])
    memcpy((char *)MinStartTime, (char *)curFile->FileStartTime, 4*sizeof(int));

  if (NumberDataFiles == 1 || MaxEndTime[3] < curFile->FileEndTime[3])
    memcpy((char *)MaxEndTime, (char *)curFile->FileEndTime, 4*sizeof(int));


  /* Read in the variables.
   */
  curFile->Variable.clear();
  curFile->categories.clear();
  for (i = 0; i < nVars; ++i)
    {
    nc_inq_var(InputFile, i, name, NULL, &nDims, dimIDs, NULL);

    if (dimIDs[0] != time_dim || nDims > 2 || strcmp(name, "time_offset") == 0)
      continue;

    vp = new VARTBL;
    curFile->Variable.push_back(vp);

    vp->name = name;
    if (nDims == 1)
      vp->OutputRate = 1;
    else
      nc_inq_dimlen(InputFile, dimIDs[1], (size_t *)&vp->OutputRate);

    if (nc_get_att_float(InputFile, i, "_FillValue", &vp->MissingValue) != NC_NOERR)
      if (nc_get_att_float(InputFile, i, "missing_value", &vp->MissingValue) != NC_NOERR)
        if (nc_get_att_float(InputFile, i, "MissingValue", &vp->MissingValue) != NC_NOERR)
          vp->MissingValue = DEFAULT_MISSING_VALUE;

    getNCattr(InputFile, i, "units", vp->units);
    getNCattr(InputFile, i, "long_name", vp->long_name);

    if (nc_get_att_text(InputFile, i, "Category", buffer) == NC_NOERR)
      {
      size_t len;
      nc_inq_attlen(InputFile, i, "Category", (size_t *)&len);
      buffer[len] = '\0';

      for (char *p = strtok(buffer, ","); p; p = strtok(NULL, ","))
        {
        vp->categories.insert(p);
        curFile->categories.insert(p);
        }
      }

    vp->inVarID	= i;
    }


  /* Compute what I call the base data rate.  Most files will be 1 second files,
   * including high-rate files.  This is for files with slow data, say every 5
   * or 10 seconds.
   */
  curFile->baseDataRate = 1;
  if (timeVarID != -1)
    {
    int max_read = 120;
    size_t start[2], count[2];
    float tf[max_read];
    int dimids[3];
    size_t recs;
    int days;

    max_read = std::min((size_t)120, NumberSeconds);

    start[0] = 0; start[1] = 0;
    count[0] = max_read; count[1] = 1;

    nc_get_vara_float(InputFile, timeVarID, start, count, tf);

    curFile->baseDataRate = baseRate(tf, max_read);
    nc_inq_vardimid( InputFile, timeVarID, dimids );
    nc_inq_dimlen( InputFile, dimids[0], &recs );
    days = (recs*curFile->baseDataRate) / 86400;
    curFile->FileEndTime[0] += days*24;
    curFile->FileEndTime[3] += days*86400;
    }

  std::sort(curFile->Variable.begin(), curFile->Variable.end(), VarCompareLT);
  OpenControlWindow(NULL, NULL, NULL);

}	/* END ADDDATAFILE */

/* -------------------------------------------------------------------- */
void SetList(Widget w, XtPointer client, XtPointer call)
{
  size_t	nVars;
  DATAFILE_INFO	*curFile = &dataFile[CurrentDataFile];
  XmString	item[curFile->Variable.size()];

  std::string varFilter;
  extern Widget varFilterText;

  XmListDeleteAllItems(varList);

  varFilter = XmTextFieldGetString(varFilterText);
  std::transform(varFilter.begin(), varFilter.end(), varFilter.begin(), (int(*)(int)) std::toupper);

  nVars = 0;
  for (size_t i = 0; i < curFile->Variable.size(); ++i)
  {
    std::string tv = curFile->Variable[i]->name;
    std::transform(tv.begin(), tv.end(), tv.begin(), (int(*)(int)) std::toupper);

    if (varFilter.length() == 0 || tv.find(varFilter, 0) != std::string::npos)
      item[nVars++] = XmStringCreateLocalized(const_cast<char *>(curFile->Variable[i]->name.c_str()));
  }

  XmListAddItems(varList, item, nVars, 1);

  for (size_t i = 0; i < nVars; ++i)
    XmStringFree(item[i]);

}	/* END SETLIST */

/* -------------------------------------------------------------------- */
static void readSet(DATASET_INFO *set)
{
  size_t	start[3], count[3];
  size_t	i, frontPad, endPad;
  int		rc;
  DATAFILE_INFO *file;

  WaitCursorAll();

  if (set->varInfo->inVarID == COMPUTED)
    {
    size_t whichExp = set->varInfo->name[4] - '1';;

    set->nPoints = NumberSeconds;

    for (i = 0; i < expSet.size(); ++i) 
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

  rc = nc_get_vara_float(file->ncid, set->varInfo->inVarID, start, count,
           &set->data[frontPad]);

  if (rc != NC_NOERR)
    {
    char msg[80];
    sprintf(msg, "dataIO.c::readSet: Failed to read data for variable %s.\n\n%s",
	set->varInfo->name.c_str(), nc_strerror(rc));
    HandleError(msg, Interactive, IRET);
    return;
    }

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

  for (i = 0; i < expSet.size(); ++i)
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
int LoadVariable(DATASET_INFO *set, std::string varName)
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
void AddVariable(DATASET_INFO *set, const char *var)
{
  int indx;

  set->fileIndex = CurrentDataFile;
  set->panelIndex = CurrentPanel;
  set->scaleLocation = whichSide();

  if ((indx = SearchTable(dataFile[CurrentDataFile].Variable, var)) == ERR)
    {
    fprintf(stderr, "AddVariable: Variable %s not found, should not happen!\n", var);
    return;
    }

  set->varInfo = dataFile[CurrentDataFile].Variable[indx];

  set->data = NULL;
  set->head = 0;

  set->stats.outlierMin = -FLT_MAX;
  set->stats.outlierMax = FLT_MAX;

  readSet(set);

}	/* END ADDVARIABLE */

/* -------------------------------------------------------------------- */
void ReduceData(int start, int newNumberSeconds)
{
  for (size_t set = 0; set < NumberDataSets; ++set)
    {
    size_t rate = dataSet[set].nPoints / NumberSeconds;
    dataSet[set].nPoints = newNumberSeconds * rate;

    memcpy((char *)dataSet[set].data, (char *)&dataSet[set].data[start*rate],
			dataSet[set].nPoints * sizeof(NR_TYPE));

    dataSet[set].data = (NR_TYPE *)realloc((void *)dataSet[set].data,
                dataSet[set].nPoints * sizeof(NR_TYPE));
    }

  for (size_t set = 0; set < expSet.size(); ++set)
    {
    if (expSet[set].nPoints == 0)
      continue;

    size_t rate = expSet[set].nPoints / NumberSeconds;
    expSet[set].nPoints = newNumberSeconds * rate;

    memcpy((char *)expSet[set].data, (char *)&expSet[set].data[start*rate],
			expSet[set].nPoints * sizeof(NR_TYPE));

    expSet[set].data = (NR_TYPE *)realloc((void *)expSet[set].data,
                expSet[set].nPoints * sizeof(NR_TYPE));
    }

  NumberSeconds = newNumberSeconds;


  /* Increment User Start Time accordingly.
   */
  int hours = start / 3600; start -= hours * 3600;
  int mins = start / 60; start -= mins * 60;
  int secs = start;

  if ((UserStartTime[2] += secs) > 59) {
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
  secs = newNumberSeconds;

  UserEndTime[0] = UserStartTime[0] + hours;
  UserEndTime[1] = UserStartTime[1] + mins;
  UserEndTime[2] = UserStartTime[2] + secs;

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

  for (size_t i = 0; i < NumberDataSets; ++i)
    ComputeStats(&dataSet[i]);

  findMinMax();

}	/* END REDUCEDATA */

/* -------------------------------------------------------------------- */
int DeleteVariable(DATASET_INFO *sets, size_t nSets, const char *var)
{
  int indx;
  bool rc = false;

  if ((indx = SearchTable(dataFile[CurrentDataFile].Variable, var)) == ERR)
    {
    fprintf(stderr, "DeleteVariable: Variable %s not found, should not happen!\n", var);
    return rc;
    }

  for (size_t set = 0; set < nSets; ++set)
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

  for (set = 0; set < expSet.size(); ++set)
    {
    expSet[set].nPoints = 0;
    delete [] expSet[set].data;
    }

  if (mode)
    expSet.clear();

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
  std::string tmpS;
  getNCattr(InputFile, NC_GLOBAL, "TimeInterval", tmpS);

  sscanf(tmpS.c_str(), "%02d:%02d:%02d-%02d:%02d:%02d",
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
  if ( curFile->FileStartTime[3] > curFile->FileEndTime[3] )
  {
    curFile->FileEndTime[0] += 24;
    curFile->FileEndTime[3] += 86400;
  }

  performSanityChecks(InputFile, curFile);

}	/* END GETTIMEINTERVAL */

/* -------------------------------------------------------------------- */
bool getNCattr(int ncid, int varID, const char attr[], std::string& dest)
{
  size_t len;
  bool rc = false;

  if (nc_inq_attlen(ncid, varID, attr, &len) == NC_NOERR)
  {
    if (nc_get_att_text(ncid, varID, attr, buffer) != NC_NOERR)
      return rc;

    buffer[len] = '\0';

    while (len > 0 && buffer[--len] < 0x20)	/* Remove extraneous CR/LF, etc */
      buffer[len] = '\0';

    dest = buffer;
    rc = true;
  }

  return rc;

}	/* END GETNCATTR */

/* -------------------------------------------------------------------- */
bool isMissingValue(float target, float fillValue)
{
  if (std::isnan(fillValue) && std::isnan(target))
    return(true);
  else
    if (fillValue == target)
      return(true);

  return(false);

}	/* END ISMISSINGVALUE */

/* -------------------------------------------------------------------- */
static int baseRate(const float tf[], int n)
{
  int i, si = 0;
  float fv[2];

  /* Given the first n values of Time from the netCDF file, find the first
   * two which are not missing values, subtract and divide by distance of
   * indices.
   */
  for (i = 0; i < n && isMissingValue(tf[i], DEFAULT_MISSING_VALUE); ++i)
    ;

  fv[0] = tf[i];
  si = i++;

  for (; i < n && isMissingValue(tf[i], DEFAULT_MISSING_VALUE); ++i)
    ;

  if (i < n)
    {
      fv[1] = tf[i];
      return (int)(fv[1] - fv[0]) / (i - si);
    }

  return 1;	// and hope for the best.
}

/* -------------------------------------------------------------------- */
static bool VarCompareLT(const VARTBL *x, const VARTBL *y)
{
    return(x->name < y->name);
}

/* END DATAIO.C */
