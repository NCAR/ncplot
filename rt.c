/*
-------------------------------------------------------------------------
OBJECT NAME:	rt.c

FULL NAME:	RealTime

ENTRY POINTS:	UpdateData()

STATIC FNS:		

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#define NO_NETCDF_2

#include "define.h"
#include <netcdf.h>


static int	InputFile;
static size_t	lastRecordNumber[3] = {0,0,0};
static size_t	ncount[3];

static void	updateSet(DATASET_INFO *);

extern XtAppContext appContext;
void	findMinMax();

/* -------------------------------------------------------------------- */
void UpdateDataRT(XtPointer client, XtIntervalId *id)
{
  int  		recDim;
  size_t	i, nRecords;

  if (Freeze)
    goto reset;

  InputFile = dataFile[0].ncid;

  nc_sync(InputFile);
  nc_inq_unlimdim(InputFile, &recDim);
  nc_inq_dimlen(InputFile, recDim, &nRecords);

  if (lastRecordNumber[0] == nRecords)
    goto reset;

  GetTimeInterval(InputFile, &dataFile[0]);
  memcpy((char *)UserEndTime, (char *)dataFile[0].FileEndTime, sizeof(int)*4);

  UserStartTime[3] = UserEndTime[3] - NumberSeconds;
  FromSecondsSinceMidnite(UserStartTime);


  ncount[0] = nRecords - lastRecordNumber[0]; ncount[2] = 1;

  /* Update Time Series variables.  */
  for (i = 0; i < NumberDataSets; ++i)
    updateSet(&dataSet[i]);

  /* Update XYY variables.  */
  for (i = 0; i < NumberXYXsets; ++i)
    if (xyXset[i].varInfo)
      updateSet(&xyXset[i]);

  for (i = 0; i < NumberXYYsets; ++i)
    if (xyYset[i].varInfo)
      updateSet(&xyYset[i]);

  /* Update XYZ variables.  */
  for (i = 0; i < 3; ++i)
    if (xyzSet[i].varInfo)
      updateSet(&xyzSet[i]);

  if (WindBarbs)
    {
    updateSet(&ui);
    updateSet(&vi);
    }


  lastRecordNumber[0] = nRecords;

  findMinMax();
  SetSubtitles();
  SetTimeText();

  DataChanged = true;
  DrawMainWindow();

reset:
  XtAppAddTimeOut(appContext, 500, UpdateDataRT, NULL);

}	/* END UPDATEDATART */

/* -------------------------------------------------------------------- */
static void updateSet(DATASET_INFO *set)
{
  ncount[1] = set->varInfo->OutputRate;
 
  if ((set->head + (ncount[0] * ncount[1])) > NumberSeconds * ncount[1])
    {
    int	nextCount = 0;

    while ((set->head + (ncount[0] * ncount[1])) > NumberSeconds * ncount[1])
      {
      --ncount[0];
      ++nextCount;
      }

    nc_get_vara_float(InputFile, set->varInfo->inVarID, lastRecordNumber, ncount,
             &set->data[set->head]);

    ncount[0] = nextCount;
    set->head = 0;
    nc_get_vara_float(InputFile, set->varInfo->inVarID, lastRecordNumber, ncount,
             &set->data[set->head]);
    }
  else
    {
    nc_get_vara_float(InputFile, set->varInfo->inVarID, lastRecordNumber, ncount,
             &set->data[set->head]);
    }
 
  if ((set->head += ncount[0] * ncount[1]) == NumberSeconds * ncount[1])
    set->head = 0;
 
  if (set->head > NumberSeconds * ncount[1])
    {
    printf("circBuff index superceeded max count, you shouldn't see this message = %d.\n", set->head);
    set->head = 0;
    }

  ComputeStats(set);

}	/* END UPDATESET */

/* END RT.C */
