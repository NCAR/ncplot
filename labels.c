/*
-------------------------------------------------------------------------
OBJECT NAME:	labels.c

FULL NAME:	Generate Various Types of Tic Mark Labels.

ENTRY POINTS:	MakeTimeTicLabel()
		MakeTicLabel()
		MakeLogTicLabel()
		MakeLegendLabel()
		SetXlabels()
		SetYlabels()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		
-------------------------------------------------------------------------
*/

#include "define.h"

/* -------------------------------------------------------------------- */
int MakeTicLabel(char buffer[], float diff, int majorTics, double value)
{
  int	idiff = (int)diff;

  if (diff < 0.01)
    sprintf(buffer, "%f", value);
  else
  if (diff < 0.1)
    sprintf(buffer, "%.4f", value);
  else
  if (diff < 1.0)
    sprintf(buffer, "%.3f", value);
  else
  if (diff == (float)idiff && (idiff % majorTics) == 0)
    sprintf(buffer, "%ld", (long)value);
  else
  if (diff < 10.0)
    sprintf(buffer, "%.2f", value);
  else
  if (diff < 40.0)
    sprintf(buffer, "%.1f", value);
  else
    sprintf(buffer, "%ld", (long)value);

  return(strlen(buffer));

}	/* END MAKETICLABEL */

/* -------------------------------------------------------------------- */
int MakeLogTicLabel(char buffer[], int value)
{
  sprintf(buffer, "10^%d", value);
  return(strlen(buffer));
}

/* -------------------------------------------------------------------- */
int MakeTimeTicLabel(char buffer[], int indx, int nTics)
{
  int	nsex, nh, nm, ns;

  if (nTics == 1)
    nsex = NumberSeconds;
  else
    nsex = indx * NumberSeconds / nTics;

  if (indx == 0)
    nh = nm = ns = 0;
  else
    {
    nh = nsex / 3600;
    nm = (nsex - (nh * 3600)) / 60;
    ns = nsex - (nh * 3600) - (nm * 60);
    }

  if ((ns += UserStartTime[2]) > 59) { ns -= 60; nm += 1; }
  if ((nm += UserStartTime[1]) > 59) { nm -= 60; nh += 1; }
  if ((nh += UserStartTime[0]) > 23) { nh -= 24; }

  if (UTCseconds)
    sprintf(buffer, "%d", nh * 3600 + nm * 60 + ns);
  else
    if (NumberSeconds > 1800)
      sprintf(buffer, "%02d:%02d", nh, nm);
    else
      sprintf(buffer, "%02d:%02d:%02d", nh, nm, ns);

  return(strlen(buffer));

}	/* END MAKETIMETICLABEL */

/* -------------------------------------------------------------------- */
int MakeLegendLabel(char buf[], DATASET_INFO *set)
{
  if (Statistics)
    {
    sprintf(buf, "%s (%s), %d s/sec",
                set->varInfo->name.c_str(),
                set->stats.units.c_str(),
                set->varInfo->OutputRate);

    if (strlen(buf) < 30)
      memset(&buf[strlen(buf)], ' ', 30-strlen(buf));
    sprintf(&buf[30], "%11.2f%11.2f%11.2f%11.2f",
                set->stats.mean, set->stats.sigma,
                set->stats.min, set->stats.max);
    }
  else
    {
    sprintf(buf, "%s (%s)",
                set->varInfo->name.c_str(), set->stats.units.c_str());
    }

  return(strlen(buf));

}	/* END MAKEXYLEGENDLABEL */

/* -------------------------------------------------------------------- */
void SetXlabels(PLOT_INFO *plot, DATASET_INFO *set, size_t nSets)
{
  /* Clean out unused X labels, and set to first active variable.
   */
  for (size_t i = 0; i < MAX_PANELS; ++i)
    plot[i].Xaxis.label = "";
 
  for (int i = nSets-1; i >= 0; --i)
    if (PlotType == TIME_SERIES)
      plot[set[i].panelIndex].Xaxis.label = set[i].stats.units.c_str();
    else
      {
      sprintf(buffer, "%s (%s)", set[i].varInfo->name.c_str(), set[i].stats.units.c_str());
      plot[set[i].panelIndex].Xaxis.label = buffer;
      }
 
}       /* END SETXLABELS */
 
/* -------------------------------------------------------------------- */
void SetYlabels(PLOT_INFO *plot, DATASET_INFO *set, size_t nSets)
{
  /* Clean out unused Y labels, and set to first active variable.
   */
  for (size_t i = 0; i < MAX_PANELS; ++i)
    for (int j = 0; j < 2; ++j)
      plot[i].Yaxis[j].label = "";
 
  for (int i = nSets-1; i >= 0; --i)
    if (PlotType == TIME_SERIES)
      plot[set[i].panelIndex].Yaxis[set[i].scaleLocation].label =
                set[i].stats.units.c_str();
    else
      if (allLabels || set[i].panelIndex == 0)
        {
        sprintf(buffer, "%s (%s)", set[i].varInfo->name.c_str(), set[i].stats.units.c_str());
        plot[set[i].panelIndex].Yaxis[set[i].scaleLocation].label = buffer;
        }
 
}       /* END SETYLABELS */

/* LABELS.C */
