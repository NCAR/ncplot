/*
-------------------------------------------------------------------------
OBJECT NAME:	plotX.c

FULL NAME:	Plot Graph in X window

ENTRY POINTS:	DrawTimeSeries()
		ResizeTimeSeries()

STATIC FNS:	plotData()

DESCRIPTION:	This is the Expose event procedure to regenerate the
		main image.

INPUT:		none

OUTPUT:		none

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2005
-------------------------------------------------------------------------
*/

#include "define.h"
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

static size_t	currentPanel;
static int	fontOffset;

void MakeLegendLabel(char *, DATASET_INFO *);
static void plotData(PLOT_INFO *plot);


/* -------------------------------------------------------------------- */
void ResizeTimeSeries()
{
  size_t i;
  int	n, totalVD, depth;
  Arg	args[5];

  n = 0;
  XtSetArg(args[n], XmNwidth, &mainPlot[0].x.windowWidth); ++n;
  XtSetArg(args[n], XmNheight, &mainPlot[0].x.windowHeight); ++n;
  XtSetArg(args[n], XtNdepth, &depth); ++n;
  XtGetValues(mainPlot[0].canvas, args, n);

  NewPixmap(&mainPlot[0], mainPlot[0].x.windowWidth, mainPlot[0].x.windowHeight, depth);

  mainPlot[0].x.titleOffset	= 45;
  mainPlot[0].x.subTitleOffset	= 65;

  totalVD = mainPlot[0].x.windowHeight - 100;

  for (i = 0; i < NumberOfPanels; ++i)
    {
    if (i > 0)
      {
      mainPlot[i].x.windowWidth = mainPlot[0].x.windowWidth;
      mainPlot[i].x.windowHeight = mainPlot[0].x.windowHeight;
      }

    mainPlot[i].x.LV = (int)(mainPlot[0].x.windowWidth * 0.1364);
    mainPlot[i].x.HD = (int)(mainPlot[0].x.windowWidth * 0.7273);

    if (NumberOfPanels == 1)
      {
      mainPlot[i].x.TH = (int)(mainPlot[0].x.windowHeight * 0.2089);
      mainPlot[i].x.VD = (int)(mainPlot[0].x.windowHeight * 0.5822); /* 5" */
      }
    else
      {
      mainPlot[i].x.TH = (int)(100 + totalVD / NumberOfPanels * i);
      mainPlot[i].x.VD = (int)(totalVD / NumberOfPanels - 40);
      }
 
    mainPlot[i].x.RV = mainPlot[i].x.LV + mainPlot[i].x.HD;
    mainPlot[i].x.BH = mainPlot[i].x.TH + mainPlot[i].x.VD;

    if (NumberOfPanels == 1)
      mainPlot[i].x.xLabelOffset = mainPlot[i].x.BH + 40;
    else
      mainPlot[i].x.xLabelOffset = mainPlot[i].x.BH + 20;

    mainPlot[i].x.ticLength		= mainPlot[i].x.HD > 400 ? 10 : 5;
    mainPlot[i].x.yLabelOffset		= 7;
    mainPlot[i].x.yTicLabelOffset	= 5;
    mainPlot[i].x.xTicLabelOffset	= 20;

    mainPlot[i].x.xLegendText	= mainPlot[i].x.LV;
    }

  fontOffset = (totalVD < 500) ? 1 : 0;

}	/* END RESIZEPLOTWINDOW */

/* -------------------------------------------------------------------- */
void DrawTimeSeries()
{
  size_t	i;
  static bool	firstTime = true;
  XFontStruct	*fontInfo;

  /* Set default Graphics Context stuff and get the GC
   */
  if (firstTime)
    {
    initPlotGC(&mainPlot[0]);

    for (i = 1; i < MAX_PANELS; ++i)
      {
      copyGC(&mainPlot[i], &mainPlot[0]);
      copyGC(&xyyPlot[i], &mainPlot[0]);
      }

    copyGC(&xyyPlot[0], &mainPlot[0]);
    copyGC(&xyzPlot, &mainPlot[0]);

    ResizeTimeSeries();
    InitializeColors(&mainPlot[0]);
    firstTime = false;
    }


  /* Set clipping to whole window */
  XSetClipMask(mainPlot[0].dpy, mainPlot[0].gc, None);

  ClearPixmap(&mainPlot[0]);

  bool warning = false;
  for (i = 0; i < NumberDataSets; ++i)
    if (dataFile[dataSet[i].fileIndex].ShowPrelimDataWarning)
      warning = true;

  plotTitlesX(&mainPlot[0], fontOffset, warning);

  fontInfo = mainPlot[0].fontInfo[3+fontOffset];
  XSetFont(mainPlot[0].dpy, mainPlot[0].gc, fontInfo->fid);

  for (currentPanel = 0; currentPanel < NumberOfPanels; ++currentPanel)
    {
    XDrawRectangle(mainPlot[currentPanel].dpy, mainPlot[currentPanel].win,
          mainPlot[currentPanel].gc,
          mainPlot[currentPanel].x.LV, mainPlot[currentPanel].x.TH,
          mainPlot[currentPanel].x.HD, mainPlot[currentPanel].x.VD);

    if (allLabels || currentPanel == NumberOfPanels-1)
      xTicsLabelsX(&mainPlot[currentPanel], fontInfo, true);
    else
      xTicsLabelsX(&mainPlot[currentPanel], fontInfo, false);

    if (NumberDataSets > 0)
      {
      plotLabelsX(&mainPlot[currentPanel], fontOffset);

      yTicsLabelsX(&mainPlot[currentPanel], fontInfo, LEFT_SIDE, true);

      for (i = 0; i < NumberDataSets; ++i)
        if (dataSet[i].scaleLocation == RIGHT_SIDE &&
            dataSet[i].panelIndex == currentPanel)
          yTicsLabelsX(&mainPlot[currentPanel], fontInfo, RIGHT_SIDE, true);

      plotData(&mainPlot[currentPanel]);
      }
    }

  UpdateAnnotationsX(&mainPlot[0]);

}	/* END DRAWTIMESERIES */

