/*
-------------------------------------------------------------------------
OBJECT NAME:	xyX.c

FULL NAME:	Plot XY in X window

ENTRY POINTS:	DrawXY()
		ResizeXY()

STATIC FNS:	plotXYdata()

DESCRIPTION:	This is the Expose event procedure to regenerate the
		whole XY image.

REFERENCES:	X.c

REFERENCED:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-8
-------------------------------------------------------------------------
*/

#include "define.h"

#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

void MakeLegendLabel(char *, DATASET_INFO *);

static int	currentPanel, fontOffset;

static void	plotXYdata(PLOT_INFO *plot);
static void	plotRegression(PLOT_INFO *plot, DATASET_INFO *x, DATASET_INFO *y);
static void	doLegendLineItem(PLOT_INFO *plot, DATASET_INFO *set,
				int ylegend, int color);



/* -------------------------------------------------------------------- */
void ResizeXY()
{
  int	n, depth, i, totalHD;
  Arg	args[5];

  n = 0;
  XtSetArg(args[n], XmNwidth, &xyyPlot[0].x.windowWidth); ++n;
  XtSetArg(args[n], XmNheight, &xyyPlot[0].x.windowHeight); ++n;
  XtSetArg(args[n], XtNdepth, &depth); ++n;
  XtGetValues(xyyPlot[0].canvas, args, n);

  NewPixmap(&mainPlot[0], xyyPlot[0].x.windowWidth, xyyPlot[0].x.windowHeight, depth);

  if (NumberOfXYpanels == 1)
    {
    xyyPlot[0].x.titleOffset    = (int)(xyyPlot[0].x.windowHeight * 0.18);
    xyyPlot[0].x.subTitleOffset = (int)(xyyPlot[0].x.windowHeight * 0.22);
    }
  else
    {
    xyyPlot[0].x.titleOffset     = 45;
    xyyPlot[0].x.subTitleOffset  = 65;
    }

  totalHD = xyyPlot[0].x.windowWidth - 20;
 
  for (i = 0; i < NumberOfXYpanels; ++i)
    {
    if (i > 0)
      {
      xyyPlot[i].x.windowWidth = xyyPlot[0].x.windowWidth;
      xyyPlot[i].x.windowHeight = xyyPlot[0].x.windowHeight;
      }
 
    if (NumberOfXYpanels == 1)
      {
      xyyPlot[i].x.LV = (int)(xyyPlot[0].x.windowWidth * 0.2);
      xyyPlot[i].x.HD = (int)(xyyPlot[0].x.windowWidth * 0.6);

      xyyPlot[i].x.TH = (int)(xyyPlot[0].x.windowHeight * 0.29);
      xyyPlot[i].x.VD = (int)(xyyPlot[0].x.windowHeight * 0.4735);

      xyyPlot[i].x.xLegendText	= 65;
      xyyPlot[i].x.yLabelOffset	= xyyPlot[i].x.LV >> 2;
      }
    else
      {
      xyyPlot[i].x.LV = (int)(totalHD / NumberOfXYpanels * i + 60);
      xyyPlot[i].x.HD = (int)(totalHD / NumberOfXYpanels - 60);

      xyyPlot[i].x.TH = (int)(xyyPlot[0].x.windowHeight * 0.2);
      xyyPlot[i].x.VD = (int)(xyyPlot[0].x.windowHeight * 0.6);

      xyyPlot[i].x.xLegendText	= xyyPlot[i].x.LV + (xyyPlot[i].x.HD >> 1) - 50;
      xyyPlot[i].x.yLabelOffset	= xyyPlot[i].x.LV - 55;
      }

 
    xyyPlot[i].x.RV = xyyPlot[i].x.LV + xyyPlot[i].x.HD;
    xyyPlot[i].x.BH = xyyPlot[i].x.TH + xyyPlot[i].x.VD;

    xyyPlot[i].x.xLabelOffset = xyyPlot[i].x.BH + 40;

    xyyPlot[i].x.ticLength = xyyPlot[i].x.HD > 250 ? 10 : 5;
    xyyPlot[i].x.yTicLabelOffset = 5;
    xyyPlot[i].x.xTicLabelOffset = 20;
    }

}	/* END RESIZEXY */

