/*
-------------------------------------------------------------------------
OBJECT NAME:	geopolmap.c

FULL NAME:	Direction Arrows.

ENTRY POINTS:	DrawGeoPolMapXY()
		DrawGeoPolMapXYZ()
		ToggleGeoPolMap()

STATIC FNS:	createCoastCommand()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2004
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <Xm/TextF.h>

static bool	GeoPolMap = False;

static void createCoastCommand(char buf[], struct axisInfo *xAxis, struct axisInfo *yAxis);

/* -------------------------------------------------------------------- */
void ToggleGeoPolMap(Widget w, XtPointer client, XtPointer call)
{
  GeoPolMap = !GeoPolMap;

  DrawMainWindow();
 
}	/* END TOGGLEGEOPOLMAP */

/* -------------------------------------------------------------------- */
void DrawGeoPolMapXY(PLOT_INFO *plot, FILE *fp)
{
  FILE	*in;

  int           i, cnt, nPts, reqSize;
  XPoint        *pts;
  NR_TYPE       datumX, datumY;
  float         xScale, yScale, xMin, yMin;
  struct axisInfo       *xAxis, *yAxis;
  struct plot_offset	*offsets;

  if (!GeoPolMap)
    return;


  /* Setup useful consts
   */
  xAxis = &plot->Xaxis;
  yAxis = &plot->Yaxis[0];
 
  xMin = xAxis->min;
  yMin = yAxis->min;
 
  if (fp)
    offsets = &plot->ps;
  else
    offsets = &plot->x;

  xScale = (float)offsets->HD / (xAxis->max - xMin);
  yScale = (float)offsets->VD / (yAxis->max - yMin);
 
  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;
  pts = (XPoint *)GetMemory(reqSize * sizeof(XPoint));


  /* Open pipe to pscoast
   */
  createCoastCommand(buffer, xAxis, yAxis);
//printf("%s\n", buffer);
  if ((in = popen(buffer, "r")) == NULL)
    {
    fprintf(stderr, "pscoast command failed.\n");
    fprintf(stderr, " %s\n", buffer);
    return;
    }


  if (fp)	/* PostScript */
    {
    fprintf(fp, "gsave\n");
    fprintf(fp, "stroke 0 0 0 setrgbcolor\n");
    PSclip(fp, plot);
    }
  else
    {
    XSetForeground(plot->dpy, plot->gc, GetColor(0));
    setClippingX(plot);
    }
 

  while (fgets(buffer, 1024, in) > 0)
    {
    if (buffer[0] == '#' || buffer[0] == '>')
      continue;

    cnt = 0;

    do
      {
      sscanf(buffer, "%f %f", &datumX, &datumY);

      if (datumX > 180.0)
        datumX -= 360.0;

      if (fp)
        {
        pts[cnt].x = (int)(xScale * (datumX - xMin));
        pts[cnt].y = (int)(yScale * (datumY - yMin));
        }
      else
        {
        pts[cnt].x = (int)(offsets->LV + (xScale * (datumX - xMin)));
        pts[cnt].y = (int)(offsets->BH - (yScale * (datumY - yMin)));
        }

      ++cnt;
      }
    while (fgets(buffer, 1024, in) > 0 && buffer[0] != '>');

    if (fp)
      {
      fprintf(fp, moveto, pts[0].x, pts[0].y);

      for (i = 1; i < cnt; ++i)
        fprintf(fp, lineto, pts[i].x, pts[i].y);

      fprintf(fp, "stroke\n");
      }
    else
      XDrawLines(plot->dpy, plot->win, plot->gc, pts, cnt, CoordModeOrigin);
    }
 
  FreeMemory(pts);
  pclose(in);

  if (fp)	/* PostScript */
    {
    PSclearClip(fp);
    fprintf(fp, "stroke grestore\n");
    }
  else
    {
    XSetClipMask(plot->dpy, plot->gc, None);
    XSetForeground(plot->dpy, plot->gc, CurrentColor());
    }

}	/* END DRAWGEOPOLMAP */

