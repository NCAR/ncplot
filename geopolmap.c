/*
-------------------------------------------------------------------------
OBJECT NAME:	geopolmap.c

FULL NAME:	Direction Arrows.

ENTRY POINTS:	DrawGeoPolMapXY()
		DrawGeoPolMapXYZ()
		ToggleGeoPolMap()

STATIC FNS:	createCoastCommand()
		setColor()

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2006
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"

#include <sys/stat.h>
#include <Xm/TextF.h>

static bool	GeoPolMap = False;
static char	gmt_path[128];

static void createCoastCommand(char buf[], struct axisInfo *xAxis, struct axisInfo *yAxis);
static void setColor(PLOT_INFO * plot, char str[], FILE *fp);

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

  int           i, cnt, reqSize;
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
 

  /* Open pipe to pscoast
   */
  createCoastCommand(buffer, xAxis, yAxis);

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
 

  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;
  pts = new XPoint[reqSize];

  while (fgets(buffer, 1024, in) > 0)
    {
    if (buffer[0] == '#' || buffer[0] == '>')
    {
      setColor(plot, buffer, fp);
      continue;
    }

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

    setColor(plot, buffer, fp);

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
 
  delete [] pts;
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

  int           i, cnt, reqSize;
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
 

  reqSize = (XMaxRequestSize(plot->dpy) - 3) >> 1;
  pts = new XPoint[reqSize];

  while (fgets(buffer, 1024, in) > 0)
    {
    if (buffer[0] == '#' || buffer[0] == '>')
    {
      setColor(plot, buffer, fp);
      continue;
    }

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

    setColor(plot, buffer, fp);

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
 
  delete [] pts;
  pclose(in);

  if (fp)	/* PostScript */
    fprintf(fp, "stroke grestore\n");
  else
    XSetForeground(plot->dpy, plot->gc, CurrentColor());

}	/* END DRAWGEOPOLMAPXYZ */

/* -------------------------------------------------------------------- */
bool TestForGMT()
{
  bool rc = false;
  char *env;
  struct stat sb;

  gmt_path[0] = '\0';

  if ( (env = getenv("GMTHOME")) )	// home built GMT
  {
    char test_path[128];

    strcpy(test_path, env);
    strcat(test_path, "/bin/gmt");

    stat(test_path, &sb);
    if ((sb.st_mode & S_IFMT) == S_IFREG) {
      strcpy(gmt_path, env);
      rc = true;
    }
  }


  stat("/usr/local/bin/gmt", &sb);
  if ((sb.st_mode & S_IFMT) == S_IFREG) {
    strcpy(gmt_path, "/usr/local");
    rc = true;
  }

  stat("/bin/gmt", &sb);
  if ((sb.st_mode & S_IFMT) == S_IFREG) {
    rc = true;
  }

  return rc;
}

/* -------------------------------------------------------------------- */
static void createCoastCommand(char buf[], struct axisInfo *xAxis, struct axisInfo *yAxis)
{
  char  scale_str[16], river_str[16], command[256];
  float	scale;
  int	xMin, xMax, yMin, yMax;

  xMin = (xAxis->min < 0.0) ? (int)xAxis->min-1 : (int)xAxis->min;
  yMin = (yAxis->min < 0.0) ? (int)yAxis->min-1 : (int)yAxis->min;
  xMax = (xAxis->max < 0.0) ? (int)xAxis->max : (int)xAxis->max+1;
  yMax = (yAxis->max < 0.0) ? (int)yAxis->max : (int)yAxis->max+1;

  /* Determine which resolution GMT database to use.
   */
  scale = sqrt((xAxis->max - xAxis->min) * (yAxis->max - yAxis->min));
  if (scale > 60)	strcpy(scale_str, "-Dc"); else
  if (scale > 40)	strcpy(scale_str, "-Dl"); else
  if (scale > 20)	strcpy(scale_str, "-Di"); else
  if (scale > 10)	strcpy(scale_str, "-Dh"); else
			strcpy(scale_str, "-Df");

  if (scale > 40)	strcpy(river_str, "");	else
  if (scale > 20)	strcpy(river_str, " -I1"); else
  if (scale > 10)	strcpy(river_str, " -Ir"); else
			strcpy(river_str, " -Ia");

  /* GMT5 only supports one of borders, shores, or rivers in in one command.
   * So now we have to string multiple pscoast commands together.
   */
  sprintf(command, "%s/bin/gmt pscoast -R%d/%d/%d/%d -M -Jx1d %s",
	gmt_path, xMin, xMax, yMin, yMax, scale_str);

  sprintf(buf, "(%s -Na; %s -W;", command, command);
  if (strlen(river_str) > 0)
  {
    strcat(buf, command);
    strcat(buf, river_str);
  }

  strcat(buf, ")");

}	/* END CREATECOASTCOMMAND */

/* -------------------------------------------------------------------- */
static void setColor(PLOT_INFO * plot, char str[], FILE *fp)
{
  if (buffer[0] == '>')
  {
    int color_idx = 0;	// Default black.

    if (strstr(str, "Border"))
      color_idx = 13;	// grey

    if (strstr(str, "River"))
      color_idx = 9;	//light blue

    if (strstr(str, "Shore"))
      color_idx = 2;	// blue

    if (fp)
    {
      float *rgb = GetColorRGB_PS(color_idx);
      fprintf(fp, "stroke %f %f %f setrgbcolor\n", rgb[0],rgb[1],rgb[2]);
    }
    else
      XSetForeground(plot->dpy, plot->gc, GetColor(color_idx));
  } 
}	/* END SETCOLOR */

/* END GEOPOLMAP.C */
