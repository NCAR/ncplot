/*
-------------------------------------------------------------------------
OBJECT NAME:	X.c

FULL NAME:	X-window ploting routines

ENTRY POINTS:	xTicsLabelsX()
		yTicsLabelsX()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	plotX.c, diffX.c, specX.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2001
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ps.h"


/* -------------------------------------------------------------------- */
void yLogTicsLabelsX(PLOT_INFO *plot, XFontStruct *fontInfo, int scale, bool labels)
{
  int		i, xoffset, yoffset, len, ticlen;
  double	yScale, incrementer, value, yMin;
  struct axisInfo	*yAxis = &plot->Yaxis[scale];

  yMin		= log10(yAxis->min);
  yScale	= (double)plot->x.VD / (log10(yAxis->max) - yMin);
  incrementer	= pow(10.0, floor(yMin));
  value		= (int)(yAxis->min / incrementer) * incrementer;

  if (labels)	// Generate first label outside of mainloop.
    {
    if (log10(incrementer) == floor(yMin))
      len = MakeLogTicLabel(buffer, (int)rint(yMin));
    else
      {
      sprintf(buffer, "%g", yAxis->min);
      len = strlen(buffer);
      }

    if (scale == LEFT_SIDE)
      xoffset = plot->x.LV - plot->x.yTicLabelOffset -
			XTextWidth(fontInfo, buffer, len);
    else
      xoffset = plot->x.RV + plot->x.yTicLabelOffset;

    yoffset = yAxis->invertAxis ? plot->x.TH : plot->x.BH;
    XDrawString(plot->dpy,plot->win,plot->gc, xoffset,yoffset+3, buffer,len);
    }


  for (i = 0; value < yAxis->max; ++i)
    {
    value += incrementer;

    if (yAxis->invertAxis)
      yoffset = (int)(plot->x.TH + (log10(value) - yMin) * yScale + 0.5);
    else
      yoffset = (int)(plot->x.BH - (log10(value) - yMin) * yScale + 0.5);

    if ((value > 0.999 && value < 1.2) || log10(incrementer) != floor(log10(value)))
      {
      ticlen = plot->grid ? plot->x.HD : plot->x.ticLength;
      incrementer *= 10;

      if (labels)
        {
        len = MakeLogTicLabel(buffer, (int)rint(log10(value)));

        if (scale == LEFT_SIDE)
          xoffset = plot->x.LV - plot->x.yTicLabelOffset -
			XTextWidth(fontInfo, buffer, len);
        else
          xoffset = plot->x.RV + plot->x.yTicLabelOffset;

        XDrawString(plot->dpy,plot->win,plot->gc,xoffset,yoffset+3,buffer,len);
        }
      }
    else
      {
      ticlen = plot->grid ? plot->x.HD : plot->x.ticLength * 2 / 3;
      }

    XDrawLine(plot->dpy, plot->win, plot->gc,
              plot->x.LV, yoffset, plot->x.LV + ticlen, yoffset);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      XDrawLine(plot->dpy, plot->win, plot->gc,
                plot->x.RV - ticlen, yoffset, plot->x.RV, yoffset);
    }

/*
  if (labels)	// Generate last label outside of mainloop.
    {
    if (log10(incrementer) == floor(log10(yAxis->max)))
      len = MakeLogTicLabel(buffer, (int)rint(log10(yAxis->max)));
    else
      {
      sprintf(buffer, "%g", yAxis->max);
      len = strlen(buffer);
      }

    if (scale == LEFT_SIDE)
      xoffset = plot->x.LV - plot->x.yTicLabelOffset -
			XTextWidth(fontInfo, buffer, len);
    else
      xoffset = plot->x.RV + plot->x.yTicLabelOffset;

    yoffset = yAxis->invertAxis ? plot->x.BH : plot->x.TH;
    XDrawString(plot->dpy,plot->win,plot->gc, xoffset,yoffset+3, buffer,len);
    }
*/
}	/* END YTICSLABELSX */

