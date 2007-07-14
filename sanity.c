/*
-------------------------------------------------------------------------
OBJECT NAME:	sanity.c

FULL NAME:	SAnity Checks for netCDF file.

DESCRIPTION:	

NOTES:		

COPYRIGHT:	University Corporation for Atmospheric Research, 2006
-------------------------------------------------------------------------
*/

#include "define.h"
#include <netcdf.h>

bool getNCattr(int ncid, int varID, char attr[], std::string & dest);


/* -------------------------------------------------------------------- */
static void checkNumberRecords(int InputFile, DATAFILE_INFO * curFile)
{
  int id;

  if (nc_inq_dimid(InputFile, "Time", &id) == NC_NOERR)
  {
    size_t length;
    size_t deltaT = (size_t)(curFile->FileEndTime[3] - curFile->FileStartTime[3]) + 1;
    nc_inq_dimlen(InputFile, id, &length);
    if (length != deltaT)
    {
      fprintf(stderr, "dataIO.c::GetTimeInterval(): Sanity check failure.");
      fprintf(stderr, " %d records vs. %d computed.\n", length, deltaT);
      nc_close(InputFile);
      exit(1);
    }
  }
}

/* -------------------------------------------------------------------- */
void checkStarts(int fd)
{
  int           id;
  size_t	edge[3];
  time_t        oldBaseTime, BaseTime;
  struct tm     StartFlight;
  bool		fileGood = true;
  std::string	tmpS;

  edge[0] = edge[1] = edge[2] = 0;

  // We can't do checks without FlightDate.  Some asc2cdf files don't
  // have it.
  if (getNCattr(fd, NC_GLOBAL, "FlightDate", tmpS) == false)
    return;

  sscanf(tmpS.c_str(), "%2d/%2d/%4d",	&StartFlight.tm_mon,
				&StartFlight.tm_mday,
				&StartFlight.tm_year);

  if (getNCattr(fd, NC_GLOBAL, "TimeInterval", tmpS) == false)
    return;

  sscanf(tmpS.c_str(), "%02d:%02d:%02d",
				&StartFlight.tm_hour,
				&StartFlight.tm_min,
				&StartFlight.tm_sec);

  StartFlight.tm_mon--;
  StartFlight.tm_year -= 1900;
  BaseTime = timegm(&StartFlight);

  if (nc_inq_varid(fd, "base_time", &id) == NC_NOERR &&
      nc_get_var1_int(fd, id, edge, (int*)&oldBaseTime) == NC_NOERR &&
      oldBaseTime != BaseTime)
  {
    fileGood = false;
    printf("\nSanity check failure: File has incorrect base_time\n");
    printf("         is (%u) %s",
                (unsigned)oldBaseTime, asctime(gmtime(&oldBaseTime)));
    printf("  should be (%u) %s",
                (unsigned)BaseTime, asctime(gmtime(&BaseTime)));
  }

  if (nc_inq_varid(fd, "Time", &id) == NC_NOERR)
  {
    char att[128], oldAtt[128];
    size_t len;

    nc_get_att_text(fd, id, "units", oldAtt);
    nc_inq_attlen(fd, id, "units", &len);
    oldAtt[len] = '\0';

    strftime(att, 128, "seconds since %F %T %z", &StartFlight);

    if (strcmp(att, oldAtt) != 0)
    {
      fileGood = false;
      printf("\nSanity check failure: File has incorrect Time:units\n");
      printf("         is: %s\n", oldAtt);
      printf("  should be: %s\n", att);
    }
  }

  if (fileGood == false)
  {
    nc_close(fd);
    exit(1);
  }
}

/* -------------------------------------------------------------------- */
void performSanityChecks(int InputFile, DATAFILE_INFO * curFile)
{
  std::string tmpS;

  if (getNCattr(InputFile, NC_GLOBAL, "TimeInterval", tmpS) == false)
    return;

  if (strcmp(tmpS.c_str(), "00:00:00-00:00:00") == 0)
  {
    fprintf(stderr, "\nSanity check failure: TimeInterval of 00:00:00-00:00:00, no data in this file?\n");
    nc_close(InputFile);
    exit(1);
  }
return;
  // Check that # records in file matches deltaT of GLOBAL_ATTTR:TimeInterval
  checkNumberRecords(InputFile, curFile);

  // Check that base_time & Time:units have correct start, there are a # of
  // files where they had a 1 second offset from FlightDate/TimeInterval
  // pair, which were correct.
  checkStarts(InputFile);
}