/* -------------------------------------------------------------------- */
void DrawXY()
{
  int		i;
  bool		ySide = False, label;
  XFontStruct	*fontInfo;

  static bool	firstTime = True;

  /* Set default Graphics Context stuff and get the GC
  */
  if (firstTime)
    {
    ResizeXY();
    firstTime = False;
    }

  XSetClipMask(xyyPlot[0].dpy, xyyPlot[0].gc, None);

  ClearPixmap(&xyyPlot[0]);

  fontOffset =
    (xyyPlot[0].x.windowWidth < 500 || xyyPlot[0].x.windowHeight < 500) ? 1 : 0;

  plotTitlesX(&xyyPlot[0], fontOffset);

  if (fontOffset == 0 && NumberOfXYpanels > 1)
    fontOffset = 1;

  for (currentPanel = 0; currentPanel < NumberOfXYpanels; ++currentPanel)
    {
    XSetLineAttributes(xyyPlot[0].dpy, xyyPlot[0].gc, 1,
					LineSolid, CapButt, JoinMiter);
    XDrawRectangle(xyyPlot[0].dpy, xyyPlot[0].win, xyyPlot[0].gc,
                xyyPlot[currentPanel].x.LV, xyyPlot[currentPanel].x.TH,
		xyyPlot[currentPanel].x.HD, xyyPlot[currentPanel].x.VD);

    for (i = 0; i < NumberXYYsets; ++i)
      if (xyYset[i].scaleLocation == RIGHT_SIDE)
        ySide = True;

    if (!ySide)
      xyyPlot[currentPanel].Yaxis[1].label[0] = '\0';

    plotLabelsX(&xyyPlot[currentPanel], fontOffset);

    fontInfo = xyyPlot[currentPanel].fontInfo[3+fontOffset];
    XSetFont(xyyPlot[0].dpy, xyyPlot[0].gc, fontInfo->fid);

    xTicsLabelsX(&xyyPlot[currentPanel], fontInfo, True);
    if (allLabels || currentPanel == 0)
      label = True;
    else
      label = False;

    yTicsLabelsX(&xyyPlot[currentPanel], fontInfo, LEFT_SIDE, label);

    if (ySide)
      yTicsLabelsX(&xyyPlot[currentPanel], fontInfo, RIGHT_SIDE, label);

    if (NumberXYXsets == 0 || NumberXYYsets == 0)
      continue;

    plotXYdata(&xyyPlot[currentPanel]);

    XSetLineAttributes(xyyPlot[0].dpy, xyyPlot[0].gc, 1,
					LineSolid, CapButt, JoinMiter);
    if (WindBarbs)
      PlotWindBarbs(&xyyPlot[currentPanel], NULL);

    DrawGeoPolMapXY(&xyyPlot[currentPanel], NULL);
    PlotLandMarksXY(&xyyPlot[currentPanel], NULL);
    }

  UpdateAnnotationsX(&xyyPlot[0]);

}	/* END DRAWXY */

/* -------------------------------------------------------------------- */
static void plotXYdata(PLOT_INFO *plot)
{
  int		i, ylegend, xSet, ySet, n, plotNum;
  bool		xChanged, yChanged;
  char		dashList[4];

  ResetColors();

  xSet = ySet = -1;

  n = MAX(NumberXYXsets, NumberXYYsets);
  for (plotNum = 0, CurrentDataSet = 0; plotNum < n; ++plotNum)
    {
    xChanged = yChanged = False;

    for (i = xSet+1; i < NumberXYXsets; ++i)
      if (xyXset[i].panelIndex == currentPanel)
        {
        xChanged = True;
        xSet = i;
        break;
        }

    for (i = ySet+1; i < NumberXYYsets; ++i)
      if (xyYset[i].panelIndex == currentPanel)
        {
        yChanged = True;
        ySet = i;
        break;
        }

    if (xSet == -1 || ySet == -1 || (!xChanged && !yChanged))
      break;

    if (Color)
      XSetForeground(plot->dpy, plot->gc, GetColor(plotNum+1));

    if (Color || plotNum == 0)
      XSetLineAttributes(plot->dpy,plot->gc, LineThickness,LineSolid,CapButt,JoinMiter);
    else
      XSetLineAttributes(plot->dpy,plot->gc, LineThickness,LineOnOffDash,CapButt,JoinMiter);

    plotXY(plot, &xyXset[xSet], &xyYset[ySet], plotNum+1);

    if (ShowRegression)
      plotRegression(plot, &xyXset[xSet], &xyYset[ySet]);


    /* Display legend for each dataset
     */
    ylegend = yLegendX(plot, CurrentDataSet++);
    doLegendLineItem(plot, &xyXset[xSet], ylegend, -1);

    ylegend = yLegendX(plot, CurrentDataSet++);
    doLegendLineItem(plot, &xyYset[ySet], ylegend, plotNum+1);

    if (!Color && CurrentDataSet > 1)
      {
      /* Change Dash style for the next dataset   */
      dashList[0] = (CurrentDataSet + 1) * 6;
      dashList[1] = (CurrentDataSet + 1) * 3;
      XSetDashes(plot->dpy, plot->gc, 0, dashList, 2);
      }
    }


  XSetForeground(plot->dpy, plot->gc, GetColor(0));
  XSetLineAttributes(plot->dpy, plot->gc, LineThickness, LineSolid, CapButt, JoinMiter);

  if (Statistics)
    XDrawString(plot->dpy, plot->win, plot->gc, plot->x.xLegendText,
	yLegendX(plot,(CurrentDataSet+1))+12, statsTitle, strlen(statsTitle));

}	/* END PLOTXYDATA */

