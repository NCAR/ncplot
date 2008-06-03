/*
-------------------------------------------------------------------------
OBJECT NAME:	X.c

FULL NAME:	X-window ploting routines

ENTRY POINTS:	plotTitlesX()
		plotLabelsX()
		setClippingX()
		xTicsLabelsX()
		yTicsLabelsX()
		plotTimeSeries()
		plotXY()
		yLegendX()
		initPlotGC()
		copyGC()
		NewPixmap()
		ClearPixmap()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	plotX.c, diffX.c, specX.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2005
-------------------------------------------------------------------------
*/

#include "define.h"

/* -------------------------------------------------------------------- */
int yLegendX(PLOT_INFO *plot, int row)
{
  return(plot->x.windowHeight - 10 - (row * 12));

}	/* END YLEGENDX */

/* -------------------------------------------------------------------- */
void initPlotGC(PLOT_INFO *plot)
{
  int             n;
  Arg             args[3];
  unsigned long   valuemask;

  plot->dpy = XtDisplay(plot->canvas);
  plot->win = 0;

  n = 0;
  XtSetArg(args[n], XtNforeground, &plot->values.foreground); ++n;
  XtSetArg(args[n], XtNbackground, &plot->values.background); ++n;
  XtGetValues(plot->canvas, args, n);

  plot->values.line_width = LineThickness;
  valuemask = GCLineWidth | GCForeground | GCBackground;

  plot->gc = XCreateGC(	plot->dpy,
                        RootWindowOfScreen(XtScreen(plot->canvas)),
                        valuemask, &plot->values);

  if ((plot->fontInfo[0] = XLoadQueryFont(plot->dpy, iv.font24)) == NULL ||
      (plot->fontInfo[1] = XLoadQueryFont(plot->dpy, iv.font18)) == NULL ||
      (plot->fontInfo[2] = XLoadQueryFont(plot->dpy, iv.font14)) == NULL ||
      (plot->fontInfo[3] = XLoadQueryFont(plot->dpy, iv.font12)) == NULL ||
      (plot->fontInfo[4] = XLoadQueryFont(plot->dpy, iv.font10)) == NULL)
    HandleError("X.c: can't load font", false, EXIT);

  if (Color)
    XSetForeground(plot->dpy, plot->gc, GetColor(0));

}	/* END INITPLOTGC */

/* -------------------------------------------------------------------- */
void copyGC(PLOT_INFO *dest, PLOT_INFO *src)
{
  int	i;

  dest->canvas	= src->canvas;
  dest->dpy	= src->dpy;
  dest->win	= src->win;
  dest->gc	= src->gc;
  dest->values	= src->values;

  for (i = 0; i < MAX_FONTS; ++i)
    dest->fontInfo[i] = src->fontInfo[i];

}	/* END COPYGC */

/* -------------------------------------------------------------------- */
void NewPixmap(PLOT_INFO *plot, int width, int height, int depth)
{
  if (plot->dpy == 0)
    {
    fprintf(stderr, "NewPixmap: display is NULL, leaving.\n");
    return;
    }

  if (plot->win)
    XFreePixmap(plot->dpy, plot->win);

  plot->win = XCreatePixmap(plot->dpy, XtWindow(plot->canvas),
                  width, height, depth);

  if (plot == &mainPlot[0])  /* Cheezy hack. */
    {
    xyzPlot.win = mainPlot[0].win;

    for (size_t i = 0; i < MAX_PANELS; ++i)
      mainPlot[i].win = xyyPlot[i].win = plot->win;
    }

}	/* END NEWPIXMAP */

/* -------------------------------------------------------------------- */
void ClearPixmap(PLOT_INFO *plot)
{
  XSetForeground(plot->dpy, plot->gc, plot->values.background);

  XFillRectangle(plot->dpy, plot->win, plot->gc,
                 0, 0, plot->x.windowWidth, plot->x.windowHeight);

  if (Color)
    XSetForeground(plot->dpy, plot->gc, GetColor(0));
  else
    XSetForeground(plot->dpy, plot->gc, plot->values.foreground);

  XSetLineAttributes(plot->dpy, plot->gc, 1, LineSolid, CapButt, JoinMiter);

}	/* END CLEARPIXMAP */

