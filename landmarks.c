/*
-------------------------------------------------------------------------
OBJECT NAME:	landmarks.c

FULL NAME:	Plot WINDS landmarks file.

ENTRY POINTS:	ToggleLandMarks()
		PlotLandMarksXY()
		PlotLandMarks3D()

STATIC FNS:	none.

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2010
-------------------------------------------------------------------------
*/

#include "define.h"

static struct
  {
  float	lat, lon;
  char	*tag;
  } landMark[128];

static size_t nMarks = 0;


/* -------------------------------------------------------------------- */
void ClearLandmarks()
{
  for (size_t i = 0; i < nMarks; ++i)
    delete landMark[i].tag;

  nMarks = 0;
}

/* -------------------------------------------------------------------- */
void ToggleLandMarks(Widget w, XtPointer client, XtPointer call)
{
  LandMarks = !LandMarks;

  if (Interactive)
    DrawMainWindow();
 
}	/* END TOGGLELANDMARKS */

/* -------------------------------------------------------------------- */
static void plotLMx(PLOT_INFO *plot, int x, int y, int i)
{
  XDrawLine(plot->dpy, plot->win, plot->gc, x-3, y-3, x+4, y+4);
  XDrawLine(plot->dpy, plot->win, plot->gc, x-3, y+3, x+4, y-4);
  XDrawString(plot->dpy, plot->win, plot->gc, x+6, y+5, landMark[i].tag,
              strlen(landMark[i].tag));
}

/* -------------------------------------------------------------------- */
static void plotLMps(FILE *fp, int x, int y, int i)
{
  fprintf(fp, "%d %d m\n", x-10, y+10);
  fprintf(fp, "%d %d l\n", x+10, y-10);

  fprintf(fp, "%d %d m\n", x-10, y-10);
  fprintf(fp, "%d %d l\n", x+10, y+10);

  fprintf(fp, "%d %d m\n", x+12, y-10);
  fprintf(fp, "(%s) s\n", landMark[i].tag);
}

/* -------------------------------------------------------------------- */
void PlotLandMarksXY(PLOT_INFO *plot, FILE *fp)
{
  int	x, y;
  float	xScale, yScale, xMin, yMin, xMax, yMax;
  struct plot_offset *plotInfo;

  if (!LandMarks)
    return;

  xMin = plot->Xaxis.min;
  xMax = plot->Xaxis.max;
  yMin = plot->Yaxis[0].min;
  yMax = plot->Yaxis[0].max;

  plotInfo = (fp) ? &plot->ps : &plot->x;

  xScale = (NR_TYPE)plotInfo->HD / (xMax - xMin);
  yScale = (NR_TYPE)plotInfo->VD / (yMax - yMin);

  for (size_t i = 0; i < nMarks; ++i)
    {
    if (landMark[i].lat < yMin || landMark[i].lat > yMax ||
        landMark[i].lon < xMin || landMark[i].lon > xMax)
      continue;

    if (fp)	/* PostScript */
      {
      x = (int)(xScale * (landMark[i].lon - xMin));
      y = (int)(yScale * (landMark[i].lat - yMin));

      plotLMps(fp, x, y, i);
      }
    else /* X window */
      {
      x = (int)(plotInfo->LV + (xScale * (landMark[i].lon - xMin)));
      y = (int)(plotInfo->BH - (yScale * (landMark[i].lat - yMin)));

      plotLMx(plot, x, y, i);
      }
    }
}	/* END PLOTLANDMARKSXY */

/* -------------------------------------------------------------------- */
void PlotLandMarks3D(
  PLOT_INFO *plot,
  int ZD,
  float cosFac, float sinFac,
  FILE *fp)			/* PostScript or Xwin	*/
{
  int   x, y;
  float xScale, yScale, zScale, xMin, zMin, xMax, zMax;
  struct plot_offset *plotInfo;

  if (!LandMarks)
    return;

  xMin = plot->Xaxis.min;
  xMax = plot->Xaxis.max;
  zMin = plot->Zaxis.min;
  zMax = plot->Zaxis.max;

  plotInfo = (fp) ? &plot->ps : &plot->x;
 
  xScale = (float)plotInfo->HD / (xMax - xMin);
  yScale = (float)plotInfo->VD / (plot->Yaxis[0].max - plot->Yaxis[0].min);
  zScale = (float)ZD / (zMax - zMin);

  for (size_t i = 0; i < nMarks; ++i)
    {
    if (landMark[i].lat < zMin || landMark[i].lat > zMax ||
        landMark[i].lon < xMin || landMark[i].lon > xMax)
      continue;
 
    if (fp)	/* PostScript */
      {
      x = (int)(xScale * (landMark[i].lon - xMin));
      y = (int)(yScale * (0.0 - plot->Yaxis[0].min));

      x += (int)(cosFac * (zScale * (landMark[i].lat - zMin)));
      y += (int)(sinFac * (zScale * (landMark[i].lat - zMin)));

      plotLMps(fp, x, y, i);
      }
    else /* X window */
      {
      x = (int)(plotInfo->LV + (xScale * (landMark[i].lon - xMin)));
      y = (int)(plotInfo->BH - (yScale * (0.0 - plot->Yaxis[0].min)));

      x += (int)(cosFac * (zScale * (landMark[i].lat - zMin)));
      y -= (int)(sinFac * (zScale * (landMark[i].lat - zMin)));
 
      plotLMx(plot, x, y, i);
      }
    }
}   /* END PLOTLANDMARKS3D */

/* -------------------------------------------------------------------- */
static void addLandmark(const char str[])
{
  char tempTag[64];

  sscanf(str, "%f %f %s\n", &landMark[nMarks].lat, &landMark[nMarks].lon,
				tempTag);
  landMark[nMarks].tag = new char[strlen(tempTag)+1];
  strcpy(landMark[nMarks].tag, tempTag);
  ++nMarks;
}

/* -------------------------------------------------------------------- */
void
setLandmarks(const std::string lm_str)
{
  std::string rest = lm_str;
  std::string current;

  while (rest.size() > 0)
  {
    size_t pos = rest.find(',');

    current = rest.substr(0, pos);
    if (pos == std::string::npos)
      rest.clear();
    else
      rest = rest.substr(pos+1);

    addLandmark(current.c_str());
  }
}	/* END SETLANDMARKS */

/* END LANDMARKS.C */
