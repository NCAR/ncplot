/*
-------------------------------------------------------------------------
OBJECT NAME:	xyzX.c

FULL NAME:	Plot Track in X window

ENTRY POINTS:	DrawXYZ()
		ResizeXYZ()

STATIC FNS:	plotXYZ(),
		zTicsLabelsX(),
		draw3dAxies();

DESCRIPTION:	This is the Expose event procedure to regenerate the
		whole 3d trackimage.

REFERENCES:	X.c

REFERENCED:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2001
-------------------------------------------------------------------------
*/

#include "define.h"

#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

static int	ZD, fontOffset;

float	cosFactor = 0.866025, sinFactor = 0.5;

static void	plotXYZ(PLOT_INFO *, XFontStruct *),
		zTicsLabelsX(PLOT_INFO *plot, XFontStruct *fontInfo),
		draw3dAxies(PLOT_INFO *plot);


/* -------------------------------------------------------------------- */
void ResizeXYZ()
{
  int	n, depth;
  Arg	args[5];

  n = 0;
  XtSetArg(args[n], XmNwidth, &xyzPlot.x.windowWidth); ++n;
  XtSetArg(args[n], XmNheight, &xyzPlot.x.windowHeight); ++n;
  XtSetArg(args[n], XtNdepth, &depth); ++n;
  XtGetValues(xyzPlot.canvas, args, n);

  NewPixmap(&mainPlot[0], xyzPlot.x.windowWidth, xyzPlot.x.windowHeight, depth);

  xyzPlot.x.HD = (int)(xyzPlot.x.windowWidth * 0.47);
  xyzPlot.x.VD = (int)(xyzPlot.x.windowHeight * 0.4);
  xyzPlot.x.LV = (int)(xyzPlot.x.windowWidth * 0.095);
  xyzPlot.x.BH = (int)(xyzPlot.x.windowHeight * 0.878);

  xyzPlot.x.titleOffset    = 45;
  xyzPlot.x.subTitleOffset = 60;
  xyzPlot.x.yLabelOffset   = 5;

  xyzPlot.x.RV = xyzPlot.x.LV + xyzPlot.x.HD;
  xyzPlot.x.TH = xyzPlot.x.BH - xyzPlot.x.VD;

  xyzPlot.x.xLabelOffset = xyzPlot.x.windowHeight - 10;

  xyzPlot.x.ticLength = xyzPlot.x.HD > 250 ? 10 : 5;
  xyzPlot.x.yTicLabelOffset = 5;
  xyzPlot.x.xTicLabelOffset = 15;

  ZD = (int)(xyzPlot.x.HD * cosFactor);
  fontOffset = 1;

}	/* END RESIZETRACKWINDOW */

/* -------------------------------------------------------------------- */
void DrawXYZ()
{
  XFontStruct	*fontInfo;
  static bool	firstTime = True;

  /* Set default Graphics Context stuff and get the GC
   */
  if (firstTime)
    {
    ResizeXYZ();
    firstTime = False;
    }

  XSetClipMask(xyzPlot.dpy, xyzPlot.gc, None);
  ClearPixmap(&xyzPlot);

  plotTitlesX(&xyzPlot, fontOffset);
  draw3dAxies(&xyzPlot);

  plotLabelsX(&xyzPlot, fontOffset);

  fontInfo = xyzPlot.fontInfo[4];
  XSetFont(xyzPlot.dpy, xyzPlot.gc, fontInfo->fid);

  xTicsLabelsX(&xyzPlot, fontInfo, True);
  yTicsLabelsX(&xyzPlot, fontInfo, LEFT_SIDE, True);
  zTicsLabelsX(&xyzPlot, fontInfo);

  if (xyzSet[0].varInfo && xyzSet[1].varInfo && xyzSet[2].varInfo)
    {
    XSetLineAttributes(xyzPlot.dpy, xyzPlot.gc, LineThickness,
                      LineSolid, CapButt, JoinMiter);
    plotXYZ(&xyzPlot, fontInfo);
    XSetLineAttributes(xyzPlot.dpy, xyzPlot.gc, 1,
                      LineSolid, CapButt, JoinMiter);
    }

  DrawGeoPolMapXYZ(&xyzPlot, ZD, cosFactor, sinFactor, NULL);
  XSetForeground(xyzPlot.dpy, xyzPlot.gc, 0);
  PlotLandMarks3D(&xyzPlot, ZD, cosFactor, sinFactor, NULL);

  UpdateAnnotationsX(&xyzPlot);

}	/* END DRAWXYZ */