/* -------------------------------------------------------------------- */
void plotTitlesX(PLOT_INFO *plot, int sizeOffset, bool showPrelimWarning)
{
  int	len, offset;

  /* Display title and subtitle
   */
  if ((len = plot->title.length()) > 0)
    {
    XSetFont(plot->dpy, plot->gc, plot->fontInfo[0+sizeOffset]->fid);
    offset = (plot->x.windowWidth >> 1) -
      (XTextWidth(plot->fontInfo[0+sizeOffset], plot->title.c_str(), len) >> 1);

    XDrawString(plot->dpy, plot->win, plot->gc, offset,
                plot->x.titleOffset, plot->title.c_str(), len);
    }

  if ((len = plot->subTitle.length()) > 0)
    {
    XSetFont(plot->dpy, plot->gc, plot->fontInfo[1+sizeOffset]->fid);
    offset = (plot->x.windowWidth >> 1) -
      (XTextWidth(plot->fontInfo[1+sizeOffset], plot->subTitle.c_str(), len) >> 1);

    XDrawString(plot->dpy, plot->win, plot->gc, offset,
                plot->x.subTitleOffset, plot->subTitle.c_str(), len);
    }

  if (showPrelimWarning)
    plotWarningX(plot, sizeOffset);

}	/* END PLOTTITLESX */

/* -------------------------------------------------------------------- */
void plotWarningX(PLOT_INFO *plot, int sizeOffset)
{
  /*
   * Display message if this file is preliminary data (global attribute
   * in the netCDF file.
   */
  int len = strlen(prelimWarning);

  XSetFont(plot->dpy, plot->gc, plot->fontInfo[2+sizeOffset]->fid);
  int xOffset = (plot->x.windowWidth >> 1) -
	(XTextWidth(plot->fontInfo[2+sizeOffset], prelimWarning, len) >> 1);

  int yOffset =
	PlotType == XYZ_PLOT ? plot->x.subTitleOffset+20 : plot->x.TH-35;

  XDrawString(	plot->dpy, plot->win, plot->gc, xOffset,
		yOffset, prelimWarning, len);

}	/* END PLOTWARNINGX */

/* -------------------------------------------------------------------- */
void plotLabelsX(PLOT_INFO *plot, int sizeOffset)
{
  int		axis, len, xOffset, yOffset;
  int		ascent, charSize, pixLen;
  XFontStruct	*fontInfo = plot->fontInfo[1+sizeOffset];
  XImage	*im_in, *im_out;

  static bool	firstTime = true;
  static Pixmap	in_pm, out_pm;

  ascent	= fontInfo->max_bounds.ascent;
  charSize	= ascent + fontInfo->max_bounds.descent;

  XSetFont(plot->dpy, plot->gc, fontInfo->fid);

  if (firstTime)
    {
    Arg	args[2];
    int depth, width = 500, height = 30;

    XtSetArg(args[0], XtNdepth, &depth);
    XtGetValues(plot->canvas, args, 1);

    in_pm  = XCreatePixmap(plot->dpy, plot->win, width, height, depth);
    out_pm = XCreatePixmap(plot->dpy, plot->win, height, width, depth);

    firstTime = false;
    }

  if ((len = plot->Xaxis.label.length()) > 0)
    {
    xOffset = plot->x.LV + (plot->x.HD >> 1) -
            (XTextWidth(fontInfo, plot->Xaxis.label.c_str(), len) >> 1);

    XDrawString(plot->dpy, plot->win, plot->gc, xOffset,
                plot->x.xLabelOffset, plot->Xaxis.label.c_str(), len);
    }


  /* For the y axis label, we must rotate the text manually
   * Setup initial pixmaps for rotation, this is done once
   * because rotation is incredibly slow.
   */
  for (axis = 0; axis < 2; ++axis)
    {
    int	i, j;

    xOffset = axis == 0 ? plot->x.yLabelOffset :
          plot->x.windowWidth - plot->x.yLabelOffset - ascent;

    if ((len = plot->Yaxis[axis].label.length()) == 0)
      continue;

    pixLen = XTextWidth(fontInfo, plot->Yaxis[axis].label.c_str(), len);

    /* Create a pixmap, draw string to it and read it back out as an Image.
     */
    XSetForeground(plot->dpy, plot->gc, plot->values.background);
    XFillRectangle(plot->dpy, in_pm, plot->gc, 0, 0, pixLen, charSize);

    if (Color)
      XSetForeground(plot->dpy, plot->gc, GetColor(0));
    else
      XSetForeground(plot->dpy, plot->gc, plot->values.foreground);

    XDrawString(plot->dpy, in_pm, plot->gc, 0, ascent,
                plot->Yaxis[axis].label.c_str(), len);

    im_in = XGetImage(plot->dpy, in_pm, 0, 0, pixLen, charSize, 0xffffffff, XYPixmap);
    im_out = XGetImage(plot->dpy, out_pm, 0,0, charSize,pixLen, 0xffffffff, XYPixmap);


    /* Rotate pixmap
     */
    for (i = 0; i < charSize; i++)
      for (j = 0; j < pixLen; j++)
        XPutPixel(im_out, i, pixLen-j-1, XGetPixel(im_in, j, i));


    /* Write out the rotated text
     */
    yOffset = plot->x.TH + (plot->x.VD >> 1) - (pixLen >> 1);

    XPutImage(plot->dpy, plot->win, plot->gc, im_out, 0, 0,
              xOffset, yOffset, charSize, pixLen);

    XDestroyImage(im_in);
    XDestroyImage(im_out);
    }


  /* Z label.
   */
  if ((len = plot->Zaxis.label.length()) > 0)
    {
    XDrawString(plot->dpy, plot->win, plot->gc, plot->x.RV + 100,
                plot->x.BH - 20, plot->Zaxis.label.c_str(), len);
    }

}	/* END PLOTLABELSX */

