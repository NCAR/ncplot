/*
-------------------------------------------------------------------------
OBJECT NAME:	barbs.c

FULL NAME:	Wind Barbs.

ENTRY POINTS:	PlotWindBarbs()
		PrintWindBarbs()
		ToggleWindBarbs()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"


static const char wbTitle[] = "Wind Vectors";
static int	timeInterval, average;

/* -------------------------------------------------------------------- */
void ToggleWindBarbs(Widget w, XtPointer client, XtPointer call)
{
  WindBarbs = !WindBarbs;
 
  if (WindBarbs)
    {
    std::string GetUI(), GetVI(), GetTI();

    if (LoadVariable(&ui, GetUI()) == ERR)
      fprintf(stderr, "Can't find variable %s\n", GetUI().c_str());

    if (LoadVariable(&vi, GetVI()) == ERR)
      fprintf(stderr, "Can't find variable %s\n", GetUI().c_str());

    timeInterval = atoi(GetTI().c_str());

    average = isAverage();
    }
 
  if (Interactive)
    DrawMainWindow();
 
}	/* END TOGGLEWINDBARBS */

/* -------------------------------------------------------------------- */
void PlotWindBarbs(PLOT_INFO *plot, FILE *fp)
{
  int	i, j, x1, y1, x2, y2, nPts;
  float	xScale, yScale, barbScale = 0, ui_sum, vi_sum, datum, yMin, yMax;
  struct plot_offset *plotInfo;

  if (plot->plotType != XY_PLOT)
    return;

  plotInfo = (fp) ? &plot->ps : &plot->x;

  if (plot->Yaxis[0].logScale) {
    yMin = log10(plot->Yaxis[0].min);
    yMax = log10(plot->Yaxis[0].max);
    }
  else {
    yMin = plot->Yaxis[0].min;
    yMax = plot->Yaxis[0].max;
    }

  xScale = (NR_TYPE)plotInfo->HD / (plot->Xaxis.max - plot->Xaxis.min);
  yScale = (NR_TYPE)plotInfo->VD / (yMax - yMin);

  nPts = std::max(xyXset[0].nPoints, xyYset[0].nPoints);

  if (xyXset[0].varInfo->name.find_first_of("LON") == std::string::npos)
    xScale = 0;

  for (i = 0; i < nPts; ++i)
    {
    if ((i % (timeInterval*2)) - timeInterval)
      continue;

    if (average)
      {
      ui_sum = vi_sum = 0.0;

      for (j = i - timeInterval; j < i + timeInterval; ++j)
        {
        ui_sum += ui.data[i];
        vi_sum += vi.data[i];
        }

      ui_sum /= timeInterval*2;
      vi_sum /= timeInterval*2;
      }
    else /* Instantaneous */
      {
      ui_sum = ui.data[i];
      vi_sum = vi.data[i];
      }

    if (fp)
      {
      barbScale = 75.0 / 10.0;
      x1 = (int)(xScale * (xyXset[0].data[i] - plot->Xaxis.min));
      x2 = (int)(x1 + (barbScale * ui_sum));

      if (plot->Yaxis[0].logScale)
        datum = log10(xyYset[0].data[i]);
      else
        datum = xyYset[0].data[i];

      if (plot->Yaxis[0].invertAxis)
        y1 = (int)(yScale * (yMax - datum));
      else
        y1 = (int)(yScale * (datum - yMin));

      y2 = (int)(y1 + (barbScale * vi_sum));

      fprintf(fp, moveto, x1, y1);
      fprintf(fp, lineto, x2, y2);
      }
    else
      {
      barbScale = 25.0 / 10.0;
      x1 = (int)(plotInfo->LV + (xScale * (xyXset[0].data[i] - plot->Xaxis.min)));
      x2 = (int)(x1 + (barbScale * ui_sum));

      if (plot->Yaxis[0].logScale)
        datum = log10(xyYset[0].data[i]);
      else
        datum = xyYset[0].data[i];

      if (plot->Yaxis[0].invertAxis)
        y1 = (int)(plotInfo->TH + (yScale * (datum - yMin)));
      else
        y1 = (int)(plotInfo->BH - (yScale * (datum - yMin)));

      y2 = (int)(y1 - (barbScale * vi_sum));

      XDrawLine(plot->dpy, plot->win, plot->gc, x1, y1, x2, y2);
      }
    }

  x1 = plotInfo->xLegendText - 10;
  x2 = (int)(x1 + (barbScale * 10.0));
  strcpy(buffer, "10 m/s");

  if (fp)
    {
    y1 = y2 = yLegendPS(plot, CurrentDataSet+3);
    fprintf(fp, moveto, x1, y1);
    fprintf(fp, lineto, x2, y2);

    fprintf(fp, moveto, x2+20, y1);
    fprintf(fp, show, buffer);

    fprintf(fp, moveto, plotInfo->xLegendText,
		yLegendPS(plot, CurrentDataSet+4));
    fprintf(fp, show, wbTitle);
    }
  else
    {
    y1 = y2 = yLegendX(plot, CurrentDataSet+1);
    XDrawLine(plot->dpy, plot->win, plot->gc, x1, y1, x2, y2);

    XDrawString(plot->dpy, plot->win, plot->gc, x2+10, y1 + 5,
                buffer, strlen(buffer));

    XDrawString(plot->dpy, plot->win, plot->gc, plotInfo->xLegendText,
                yLegendX(plot, CurrentDataSet+2), wbTitle, strlen(wbTitle));
    }

}	/* END PLOTWINDBARBS */

/* END BARBS.C */
