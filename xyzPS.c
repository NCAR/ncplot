/*
-------------------------------------------------------------------------
OBJECT NAME:	xyzPS.c

FULL NAME:	Produce PostScript File of 3D Track Plot

ENTRY POINTS:	PrintXYZ()

STATIC FNS:	PScube()
		PSzTics()
		PSplot3dTrack()
		ResizePStrackPlot()

DESCRIPTION:	This does PostScript printing.

REFERENCES:	ps.c

REFERENCED BY:	track Print Button

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2000
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

static void	PScube(FILE *fp, PLOT_INFO *plot),
		PSzTics(FILE *fp, PLOT_INFO *plot),
		PSplot3dTrack(FILE *fp, PLOT_INFO *plot),
		ResizePStrackPlot();

static int	ZD;

extern float	*NextColorRGB_PS();
extern float cosFactor, sinFactor;


/* -------------------------------------------------------------------- */
void PrintXYZ()
{
  FILE	*fp;

  ResizePStrackPlot();

  if ((fp = openPSfile(outFile)) == NULL)
    return;

  PSheader(fp, &xyzPlot);
  PStitles(fp, &xyzPlot);
  PScube(fp, &xyzPlot);

  /* Then move the origin
   */
  fprintf(fp, "%d %d translate\n", xyzPlot.ps.LV, xyzPlot.ps.BH);

  PSlabels(fp, &xyzPlot);
  fprintf(fp, "1 setlinewidth\n");
  PSxTics(fp, &xyzPlot, True);
  PSyTics(fp, &xyzPlot, 0, True);
  PSzTics(fp, &xyzPlot);
  fprintf(fp, "stroke 0 0 moveto\n");

  fprintf(fp, "newpath\n");
  fprintf(fp, "[] 0 setdash\n");

  PSplot3dTrack(fp, &xyzPlot);

  DrawGeoPolMapXYZ(&xyzPlot, ZD, cosFactor, sinFactor, fp);
  PlotLandMarks3D(&xyzPlot, ZD, cosFactor, sinFactor, fp);

  fprintf(fp, "%d %d translate\n", -xyzPlot.ps.LV, -xyzPlot.ps.BH);
  UpdateAnnotationsPS(&xyzPlot, fp);
  closePSfile(fp);

}	/* END TRACKPOSTSCRIPT */

/* -------------------------------------------------------------------- */
static void ResizePStrackPlot()
{
  /* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
   * (0,0) is in the lower left corner of the paper, held at portrait.
   */
  SetPlotRatios(&xyzPlot);

  xyzPlot.ps.LV = (int)(300 * printerSetup.widthRatio);
  xyzPlot.ps.BH = (int)(300 * printerSetup.heightRatio);

  xyzPlot.ps.HD = (int)(xyzPlot.ps.windowWidth * 0.47);
  xyzPlot.ps.VD = (int)(xyzPlot.ps.windowHeight * 0.4);

  xyzPlot.ps.TH = (int)(xyzPlot.ps.BH + xyzPlot.ps.VD);
  xyzPlot.ps.RV = (int)(xyzPlot.ps.LV + xyzPlot.ps.HD);


  xyzPlot.ps.ticLength		= (int)(25 * printerSetup.fontRatio);
  xyzPlot.ps.xLabelOffset	= (int)(-160 * printerSetup.fontRatio);
  xyzPlot.ps.yLabelOffset	= (int)(-200 * printerSetup.fontRatio);
  xyzPlot.ps.yTicLabelOffset	= (int)(-15 * printerSetup.fontRatio);
  xyzPlot.ps.xTicLabelOffset	= (int)(-45 * printerSetup.fontRatio);

  xyzPlot.ps.xLegendText	= (int)(-300 * printerSetup.widthRatio);

  ZD = (int)(xyzPlot.ps.HD * cosFactor);

}	/* END RESIZEPSTRACKPLOT */

/* -------------------------------------------------------------------- */
static void PScube(FILE *fp, PLOT_INFO *plot)
{
  int	x, y;

  /* Draw the bounding box for graph.
   */
  fprintf(fp, "3 setlinewidth\n");
  fprintf(fp, "stroke\n");
  x = plot->ps.LV;
  y = plot->ps.TH;
  fprintf(fp, moveto, x, y);

  y -= plot->ps.VD;
  fprintf(fp, lineto, x, y);

  x += plot->ps.HD;
  fprintf(fp, lineto, x, y);

  x += (int)(cosFactor * ZD);
  y += (int)(sinFactor * ZD);
  fprintf(fp, lineto, x, y);

  x -= plot->ps.HD;
  fprintf(fp, lineto, x, y);
  fprintf(fp, lineto, plot->ps.LV, plot->ps.BH);


  /* Do second part.
   */
  x = plot->ps.LV;
  y = plot->ps.TH;
  fprintf(fp, moveto, x, y);

  x += (int)(cosFactor * ZD);
  y += (int)(sinFactor * ZD);
  fprintf(fp, lineto, x, y);

  x += plot->ps.HD;
  fprintf(fp, lineto, x, y);

  y -= plot->ps.VD;
  fprintf(fp, lineto, x, y);

  x -= plot->ps.HD;
  fprintf(fp, lineto, x, y);

  y += plot->ps.VD;
  fprintf(fp, lineto, x, y);

}	/* END PSCUBE */

