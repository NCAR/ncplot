/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:	Initialize Procedures

ENTRY POINTS:	Initialize()
		processArgs()

STATIC FNS:	initPlot()

DESCRIPTION:	Inializiation routines run once at the beginning of main
		Initialize is used to set all global variables.
		Process_args does just that.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-9
-------------------------------------------------------------------------
*/

#include <pthread.h>

#include "define.h"
#include "ps.h"
#include "spec.h"

static void initPlot(PLOT_INFO *);

char *insVariables[3], *gpsVariables[3], *gpsCorrected[3], *windVariables[3];


/* --------------------------------------------------------------------- */
void Initialize()
{
  int		i;
  char		*p;
  pthread_t	tid;

  char *insVar[] = { "LON", "LAT", "PALT" };
  char *gpsVar[] = { "GGLON", "GGLAT", "GGALT" };
  char *gpsCor[] = { "LONC", "LATC", "ALTX" };
  char *windVar[] = { "UIC", "VIC", "300" };


  pthread_create(&tid, NULL, GetPrinterList, NULL);

  parmsFile = outFile = timeSeg = NULL;

  Interactive		= True;
  DataChanged		= True;
  AsciiWinOpen		= False;
  Color			= False;
  Freeze		= False;
  UTCseconds		= False;
  LandMarks		= False;
  ScatterPlot		= False;
  WindBarbs		= False;
  Statistics		= True;
  allLabels		= True;
  ShowRegression	= 0;

  CurrentDataFile	= 0;
  CurrentDataSet	= 0;
  CurrentPanel		= 0;
  NumberDataFiles	= 0;
  NumberDataSets	= 0;
  NumberXYYsets		= 0;
  NumberOfPanels	= 1;
  NumberOfXYpanels	= 1;
  LineThickness		= 1;

  ProjectToXY = ProjectToBack = False;

  nDirectionArrows = 0;
  nTimeStamps = 0;

  for (i = 0; i < MAX_DATASETS; ++i)
    dataSet[i].varInfo = NULL;

  for (i = 0; i < MAX_DATASETS; ++i)
    {
    xyXset[i].varInfo = NULL;
    xyYset[i].varInfo = NULL;
    }

  for (i = 0; i < 3; ++i)
    xyzSet[i].varInfo = NULL;

  strcpy(tasVarName, "TASX");
  tas.data = NULL;

  for (i = 0; i < MAX_DATAFILES; ++i)
    dataFile[i].ncid = -1;	/* Inactivate all data file arrays.	*/

  for (i = 0; i < MAX_PANELS; ++i)
    {
    initPlot(&mainPlot[i]);
    mainPlot[i].windowOpen = True;

    initPlot(&xyyPlot[i]);
    xyyPlot[i].plotType		= XY_PLOT;
    xyyPlot[i].x.windowWidth	= 625;
    xyyPlot[i].x.windowHeight	= 800;
    strcpy(xyyPlot[i].Xaxis.label, "");
    xyyPlot[i].Yaxis[0].nMajorTics = xyyPlot[i].Yaxis[1].nMajorTics =
                                xyyPlot[i].Xaxis.nMajorTics;
    }


  initPlot(&xyzPlot);
  initPlot(&specPlot);
  initPlot(&diffPlot);

  diffPlot.Yaxis[0].nMajorTics  = 4;


  xyzPlot.plotType		= XYZ_PLOT;
  xyzPlot.x.windowWidth		= 800;
  xyzPlot.x.windowHeight	= 625;
  xyzPlot.Xaxis.nMinorTics	= 0;
  xyzPlot.Yaxis[0].nMinorTics	= 0;
  strcpy(xyzPlot.Xaxis.label, "");


  specPlot.plotType		= XY_PLOT;
  specPlot.Xaxis.logScale	= True;
  specPlot.Xaxis.nMinorTics	= 10;
  specPlot.Yaxis[0].nMinorTics	= 10;
  specPlot.Xaxis.min		= 0.001;
  specPlot.Xaxis.max		= 1000;
  strcpy(specPlot.Xaxis.label, "Frequency (Hz)");

  for (i = 0; i < MAX_PSD; ++i)
    {
    psd[i].M = 512;
    psd[i].Pxx = psd[i].Qxx = psd[i].Special = NULL;
    psd[i].detrendFn = DetrendLinear;
    psd[i].windowFn  = Parzen;
    psd[i].ELIAcnt = 0;
    psd[i].ELIAx = psd[i].ELIAy = NULL;
    }


  if ((p = (char *)getenv("DATA_DIR")) != NULL)
    {
    strcpy(DataPath, p);
    strcat(DataPath, "/*.nc");
    }
  else
    strcpy(DataPath, "/*");

#ifdef SVR4
  strcpy(printerSetup.lpCommand, "lp -o nobanner");
#else
  strcpy(printerSetup.lpCommand, "lpr -h");
#endif
  printerSetup.width = 8.5;
  printerSetup.height = 11.0;
  printerSetup.shape = LANDSCAPE;
  printerSetup.dpi = 288;
  printerSetup.color = False;

  if (RealTime)
    {
    strcpy(asciiFormat, "%14.4f");
    nASCIIpoints = 30;
    }
  else
    {
    strcpy(asciiFormat, "%14e");
    nASCIIpoints = 1000;
    }


  for (i = 0; i < 3; ++i)
    {
    insVariables[i] = (char *)malloc(NAMELEN);
    gpsVariables[i] = (char *)malloc(NAMELEN);
    gpsCorrected[i] = (char *)malloc(NAMELEN);
    windVariables[i] = (char *)malloc(NAMELEN);

    strcpy(insVariables[i], insVar[i]);
    strcpy(gpsVariables[i], gpsVar[i]);
    strcpy(gpsCorrected[i], gpsCor[i]);
    strcpy(windVariables[i], windVar[i]);
    }

}	/* END INITIALIZE */