/* -------------------------------------------------------------------- */
void setClippingX(PLOT_INFO *plot)
{
  XRectangle  clip_area[1];

  /* Set clipping so that graph data cannot exceed box boundries
   */
  clip_area[0].x = plot->x.LV;
  clip_area[0].y = plot->x.TH;
  clip_area[0].width = plot->x.HD;
  clip_area[0].height = plot->x.VD;
  XSetClipRectangles(plot->dpy, plot->gc, 0, 0, clip_area, 1, Unsorted);

}	/* END SETCLIPPINGX */

/* -------------------------------------------------------------------- */
void yTicsLabelsX(PLOT_INFO *plot, XFontStruct *fontInfo, int scale, bool labels)
{
  int		i, j, xoffset, yoffset, len, ticlen;
  float		nMajorYpix, nMinorYpix;
  double	yDiff, value;
  struct axisInfo	*yAxis = &plot->Yaxis[scale];

  XSetFont(plot->dpy, plot->gc, fontInfo->fid);

  if (yAxis->logScale) {	// Moved Log Tics out of here.
    yLogTicsLabelsX(plot, fontInfo, scale, labels);
    return;
    }

  /* Draw Y-axis tic marks and #'s
   */
  yDiff		= yAxis->max - yAxis->min;
  ticlen	= plot->grid ? plot->x.HD : plot->x.ticLength;
  nMajorYpix	= (float)plot->x.VD / yAxis->nMajorTics;
  nMinorYpix	= nMajorYpix / yAxis->nMinorTics;

  for (i = 0; i <= yAxis->nMajorTics; ++i)
    {
    yoffset = (int)(plot->x.BH - (nMajorYpix * i));

    if (yoffset < plot->x.TH)
      yoffset = plot->x.TH;

    XDrawLine(plot->dpy, plot->win, plot->gc,
              plot->x.LV, yoffset, plot->x.LV + ticlen, yoffset);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      XDrawLine(plot->dpy, plot->win, plot->gc,
                plot->x.RV - ticlen, yoffset, plot->x.RV, yoffset);

    if (labels)
      {
      if (yAxis->invertAxis)
        value = yAxis->max - (yDiff / yAxis->nMajorTics * i);
      else
        value = yAxis->min + (yDiff / yAxis->nMajorTics * i);

      len = MakeTicLabel(buffer, yDiff, yAxis->nMajorTics, value);

      if (scale == LEFT_SIDE)
        xoffset = plot->x.LV - plot->x.yTicLabelOffset -
              XTextWidth(fontInfo, buffer, len);
      else
        xoffset = plot->x.RV + plot->x.yTicLabelOffset;

      XDrawString(plot->dpy,plot->win,plot->gc, xoffset, yoffset+4, buffer,len);
      }

    if (i != yAxis->nMajorTics)
      {
      int	minorTicLen = ticlen;

      if (!plot->grid)
        minorTicLen = minorTicLen * 2 / 3;

      for (j = 1; j < yAxis->nMinorTics; ++j)
        {
        yoffset = (int)(plot->x.BH - (nMajorYpix * i) - (nMinorYpix * j));

        XDrawLine(plot->dpy, plot->win, plot->gc, plot->x.LV, yoffset,
                  plot->x.LV + minorTicLen, yoffset);

        if (!plot->grid && plot->plotType != XYZ_PLOT)
          XDrawLine(plot->dpy, plot->win, plot->gc, plot->x.RV, yoffset,
                    plot->x.RV - minorTicLen, yoffset);
        }
      }
    }

}	/* END YTICSLABELSX */