/* -------------------------------------------------------------------- */
void PSzTics(FILE *fp, PLOT_INFO *plot)
{
  int		i;
  int		x, y, ticlen;
  float		nMajorZpix;
  double	zDiff, value;

  ticlen = plot->grid ? plot->ps.VD : plot->ps.ticLength;
  nMajorZpix = (float)ZD / plot->Zaxis.nMajorTics;
  zDiff = plot->Zaxis.max - plot->Zaxis.min;

  for (i = 0; i <= plot->Zaxis.nMajorTics; ++i)
    {
    x = (int)((nMajorZpix * i + 0.5) * cosFactor);
    y = (int)((nMajorZpix * i) * sinFactor);

    fprintf(fp, moveto, x, y);
    fprintf(fp, lineto, x + ticlen, y);

    if (!plot->grid)
      {
      x += plot->ps.HD;
      fprintf(fp, moveto, x - ticlen, y);
      fprintf(fp, lineto, x, y);
      }

    value = (int)(plot->Zaxis.min + (zDiff / plot->Zaxis.nMajorTics * i));
    MakeTicLabel(buffer, zDiff, plot->Zaxis.nMajorTics, value);

    fprintf(fp, moveto, x + 25, y - 10);
    fprintf(fp, show, buffer);
    }

}	/* END PSZTICS */