/* -------------------------------------------------------------------- */
void xLogTicsLabelsX(PLOT_INFO *plot, XFontStruct *fontInfo, bool labels)
{
  int		i, xoffset, yoffset, len, ticlen;
  double	xScale, incrementer, value, xMin;
  struct axisInfo	*xAxis = &plot->Xaxis;

  xMin		= log10(xAxis->min);
  yoffset	= plot->x.BH + plot->x.xTicLabelOffset;
  xScale        = plot->x.HD / (log10(xAxis->max) - xMin);
  incrementer   = pow(10.0, floor(xMin));
  value         = (int)(xAxis->min / incrementer) * incrementer;

  if (labels)   // Generate first label outside of mainloop.
    {
    if (log10(incrementer) == floor(xMin))
      len = MakeLogTicLabel(buffer, (int)rint(xMin));
    else
      {
      sprintf(buffer, "%g", xAxis->min);
      len = strlen(buffer);
      }

    if (xAxis->invertAxis)
      xoffset = plot->x.RV - (XTextWidth(fontInfo, buffer, len)>>1);
    else
      xoffset = plot->x.LV - (XTextWidth(fontInfo, buffer, len)>>1);

    XDrawString(plot->dpy,plot->win,plot->gc, xoffset,yoffset, buffer,len);
    }


  for (i = 0; value < xAxis->max; ++i)
    {
    value += incrementer;

    if (xAxis->invertAxis)
      xoffset = (int)(plot->x.RV - (log10(value) - xMin) * xScale + 0.5);
    else
      xoffset = (int)(plot->x.LV + (log10(value) - xMin) * xScale + 0.5);

    if ((value > 0.95 && value < 1.05) || log10(incrementer) != floor(log10(value)))
      {
      ticlen = plot->grid ? plot->x.VD : plot->x.ticLength;
      incrementer *= 10;

      if (labels)
        {
        len = MakeLogTicLabel(buffer, (int)rint(log10(value)));

        XDrawString(plot->dpy, plot->win, plot->gc, xoffset -
		(XTextWidth(fontInfo, buffer, len)>>1), yoffset, buffer, len);
        }
      }
    else
      {
      ticlen = plot->grid ? plot->x.VD : plot->x.ticLength * 2 / 3;
      }

    XDrawLine(plot->dpy, plot->win, plot->gc,
		xoffset, plot->x.BH, xoffset, plot->x.BH - ticlen);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      XDrawLine(plot->dpy, plot->win, plot->gc,
		xoffset, plot->x.TH, xoffset, plot->x.TH + ticlen);
    }


  if (labels)   // Generate first label outside of mainloop.
    {
    if (log10(incrementer) == floor(log10(xAxis->max)))
      len = MakeLogTicLabel(buffer, (int)rint(log10(xAxis->max)));
    else
      {
      sprintf(buffer, "%g", xAxis->max);
      len = strlen(buffer);
      }

    if (xAxis->invertAxis)
      xoffset = plot->x.LV - (XTextWidth(fontInfo, buffer, len)>>1);
    else
      xoffset = plot->x.RV - (XTextWidth(fontInfo, buffer, len)>>1);

    XDrawString(plot->dpy,plot->win,plot->gc, xoffset,yoffset, buffer,len);
    }

}	/* END XTICSLABELSX */

/* -------------------------------------------------------------------- */
void yLogTicsLabelsPS(FILE *fp, PLOT_INFO *plot, int scale, bool labels)
{
  int		i, yoffset, len, ticlen;
  double	yScale, incrementer, value;
  struct axisInfo	*yAxis = &plot->Yaxis[scale];

  yScale	= plot->ps.VD / (log10(yAxis->max) - log10(yAxis->min));
  incrementer	= pow(10.0, floor(log10(yAxis->min)));
  value		= (int)(yAxis->min / incrementer) * incrementer;


  if (labels)	// Generate first label outside of mainloop.
    {
    if (log10(incrementer) == floor(log10(yAxis->min)))
      len = MakeLogTicLabel(buffer, (int)rint(log10(yAxis->min)));
    else
      {
      sprintf(buffer, "%g", yAxis->min);
      len = strlen(buffer);
      }

    if (yAxis->invertAxis)
      yoffset = plot->ps.VD;
    else
      yoffset = 0;

    if (scale == LEFT_SIDE)
      fprintf(fp, "%d (%s) stringwidth pop sub %d moveto (%s) show\n",
              plot->ps.yTicLabelOffset, buffer, yoffset-10, buffer);
    else
      fprintf(fp, "%d %d moveto (%s) show\n",
              plot->ps.HD - plot->ps.yTicLabelOffset, yoffset-10, buffer);
    }


  for (i = 0; value < yAxis->max; ++i)
    {
    value += incrementer;

    if (yAxis->invertAxis)
      yoffset = (int)(plot->ps.VD - (log10(value)-log10(yAxis->min)) * yScale);
    else
      yoffset = (int)((log10(value)-log10(yAxis->min)) * yScale);

    if ((value > 0.95 && value < 1.05) || log10(incrementer) != floor(log10(value)))
      {
      ticlen = plot->grid ? plot->ps.HD : plot->ps.ticLength;
      incrementer *= 10;

      if (labels)
        {
        len = MakeLogTicLabel(buffer, (int)rint(log10(value)));

        if (scale == LEFT_SIDE)
          fprintf(fp, "%d (%s) stringwidth pop sub %d moveto (%s) show\n",
              plot->ps.yTicLabelOffset, buffer, yoffset-10, buffer);
        else
          fprintf(fp, "%d %d moveto (%s) show\n",
              plot->ps.HD - plot->ps.yTicLabelOffset, yoffset-10, buffer);
        }
      }
    else
      {
      ticlen = plot->grid ? plot->ps.HD : plot->ps.ticLength * 2 / 3;
      }

      fprintf(fp, moveto, 0, yoffset);
      fprintf(fp, lineto, ticlen, yoffset);

      if (!plot->grid && plot->plotType != XYZ_PLOT)
        {
        fprintf(fp, moveto, plot->ps.HD - ticlen, yoffset);
        fprintf(fp, lineto, plot->ps.HD, yoffset);
        }
    }

  if (labels)	// Generate last label outside of mainloop.
    {
    if (log10(incrementer) == floor(log10(yAxis->max)))
      len = MakeLogTicLabel(buffer, (int)rint(log10(yAxis->max)));
    else
      {
      sprintf(buffer, "%g", yAxis->max);
      len = strlen(buffer);
      }

    if (yAxis->invertAxis)
      yoffset = 0;
    else
      yoffset = plot->ps.VD;

    if (scale == LEFT_SIDE)
      fprintf(fp, "%d (%s) stringwidth pop sub %d moveto (%s) show\n",
              plot->ps.yTicLabelOffset, buffer, yoffset-10, buffer);
    else
      fprintf(fp, "%d %d moveto (%s) show\n",
              plot->ps.HD - plot->ps.yTicLabelOffset, yoffset-10, buffer);
    }

}	/* END YTICSLABELSPS */