/* -------------------------------------------------------------------- */
void xTicsLabelsX(PLOT_INFO *plot, XFontStruct *fontInfo, bool labels)
{
  int		ticlen, i, j, offset, len;
  float		nMajorXpix, nMinorXpix;
  double	xDiff, value;
  struct axisInfo	*xAxis = &plot->Xaxis;

  XSetFont(plot->dpy, plot->gc, fontInfo->fid);

  if (xAxis->logScale) {	// Moved Log Tics out of here.
    xLogTicsLabelsX(plot, fontInfo, labels);
    return;
    }


  /* Draw X-axis tic marks and #'s
   */
  xDiff		= xAxis->max - xAxis->min;
  ticlen	= plot->grid ? plot->x.VD : plot->x.ticLength;
  nMajorXpix	= (float)plot->x.HD / xAxis->nMajorTics;
  nMinorXpix	= (float)nMajorXpix / xAxis->nMinorTics;


  for (i = 0; i <= xAxis->nMajorTics; ++i)
    {
    if (xAxis->invertAxis)
      offset = (int)(plot->x.RV - (nMajorXpix * i + 0.5));
    else
      offset = (int)(plot->x.LV + (nMajorXpix * i + 0.5));

    if (offset > plot->x.RV) offset = plot->x.RV;
    if (offset < plot->x.LV) offset = plot->x.LV;

    XDrawLine(plot->dpy, plot->win, plot->gc,
              offset, plot->x.BH, offset, plot->x.BH - ticlen);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      XDrawLine(plot->dpy, plot->win, plot->gc, offset, plot->x.TH,
                offset, plot->x.TH + ticlen);

    /* Label.
     */
    if (labels)
      {
      if (plot->plotType == TIME_SERIES)
        len = MakeTimeTicLabel(buffer, i, xAxis->nMajorTics);
      else
        {
        value = xAxis->min + (xDiff / xAxis->nMajorTics * i);
        len = MakeTicLabel(buffer, xDiff, xAxis->nMajorTics, value);
        }

      offset -= (XTextWidth(fontInfo, buffer, len) >> 1);
      XDrawString(plot->dpy, plot->win, plot->gc, offset,
		plot->x.BH + plot->x.xTicLabelOffset, buffer, len);
      }

    if (i != xAxis->nMajorTics)
      {
      int	minorTicLen = ticlen;

      if (!plot->grid)
        minorTicLen = (minorTicLen << 1) / 3;

      for (j = 1; j < xAxis->nMinorTics; ++j)
        {
        offset = (int)(plot->x.LV + ((nMajorXpix * i) + (nMinorXpix * j) +0.5));

        XDrawLine(plot->dpy, plot->win, plot->gc, offset, plot->x.BH,
                  offset, plot->x.BH - minorTicLen);

        if (!plot->grid && plot->plotType != XYZ_PLOT)
          XDrawLine(plot->dpy, plot->win, plot->gc, offset, plot->x.TH,
                    offset, plot->x.TH + minorTicLen);
        }
      }
    }

}	/* END XTICSLABELSX */