/* -------------------------------------------------------------------- */
static void plotData(PLOT_INFO *plot)
{
  DATASET_INFO *set;
  VARTBL	*vp;
  int		cnt, cntRt, cntLft, ylegend;
  char		dashList[4];

  ResetColors();

  cnt = cntRt = cntLft = 0;
  for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
    {
    if (dataSet[CurrentDataSet].panelIndex != currentPanel)
      continue;

    set = &dataSet[CurrentDataSet];

    if (Color)
      XSetForeground(plot->dpy, plot->gc, NextColor());

    if (Color || cnt == 0)
      XSetLineAttributes(plot->dpy, plot->gc, LineThickness, LineSolid, CapButt, JoinMiter);
    else
      XSetLineAttributes(plot->dpy, plot->gc, LineThickness, LineOnOffDash, CapButt, JoinMiter);

    plotTimeSeries(plot, set);


    /* Display legend for each dataset
     */
    vp = set->varInfo;

    if (Color)
      XSetLineAttributes(plot->dpy,plot->gc, 3,LineSolid,CapButt,JoinMiter);

    if (Statistics)
      {
      int	len;

      ylegend = yLegendX(&mainPlot[0], cnt);
      MakeLegendLabel(buffer, set);

      len = XTextWidth(plot->fontInfo[3+fontOffset], buffer, strlen(buffer));

      if (set->scaleLocation == LEFT_SIDE)
        XDrawLine(plot->dpy, plot->win, plot->gc, plot->x.xLegendText-45,
                ylegend-2, plot->x.xLegendText-5, ylegend-2);
      else
        XDrawLine(plot->dpy, plot->win, plot->gc, plot->x.xLegendText+len+5,
                ylegend-2, plot->x.xLegendText+len+45, ylegend-2);

      if (Color)
        XSetForeground(plot->dpy, plot->gc, GetColor(0));

      XDrawString(plot->dpy, plot->win, plot->gc, plot->x.xLegendText,
                  ylegend+3, buffer, strlen(buffer));
      }
    else
      {
      int	pos;

      ylegend = plot->x.TH - 5;

      if (set->scaleLocation == LEFT_SIDE)
        pos = plot->x.LV + (cntLft++ * 100);
      else
        pos = plot->x.RV - (++cntRt * 100);

      XDrawLine(plot->dpy, plot->win, plot->gc, pos, ylegend-2, pos + 35, ylegend-2);

      if (Color)
        XSetForeground(plot->dpy, plot->gc, GetColor(0));

      XDrawString(plot->dpy, plot->win, plot->gc, pos + 40,
                  ylegend+3, vp->name.c_str(), vp->name.size());
      }

    if (!Color)
      {
      /* Change Dash style for the next dataset   */
      dashList[0] = (cnt + 1) * 6;
      dashList[1] = (cnt + 1) * 3;
      XSetDashes(plot->dpy, plot->gc, 0, dashList, 2);
      }

    ++cnt;
    }

  if (Color)
    XSetForeground(plot->dpy, plot->gc, GetColor(0));

  XSetLineAttributes(plot->dpy, plot->gc, 1, LineSolid, CapButt, JoinMiter);

  if (Statistics)
    XDrawString(plot->dpy, plot->win, plot->gc, plot->x.xLegendText,
	yLegendX(plot, (cnt+1)) + 12, statsTitle, strlen(statsTitle));

}	/* END PLOTDATA */

/* END PLOTX.C */