/* -------------------------------------------------------------------- */
static void plotRegression(PLOT_INFO *plot, DATASET_INFO *x, DATASET_INFO *y)
{
  int	i, j, y1, y2;
  float	xMin = plot->Xaxis.min;
  float	xMax = plot->Xaxis.max;
  float	yMin = plot->Yaxis[0].min;
  float	yScale = (float)plot->x.VD / (plot->Yaxis[0].max - yMin);
  float	yInterMin, yInterMax;

  void fitcurve(DATASET_INFO *x, DATASET_INFO *y, int ideg);

  printf("X axis variable: %s from %s\n",
		x->varInfo->name, dataFile[x->fileIndex].fileName);
  printf("Y axis variable: %s from %s\n",
		y->varInfo->name, dataFile[y->fileIndex].fileName);

  fitcurve(x, y, ShowRegression);

  setClippingX(plot);
  XSetForeground(plot->dpy, plot->gc, GetColor(0));

  
  if (ShowRegression == 1)	/* Linear */
    {
    yInterMin = yScale * (regretCo[0] + (regretCo[1] * xMin) - yMin);
    yInterMax = yScale * (regretCo[0] + (regretCo[1] * xMax) - yMin);

    if (plot->Yaxis[0].invertAxis)
      {
      y1 = plot->x.TH + (int)yInterMin;
      y2 = plot->x.TH + (int)yInterMax;
      }
    else
      {
      y1 = plot->x.BH - (int)yInterMin;
      y2 = plot->x.BH - (int)yInterMax;
      }

    XDrawLine(plot->dpy,plot->win,plot->gc, plot->x.LV, y1, plot->x.RV, y2);
    }
  else
    {
    float	inc = (xMax - xMin) / plot->x.HD * 4;

    xMax = xMin + inc;

    for (i = plot->x.LV; i < plot->x.RV; i += 4)
      {
      yInterMin = regretCo[0] + (regretCo[1] * xMin);
      yInterMax = regretCo[0] + (regretCo[1] * xMax);

      for (j = 2; j <= ShowRegression; ++j)
        {
        yInterMin += regretCo[j] * pow(xMin, (double)j);
        yInterMax += regretCo[j] * pow(xMax, (double)j);
        }

      yInterMin = yScale * (yInterMin - yMin);
      yInterMax = yScale * (yInterMax - yMin);

      if (plot->Yaxis[0].invertAxis)
        {
        y1 = plot->x.TH + (int)yInterMin;
        y2 = plot->x.TH + (int)yInterMax;
        }
      else
        {
        y1 = plot->x.BH - (int)yInterMin;
        y2 = plot->x.BH - (int)yInterMax;
        }

      XDrawLine(plot->dpy, plot->win, plot->gc, i, y1, i + 4, y2);

      xMin = xMax; xMax += inc;
      }
    }

  XSetClipMask(plot->dpy, plot->gc, None);

}	/* END PLOTREGRESSION */

/* -------------------------------------------------------------------- */
static void doLegendLineItem(PLOT_INFO *plot, DATASET_INFO *set, int ylegend, int color)
{
  int	start;

  MakeLegendLabel(buffer, set);

  if (set->scaleLocation == LEFT_SIDE)
    start = plot->x.xLegendText - 45;
  else
    start = plot->x.xLegendText +
	XTextWidth(xyyPlot[currentPanel].fontInfo[3+fontOffset],
	buffer, strlen(buffer)) + 5;

  if (color > 0)
    {
    if (Color)
      {
      XSetLineAttributes(plot->dpy,plot->gc, 3, LineSolid,CapButt,JoinMiter);
      XSetForeground(plot->dpy, plot->gc, GetColor(color));
      }

    XDrawLine(plot->dpy, plot->win, plot->gc, start, ylegend-2, start+40, ylegend-2);
    }

  XSetForeground(plot->dpy, plot->gc, GetColor(0));
  XDrawString(plot->dpy, plot->win, plot->gc, plot->x.xLegendText,
		ylegend+3, buffer, strlen(buffer));

}

/* END XYX.C */