/* -------------------------------------------------------------------- */
void plotTimeSeries(PLOT_INFO *plot, DATASET_INFO *set)
{
  size_t	i, cnt, reqSize;
  XPoint	*pts;
  NR_TYPE	datum;
  float		xScale, yScale, halfSecond, yMin;
  struct axisInfo	*yAxis;

  yAxis = &plot->Yaxis[set->scaleLocation];

  if (yAxis->logScale)
    {
    yMin = log10(yAxis->min);
    yScale = (float)plot->x.VD / (log10(yAxis->max) - yMin);
    }
  else
    {
    yMin = yAxis->min;
    yScale = (float)plot->x.VD / (yAxis->max - yMin);
    }

  xScale = (float)plot->x.HD / set->nPoints;

  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;

  pts = new XPoint[set->nPoints];

  if (set->nPoints == NumberSeconds)
    halfSecond = plot->x.HD / NumberSeconds / 2;
  else
  if (set->nPoints < NumberSeconds)
    halfSecond = (plot->x.HD / NumberSeconds) *
                 (dataFile[set->fileIndex].baseDataRate / 2);
  else
    halfSecond = 0.0;

  /* Display lines            */
  setClippingX(plot);


  /* Calculate points for lines.
   */
  for (i = 0; i < set->nPoints; ++i)
    {
    while (isMissingValue((datum = set->data[(set->head + i) % set->nPoints]),
	set->missingValue) && i < set->nPoints)
      ++i;

    for (cnt = 0; !isMissingValue(datum, set->missingValue) &&
              cnt < reqSize && i < set->nPoints; ++cnt)
      {
      if (yAxis->logScale)
        {
        if (datum <= 0.0)
          datum = yMin;
        else
          datum = log10(datum);
        }

      pts[cnt].x = (int)(plot->x.LV + (xScale * i) + halfSecond);

      if (yAxis->invertAxis)
        pts[cnt].y = (int)(plot->x.TH + (yScale * (datum - yMin)));
      else
        pts[cnt].y = (int)(plot->x.BH - (yScale * (datum - yMin)));

      /* Do some level of clipping, so server doesn't take forever.	*/
      if (pts[cnt].y < -10000) pts[cnt].y = -10000;
      if (pts[cnt].y > 10000) pts[cnt].y = 10000;


      /* Throw out duplicate points.
       */
      if (cnt > 0 && pts[cnt].x == pts[cnt-1].x && pts[cnt].y == pts[cnt-1].y)
        --cnt;

      /* Get next data value */
      datum = set->data[(set->head + ++i) % set->nPoints];
      }


    if (cnt > 1)
      XDrawLines(plot->dpy, plot->win, plot->gc, pts, cnt, CoordModeOrigin);
    else
      { /* if it's just a point then draw small cross. */
      XDrawLine(plot->dpy, plot->win, plot->gc,
		pts[0].x, pts[0].y-2, pts[0].x, pts[0].y+2);
      XDrawLine(plot->dpy, plot->win, plot->gc,
		pts[0].x-2, pts[0].y, pts[0].x+2, pts[0].y);
      }
    }

  XSetClipMask(plot->dpy, plot->gc, None);

  delete [] pts;

}	/* END PLOTTIMESERIES */