/* --------------------------------------------------------------------- */
static void initPlot(PLOT_INFO *plot)
{
  memset((char *)plot, 0, sizeof(PLOT_INFO));

  plot->windowOpen	= False;
  plot->grid		= False;
  plot->autoScale	= True;
  plot->autoTics	= True;
  plot->plotType	= TIME_SERIES;

  strcpy(plot->Xaxis.label, "UTC");

  plot->Xaxis.nMajorTics	= 5;
  plot->Xaxis.nMinorTics	= 2;
  plot->Yaxis[0].nMajorTics	= 8;
  plot->Yaxis[1].nMajorTics	= 8;
  plot->Yaxis[0].nMinorTics	= 2;
  plot->Yaxis[1].nMinorTics	= 2;
  plot->Zaxis.nMajorTics	= 5;
  plot->Zaxis.nMinorTics	= 0;

  plot->Xaxis.min = plot->Yaxis[0].min = plot->Yaxis[1].min =
                    plot->Zaxis.min = FLT_MAX;
  plot->Xaxis.max = plot->Yaxis[0].max = plot->Yaxis[1].max =
                    plot->Zaxis.max = -FLT_MAX;

}	/* END INITPLOT */

/* --------------------------------------------------------------------- */
void ProcessArgs(char **argv)
{
  while (*++argv)
    if ((*argv)[0] == '-')
      switch ((*argv)[1])
        {
/*        case 'p':
          if (*++argv == NULL)
            return;

          parmsFile = GetMemory((unsigned)strlen(*argv)+1);

          strcpy(parmsFile, *argv);
          break;

        case 'f':
          if (*++argv == NULL)
            return;

          outFile = GetMemory((unsigned)strlen(*argv)+1);

          strcpy(outFile, *argv);
          break;

        case 'h':
          Interactive = False;
          break;
*/
        case 's':	/* setup file	*/
          if (*++argv == NULL)
            return;

          SetTemplateFile(*argv);
          break;

        case 't':	/* Time Segment	*/
          if (*++argv == NULL)
            return;

          timeSeg = (char *)GetMemory((unsigned)strlen(*argv)+1);

          strcpy(timeSeg, *argv);
          break;

        case 'r':	/* Realtime		*/
          RealTime = True;
          strcpy(dataFile[0].fileName, "/home/tmp/RealTime.nc");
          NumberDataFiles = 1;
          break;
        }
    else
      {
      strcpy(dataFile[0].fileName, *argv);
      NumberDataFiles = 1;
      }

}	/* END PROCESSARGS */

/* END INIT.C */