/* -------------------------------------------------------------------- */
void DrawGeoPolMapXYZ(PLOT_INFO *plot, int ZD, float cosFac, float sinFac, FILE *fp)
{
  FILE	*in;

  int           i, cnt, nPts, reqSize;
  XPoint        *pts;
  NR_TYPE       datumX, datumY;
  float         xScale, yScale, zScale, xMin, zMin, xMax, zMax, x, y;
  struct axisInfo       *xAxis, *yAxis;
  struct plot_offset	*offsets;

  if (!GeoPolMap)
    return;


  /* Setup useful consts
   */
  xAxis = &plot->Xaxis;
  yAxis = &plot->Yaxis[0];
 
  xMin = xAxis->min;
  xMax = xAxis->max;
  zMin = plot->Zaxis.min;
  zMax = plot->Zaxis.max;
 
  if (fp)
    offsets = &plot->ps;
  else
    offsets = &plot->x;

  xScale = (float)offsets->HD / (xAxis->max - xMin);
  yScale = (float)offsets->VD / (yAxis->max - yAxis->min);
  zScale = (float)ZD / (zMax - zMin);
 
  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;
  pts = (XPoint *)GetMemory(reqSize * sizeof(XPoint));


  /* Open pipe to pscoast
   */
  createCoastCommand(buffer, xAxis, &plot->Zaxis);
  if ((in = popen(buffer, "r")) == NULL)
    {
    fprintf(stderr, "pscoast command failed.\n");
    fprintf(stderr, " %s\n", buffer);
    return;
    }


  if (fp)	/* PostScript */
    {
    fprintf(fp, "gsave\n");
    fprintf(fp, "stroke 0 0 0 setrgbcolor\n");
    }
  else
    XSetForeground(plot->dpy, plot->gc, GetColor(0));
 

  while (fgets(buffer, 1024, in) > 0)
    {
    if (buffer[0] == '#' || buffer[0] == '>')
      continue;

    cnt = 0;

    do
      {
      sscanf(buffer, "%f %f", &datumX, &datumY);

      if (datumX > 180.0)
        datumX -= 360.0;

      if (datumY < zMin || datumY > zMax || datumX < xMin || datumX > xMax)
        continue;

      if (fp)
        {
        x = xScale * (datumX - xMin);
        y = yScale * (0.0 - yAxis->min);

        x += cosFac * (zScale * (datumY - zMin));
        y += sinFac * (zScale * (datumY - zMin));
        }
      else
        {
        x = offsets->LV + (xScale * (datumX - xMin));
        y = offsets->BH - (yScale * (0.0 - yAxis->min));

        x += cosFac * (zScale * (datumY - zMin));
        y -= sinFac * (zScale * (datumY - zMin));
        }

      pts[cnt].x = (int)x;
      pts[cnt].y = (int)y;
      ++cnt;
      }
    while (fgets(buffer, 1024, in) > 0 && buffer[0] != '>');

    if (fp)
      {
      fprintf(fp, moveto, pts[0].x, pts[0].y);

      for (i = 1; i < cnt; ++i)
        fprintf(fp, lineto, pts[i].x, pts[i].y);

      fprintf(fp, "stroke\n");
      }
    else
      XDrawLines(plot->dpy, plot->win, plot->gc, pts, cnt, CoordModeOrigin);
    }
 
  FreeMemory(pts);
  pclose(in);

  if (fp)	/* PostScript */
    fprintf(fp, "stroke grestore\n");
  else
    XSetForeground(plot->dpy, plot->gc, CurrentColor());

}	/* END DRAWGEOPOLMAPXYZ */

/* -------------------------------------------------------------------- */
static void createCoastCommand(char buf[], struct axisInfo *xAxis, struct axisInfo *yAxis)
{
  float	scale;
  int	xMin, xMax, yMin, yMax;

  xMin = (xAxis->min < 0.0) ? (int)xAxis->min-1 : (int)xAxis->min;
  yMin = (yAxis->min < 0.0) ? (int)yAxis->min-1 : (int)yAxis->min;
  xMax = (xAxis->max < 0.0) ? (int)xAxis->max : (int)xAxis->max+1;
  yMax = (yAxis->max < 0.0) ? (int)yAxis->max : (int)yAxis->max+1;

  /* Determine which resolution GMT database to use.
   */
  scale = sqrt((xAxis->max - xAxis->min) * (yAxis->max - yAxis->min));

  sprintf(buf, "pscoast -R%d/%d/%d/%d -M -Na -W ", xMin, xMax, yMin, yMax);

  if (scale > 60)	strcat(buf, "-Dc");	else
  if (scale > 40)	strcat(buf, "-Dl");	else
  if (scale > 20)	strcat(buf, "-I1 -Di"); else
  if (scale > 10)	strcat(buf, "-Ir -Dh"); else
			strcat(buf, "-Ia -Df");

}	/* END CREATECOASTCOMMAND */

/* END GEOPOLMAP.C */