/* -------------------------------------------------------------------- */
static void plotXYZ(PLOT_INFO *plot, XFontStruct *fontInfo)
{
  int		i, cnt, cntXY, cntBack, cntSide, reqSize, segCnt = 0;
  XPoint	*pts,	/* Regular data points. */
		*ptsXY,	/* Points to project to XY plane. */
		*ptsBack,	/* Points to project to back wall. */
		*ptsSide;	/* Points to project to side wall. */

  float		xScale, yScale, zScale, xMin, yMin, zMin;
  NR_TYPE	datumX, datumY, datumZ;

  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;


  xMin = plot->Xaxis.min;
  yMin = plot->Yaxis[0].min;
  zMin = plot->Zaxis.min;

  xScale = (float)plot->x.HD / (plot->Xaxis.max - xMin);
  yScale = (float)plot->x.VD / (plot->Yaxis[0].max - yMin);
  zScale = (float)ZD / (plot->Zaxis.max - zMin);

  pts = (XPoint *)GetMemory(xyzSet[0].nPoints * sizeof(XPoint));

  if (ProjectToXY)
    ptsXY = (XPoint *)GetMemory(xyzSet[0].nPoints * sizeof(XPoint));

  if (ProjectToBack)
    {
    ptsBack = (XPoint *)GetMemory(xyzSet[0].nPoints * sizeof(XPoint));
    ptsSide = (XPoint *)GetMemory(xyzSet[0].nPoints * sizeof(XPoint));
    }


  for (i = 0; i < xyzSet[0].nPoints; )
    {
    /* First loop is to skip any start-up Missing Data.
     */
    do
      {
      datumX = xyzSet[0].data[(xyzSet[0].head + i) % xyzSet[0].nPoints];
      datumY = xyzSet[1].data[(xyzSet[1].head + i) % xyzSet[1].nPoints];
      datumZ = xyzSet[2].data[(xyzSet[2].head + i) % xyzSet[2].nPoints];
      ++i;
      }
    while (datumX == xyzSet[0].missingValue ||
           datumY == xyzSet[1].missingValue ||
           datumZ == xyzSet[2].missingValue);


    cnt = cntXY = cntBack = cntSide = 0;
    for (; cnt < reqSize && i < xyzSet[0].nPoints; ++cnt)
      {
      if (datumX == xyzSet[0].missingValue ||
          datumY == xyzSet[1].missingValue ||
          datumZ == xyzSet[2].missingValue)
        break;

      pts[cnt].x = (int)(plot->x.LV + (xScale * (datumX - xMin)));
      pts[cnt].y = (int)(plot->x.BH - (yScale * (datumY - yMin)));

      if (ProjectToBack)
        {
        ptsBack[cntBack].x = (int)(pts[cnt].x + cosFactor * ZD);
        ptsBack[cntBack].y = (int)(pts[cnt].y - sinFactor * ZD);
        ++cntBack;
        }

      pts[cnt].x += (int)(cosFactor * (zScale * (datumZ - zMin)));
      pts[cnt].y -= (int)(sinFactor * (zScale * (datumZ - zMin)));

      if (ProjectToXY)
        {
        ptsXY[cntXY].x = pts[cnt].x;
        ptsXY[cntXY].y = (int)(pts[cnt].y + (int)(yScale * (datumY - yMin)));
        ++cntXY;
        }

      if (ProjectToBack)
        {
        ptsSide[cntSide].x = pts[cnt].x;
        ptsSide[cntSide].x -= (int)(xScale * (datumX - xMin));

        ptsSide[cntSide].y = pts[cnt].y;
        ++cntSide;
        }


      if (nDirectionArrows &&
		(i+1) % (xyzSet[0].nPoints / nDirectionArrows) == 0)
        PlotDirectionArrow(plot, pts[cnt].x, pts[cnt].y, 
                           pts[cnt-4].x, pts[cnt-4].y, NULL);

      if (nTimeStamps && ((segCnt == 0 && cnt == 0) ||
			 (i+1) % (xyzSet[0].nPoints / nTimeStamps) == 0))
        PlotTimeStamps(plot, pts[cnt].x, pts[cnt].y,
                       (i+1) / (xyzSet[0].nPoints / nTimeStamps), NULL);
 

      /* Throw out duplicate points.
       */
      if (pts[cnt].x == pts[cnt-1].x && pts[cnt].y == pts[cnt-1].y)
        --cnt;

      if (ProjectToXY && ptsXY[cntXY].x == ptsXY[cntXY-1].x &&
			 ptsXY[cntXY].y == ptsXY[cntXY-1].y)
        --cntXY;

      if (ProjectToBack)
        {
        if (	ptsBack[cntBack].x == ptsBack[cntBack-1].x &&
		ptsBack[cntBack].y == ptsBack[cntBack-1].y)
          --cntBack;

        if (	ptsSide[cntSide].x == ptsSide[cntSide-1].x &&
		ptsSide[cntSide].y == ptsSide[cntSide-1].y)
          --cntSide;
        }

      datumX = xyzSet[0].data[(xyzSet[0].head + i) % xyzSet[0].nPoints];
      datumY = xyzSet[1].data[(xyzSet[1].head + i) % xyzSet[1].nPoints];
      datumZ = xyzSet[2].data[(xyzSet[2].head + i) % xyzSet[2].nPoints];
      ++i;
      }

    if (Color)
      XSetForeground(plot->dpy, plot->gc, GetColor(2));

    /* Draw projections first, then regular plot.
     */
    if (ProjectToXY)
      XDrawLines(plot->dpy, plot->win, plot->gc, ptsXY, cntXY, CoordModeOrigin);

    if (ProjectToBack)
      {
      XDrawLines(plot->dpy, plot->win, plot->gc, ptsBack, cntBack, CoordModeOrigin);
      XDrawLines(plot->dpy, plot->win, plot->gc, ptsSide, cntSide, CoordModeOrigin);
      }

    if (Color)
      XSetForeground(plot->dpy, plot->gc, GetColor(1));
    XDrawLines(plot->dpy, plot->win, plot->gc, pts, cnt, CoordModeOrigin);
    ++segCnt;
    }


  XSetForeground(plot->dpy, plot->gc, GetColor(0));
  XSetClipMask(plot->dpy, plot->gc, None);

  FreeMemory(pts);

  if (ProjectToXY)
    FreeMemory(ptsXY);

  if (ProjectToBack) {
    FreeMemory(ptsBack);
    FreeMemory(ptsSide);
    }

}	/* END PLOTXYZ */