/* -------------------------------------------------------------------- */
void plotXY(PLOT_INFO *plot, DATASET_INFO *Xset, DATASET_INFO *Yset, int color)
{
  int		i, cnt, nPts, reqSize, xIn, yIn, segCnt = 0;
  XPoint	*pts;
  NR_TYPE	datumX, datumY;
  float		xScale, yScale, xRatio, yRatio, xMin, yMin;
  struct axisInfo	*xAxis, *yAxis;

  xAxis = &plot->Xaxis;
  yAxis = &plot->Yaxis[Yset->scaleLocation];


  if (xAxis->logScale) {
    xMin = log10(xAxis->min);
    xScale = (float)plot->x.HD / (log10(xAxis->max) - xMin);
    }
  else {
    xMin = xAxis->min;
    xScale = (float)plot->x.HD / (xAxis->max - xMin);
    }

  if (yAxis->logScale) {
    yMin = log10(yAxis->min);
    yScale = (float)plot->x.VD / (log10(yAxis->max) - yMin);
    }
  else {
    yMin = yAxis->min;
    yScale = (float)plot->x.VD / (yAxis->max - yMin);
    }

  xRatio = yRatio = 1.0;
 
  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;

  /* dataSet[0] is guaranteed to be the X axis variable.
   */
  nPts = std::max(Xset->nPoints, Yset->nPoints);
  pts = new XPoint[nPts];
 
  if (Xset->nPoints > Yset->nPoints)
    yRatio = (float)Yset->nPoints / Xset->nPoints;
  else
  if (Xset->nPoints < Yset->nPoints)
    xRatio = (float)Xset->nPoints / Yset->nPoints;


  for (i = 0; i < nPts; )
    {
    /* First loop is to skip any start-up Missing Data.
     */
    do
      {
      xIn = (int)(xRatio * i);	/* This is to handle differing data rates. */
      yIn = (int)(yRatio * i);

      datumX = Xset->data[(Xset->head + xIn) % Xset->nPoints];
      datumY = Yset->data[(Yset->head + yIn) % Yset->nPoints];

      ++i;
      }
    while (isMissingValue(datumX, Xset->missingValue) || isMissingValue(datumY, Yset->missingValue));

//    ++nPts;

    for (cnt = 0; cnt < reqSize && i < nPts; ++cnt)
      {
      if (isMissingValue(datumX, Xset->missingValue) || isMissingValue(datumY, Yset->missingValue))
        break;

      if (xAxis->logScale)
        {
        if (datumX <= 0.0)
          datumX = xMin;
        else
          datumX = log10(datumX);
        }

      if (yAxis->logScale)
        {
        if (datumY <= 0.0)
          datumY = yMin;
        else
          datumY = log10(datumY);
        }

      if (xAxis->invertAxis)
        pts[cnt].x = (int)(plot->x.RV - (xScale * (datumX - xMin)));
      else
        pts[cnt].x = (int)(plot->x.LV + (xScale * (datumX - xMin)));

      if (yAxis->invertAxis)
        pts[cnt].y = (int)(plot->x.TH + (yScale * (datumY - yMin)));
      else
        pts[cnt].y = (int)(plot->x.BH - (yScale * (datumY - yMin)));

      /* Do some level of clipping, so server doesn't take forever.	*/
      if (pts[cnt].x < -10000) pts[cnt].x = -10000;
      if (pts[cnt].y < -10000) pts[cnt].y = -10000;
      if (pts[cnt].x > 10000) pts[cnt].x = 10000;
      if (pts[cnt].y > 10000) pts[cnt].y = 10000;


      if (nDirectionArrows && (i+1) % (nPts / nDirectionArrows) == 0)
        PlotDirectionArrow(plot, pts[cnt].x, pts[cnt].y,
                           pts[cnt-4].x, pts[cnt-4].y, NULL);

      if (nTimeStamps && ((segCnt == 0 && cnt == 0) ||
			 (i+1) % (nPts / nTimeStamps) == 0))
        {
//        printf ("(i+1)=%d, nPts=%d/nTimeStamps=%d = %d\n",i+1,nPts,nTimeStamps,nPts/nTimeStamps);
        PlotTimeStamps(plot, pts[cnt].x, pts[cnt].y, (i+1) / (nPts / nTimeStamps), NULL);
        }

      /* Throw out duplicate points.
       */
      if (cnt > 0 && pts[cnt].x == pts[cnt-1].x && pts[cnt].y == pts[cnt-1].y)
        --cnt;

      /* Get next data values.
       */
      xIn = (int)(xRatio * i);	/* This is to handle differing data rates. */
      yIn = (int)(yRatio * i);

      datumX = Xset->data[(Xset->head + xIn) % Xset->nPoints];
      datumY = Yset->data[(Yset->head + yIn) % Yset->nPoints];
      ++i;
      }

    /* Display lines            */
    setClippingX(plot);

    if (Color)
      XSetForeground(plot->dpy, plot->gc, GetColor(color));

    if (ScatterPlot)
      XDrawPoints(plot->dpy, plot->win, plot->gc, pts, cnt, CoordModeOrigin);
    else
      XDrawLines(plot->dpy, plot->win, plot->gc, pts, cnt, CoordModeOrigin);

    ++segCnt;
    }

  XSetClipMask(plot->dpy, plot->gc, None);

  delete [] pts;

}	/* END PLOTXY */

/* END X.C */