/* -------------------------------------------------------------------- */
void xLogTicsLabelsPS(FILE *fp, PLOT_INFO *plot, bool labels)
{
  int		i, xoffset, yoffset, len, ticlen;
  double	xScale, incrementer, value;
  struct axisInfo	*xAxis = &plot->Xaxis;

  yoffset	= plot->ps.xTicLabelOffset;
  xScale        = plot->ps.HD / (log10(xAxis->max) - log10(xAxis->min));
  incrementer   = pow(10.0, floor(log10(xAxis->min)));
  value         = (int)(xAxis->min / incrementer) * incrementer;

  if (labels)   // Generate first label outside of mainloop.
    {
    if (log10(incrementer) == floor(log10(xAxis->min)))
      len = MakeLogTicLabel(buffer, (int)rint(log10(xAxis->min)));
    else
      {
      sprintf(buffer, "%g", xAxis->min);
      len = strlen(buffer);
      }

    if (xAxis->invertAxis)
      xoffset = plot->ps.HD;
    else
      xoffset = 0;

    fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
		xoffset, buffer, plot->ps.xTicLabelOffset);
    fprintf(fp, show, buffer);
    }

  for (i = 0; value < xAxis->max; ++i)
    {
    value += incrementer;

    if (xAxis->invertAxis)
      xoffset = (int)(plot->ps.HD - (log10(value)-log10(xAxis->min)) * xScale);
    else
      xoffset = (int)((log10(value) - log10(xAxis->min)) * xScale);

    if ((value > 0.95 && value < 1.05) || log10(incrementer) != floor(log10(value)))
      {
      ticlen = plot->grid ? plot->ps.VD : plot->ps.ticLength;
      incrementer *= 10;

      if (labels)
        {
        len = MakeLogTicLabel(buffer, (int)rint(log10(value)));

        fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
    		xoffset, buffer, plot->ps.xTicLabelOffset);
        fprintf(fp, show, buffer);
        }
      }
    else
      {
      ticlen = plot->grid ? plot->ps.VD : plot->ps.ticLength * 2 / 3;
      }

    fprintf(fp, moveto, xoffset, 0);
    fprintf(fp, lineto, xoffset, ticlen);

    if (!plot->grid && plot->plotType != XYZ_PLOT)
      {
      fprintf(fp, moveto, xoffset, plot->ps.VD - plot->ps.ticLength);
      fprintf(fp, lineto, xoffset, plot->ps.VD);
      }
    }

  if (labels)   // Generate first label outside of mainloop.
    {
    if (log10(incrementer) == floor(log10(xAxis->max)))
      len = MakeLogTicLabel(buffer, (int)rint(log10(xAxis->max)));
    else
      {
      sprintf(buffer, "%g", xAxis->max);
      len = strlen(buffer);
      }

    if (xAxis->invertAxis)
      xoffset = 0;
    else
      xoffset = plot->ps.HD;

    fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
    		xoffset, buffer, plot->ps.xTicLabelOffset);
    fprintf(fp, show, buffer);
    }

}	/* END XTICSLABELSPS */