/* -------------------------------------------------------------------- */
static void draw3dAxies(PLOT_INFO *plot)
{
  XPoint	pts[8];

  pts[0].x = plot->x.LV;
  pts[0].y = plot->x.TH;
  pts[1].x = plot->x.LV;
  pts[1].y = plot->x.BH;
  pts[2].x = plot->x.RV;
  pts[2].y = plot->x.BH;

  pts[3].x = pts[2].x + (int)(cosFactor * ZD);
  pts[3].y = pts[2].y - (int)(sinFactor * ZD);

  pts[4].x = pts[3].x - plot->x.HD;
  pts[4].y = pts[3].y;

  pts[5].x = plot->x.LV;
  pts[5].y = plot->x.BH;

  XDrawLines(plot->dpy, plot->win, plot->gc, pts, 6, CoordModeOrigin);

  pts[1].x = pts[0].x + (int)(cosFactor * ZD);
  pts[1].y = pts[0].y - (int)(sinFactor * ZD);

  pts[2].x = pts[1].x + plot->x.HD;
  pts[2].y = pts[1].y;

  pts[5].x = pts[4].x;
  pts[5].y = pts[4].y - plot->x.VD;

  XDrawLines(plot->dpy, plot->win, plot->gc, pts, 6, CoordModeOrigin);

}	/* END DRAW3DAXIES */

/* -------------------------------------------------------------------- */
static void zTicsLabelsX(PLOT_INFO *plot, XFontStruct *fontInfo)
{
  int		ticlen, i, xOffset, yOffset;
  float		nMajorZpix;
  double	zDiff, value;

  /* Draw Z-axis tic marks and #'s
   */
  ticlen	= plot->x.ticLength;
  nMajorZpix	= (float)ZD / plot->Zaxis.nMajorTics;
  zDiff		= plot->Zaxis.max - plot->Zaxis.min;

  for (i = 0; i <= plot->Zaxis.nMajorTics; ++i)
    {
    xOffset = plot->x.LV + (int)((nMajorZpix * i + 0.5) * cosFactor);
    yOffset = plot->x.BH - (int)((nMajorZpix * i) * sinFactor);

    XDrawLine(plot->dpy, plot->win, plot->gc,
          xOffset, yOffset, xOffset + ticlen, yOffset);

    xOffset += plot->x.HD;

    XDrawLine(plot->dpy, plot->win, plot->gc,
          xOffset - ticlen, yOffset, xOffset, yOffset);

    /* Label.
     */
    value = plot->Zaxis.min + (zDiff / plot->Zaxis.nMajorTics * i);
    MakeTicLabel(buffer, zDiff, plot->Zaxis.nMajorTics, value);

    XDrawString(plot->dpy, plot->win, plot->gc, xOffset + 5,
        yOffset + 5, buffer, strlen(buffer));
    }

}	/* END ZTICSLABELSX */

/* END XYZX.C */