/* -------------------------------------------------------------------- */
void PSplot3dTrack(FILE *fp, PLOT_INFO *plot)
{
  char		*p;
  int		i, nPts, indxX, indxY, indxZ;
  int		cnt, cntXY, cntBack, cntSide;
  XPoint	*pts, *ptsXY, *ptsBack, *ptsSide;
  float		x, y, x1, y1, xMin, yMin, zMin, *rgb;
  double	xScale, yScale, zScale;

  nPts = xyzSet[0].nPoints;

  xMin = plot->Xaxis.min;
  yMin = plot->Yaxis[0].min;
  zMin = plot->Zaxis.min;

  xScale = plot->ps.HD / (plot->Xaxis.max - xMin);
  yScale = plot->ps.VD / (plot->Yaxis[0].max - yMin);
  zScale = ZD / (plot->Zaxis.max - zMin);


  pts = (XPoint *)GetMemory(nPts * sizeof(XPoint));

  if (ProjectToXY)
    ptsXY = (XPoint *)GetMemory(nPts * sizeof(XPoint));

  if (ProjectToBack)
    {
    ptsBack = (XPoint *)GetMemory(nPts * sizeof(XPoint));
    ptsSide = (XPoint *)GetMemory(nPts * sizeof(XPoint));
    }

  cnt = cntXY = cntBack = cntSide = 0;
  ResetColors();


  /* Two main loops.  First one computes all the points.  Second one generates
   * the PostScript.
   */
  for (i = 0; i < nPts; ++i)
    {
    if (isMissingValue(xyzSet[0].data[(xyzSet[0].head + i) % nPts], xyzSet[0].missingValue))
      {
      while (isMissingValue(xyzSet[0].data[(xyzSet[0].head + i) % nPts], xyzSet[0].missingValue) && i < nPts)
        ++i;

      /* Mark the time break with "-1".
       */
      pts[cnt++].x = -1;

      if (ProjectToXY)
        ptsXY[cntXY++].x = -1;

      if (ProjectToBack)
        {
        ptsBack[cntBack].x = -1;
        ptsSide[cntSide++].x = -1;
        }
      }


    indxX = (xyzSet[0].head + i) % nPts;
    indxY = (xyzSet[1].head + i) % nPts;
    indxZ = (xyzSet[2].head + i) % nPts;

    x = xScale * (xyzSet[0].data[indxX] - xMin);
    y = yScale * (xyzSet[1].data[indxY] - yMin);

    if (ProjectToBack)
      {
      ptsBack[cntBack].x = (int)(x + (cosFactor * ZD));
      ptsBack[cntBack].y = (int)(y + (sinFactor * ZD));
      ++cntBack;
      }

    x += (int)(cosFactor * (zScale * (xyzSet[2].data[indxZ] - zMin)));
    y += (int)(sinFactor * (zScale * (xyzSet[2].data[indxZ] - zMin)));

    if (ProjectToXY)
      {
      ptsXY[cntXY].x = (int)x;
      ptsXY[cntXY].y = (int)(y - (yScale * (xyzSet[1].data[indxY] - yMin)));
      ++cntXY;
      }

    if (ProjectToBack)
      {
      ptsSide[cntSide].x = (int)x;
      ptsSide[cntSide].x -= (int)(xScale * (xyzSet[0].data[indxX] - xMin));

      ptsSide[cntSide].y = (int)y;
      ++cntSide;
      }

    pts[cnt].x = (int)x;
    pts[cnt].y = (int)y;
    ++cnt;

    /* Throw out duplicate points.
     */
    if (pts[cnt-2].x == pts[cnt-1].x && pts[cnt-2].y == pts[cnt-1].y)
      --cnt;

    if (ProjectToXY && ptsXY[cntXY-2].x == ptsXY[cntXY-1].x &&
                       ptsXY[cntXY-2].y == ptsXY[cntXY-1].y)
      --cntXY;

    if (ProjectToBack)
      {
      if (    ptsBack[cntBack-2].x == ptsBack[cntBack-1].x &&
              ptsBack[cntBack-2].y == ptsBack[cntBack-1].y)
        --cntBack;

      if (    ptsSide[cntSide-2].x == ptsSide[cntSide-1].x &&
              ptsSide[cntSide-2].y == ptsSide[cntSide-1].y)
        --cntSide;
      }

    if (nDirectionArrows && (i+1) % (nPts / nDirectionArrows) == 0)
      {
      float x2, y2;

      /* Compute 4 points ago, so we can figure out direction.
       */
      x2 = xScale * (xyzSet[0].data[indxX-4] - xMin),
      y2 = yScale * (xyzSet[1].data[indxY-4] - yMin);

      x2 += cosFactor * (zScale * (xyzSet[2].data[indxZ-4] - zMin));
      y2 += sinFactor * (zScale * (xyzSet[2].data[indxZ-4] - zMin));

      PlotDirectionArrow(plot, (int)x, (int)y, (int)x2, (int)y2, fp);
      }

    if (nTimeStamps && (i == 0 || (i+1) % (nPts / nTimeStamps) == 0))
      PlotTimeStamps(plot, (int)x, (int)y, (i+1) / (nPts / nTimeStamps), fp);
    }


  fprintf(fp, "%d setlinewidth\n", LineThickness<<1);

  if (printerSetup.color)
    {
    rgb = NextColorRGB_PS();
    fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0],rgb[1],rgb[2]);
    }

  for (i = 0; i < cnt; ++i)
    {
    p = (char *)lineto;

    while (pts[i].x == -1 || i == 0)
      {
      fprintf(fp, "stroke\nnewpath\n");
      p = (char *)moveto;
      ++i;
      }

    fprintf(fp, p, pts[i].x, pts[i].y);

    if (!(i % 1024)) {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, pts[i].x, pts[i].y);
      }
    }

  FreeMemory(pts);

  if (!ProjectToXY && !ProjectToBack)
    return;


  if (printerSetup.color)
    {
    rgb = NextColorRGB_PS();
    fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0],rgb[1],rgb[2]);
    }

  fprintf(fp, "stroke\nnewpath\n");

  for (i = 0; i < cntXY; ++i)
    {
    p = (char *)lineto;

    while (ptsXY[i].x == -1 || i == 0)
      {
      fprintf(fp, "stroke\nnewpath\n");
      p = (char *)moveto;
      ++i;
      }

    fprintf(fp, p, ptsXY[i].x, ptsXY[i].y);

    if (!(i % 1024)) {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, ptsXY[i].x, ptsXY[i].y);
      }
    }

  fprintf(fp, "stroke\nnewpath\n");

  for (i = 0; i < cntBack; ++i)
    {
    p = (char *)lineto;

    while (ptsBack[i].x == -1 || i == 0)
      {
      fprintf(fp, "stroke\nnewpath\n");
      p = (char *)moveto;
      ++i;
      }

    fprintf(fp, p, ptsBack[i].x, ptsBack[i].y);

    if (!(i % 1024)) {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, ptsBack[i].x, ptsBack[i].y);
      }
    }

  fprintf(fp, "stroke\nnewpath\n");

  for (i = 0; i < cntSide; ++i)
    {
    p = (char *)lineto;

    while (ptsSide[i].x == -1 || i == 0)
      {
      fprintf(fp, "stroke\nnewpath\n");
      p = (char *)moveto;
      ++i;
      }

    fprintf(fp, p, ptsSide[i].x, ptsSide[i].y);

    if (!(i % 1024)) {
      fprintf(fp, "stroke\n");
      fprintf(fp, moveto, ptsSide[i].x, ptsSide[i].y);
      }
    }

  fprintf(fp, "1 setlinewidth\n");

  if (ProjectToXY)
    FreeMemory(ptsXY);

  if (ProjectToBack)
    {
    FreeMemory(ptsBack);
    FreeMemory(ptsSide);
    }

}	/* END PSPLOT3DTRACK */

/* END XYZPS.C */
