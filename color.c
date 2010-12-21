/*
-------------------------------------------------------------------------
OBJECT NAME:	color.c

FULL NAME:	Color values.

ENTRY POINTS:	InitializeColors()
		ResetColors()
		GetColor()
		NextColor()
		CurrentColor()
		SetColorNames()
		GetColorName()

STATIC FNS:	none

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2006
-------------------------------------------------------------------------
*/

#include "define.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdint.h>

#ifdef PNG
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#endif


#define BLACK		0
#define BACKGROUND	8


struct colorinfo_struct {
	char		name[20];
	float		ps_rgb[3];	/* For PostScript	*/
	uint16_t	x_rgb[3];	/* For X (save PNG)	*/
	uint32_t	pixel;
	uint32_t	cpixel;		/* Xserver byte order of 'pixel' */
} colorInfo[] = {
	{ "Black",	{ 0.0, 0.0, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "red",	{ 1.0, 0.0, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "blue",	{ 0.0, 0.0, 1.0 },	{ 0, 0, 0}, 0, 0 },
	{ "green",	{ 0.0, 1.0, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "yellow",	{ 1.0, 1.0, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "maroon",	{ 0.69, 0.1882, 0.3765 },	{ 0, 0, 0}, 0, 0 },
	{ "violet",	{ 0.9333, 0.5098, 0.9333 },	{ 0, 0, 0}, 0, 0 },
	{ "orange",	{ 1.0, 0.6471, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "purple",	{ 0.6275, 0.1255, 0.9412 }, { 0, 0, 0}, 0, 0 },
	{ "light blue",	{ 0.6784, 0.8471, 0.902 },	{ 0, 0, 0}, 0, 0 },
	{ "bisque",	{ 1.0, 0.8941, 0.7686 },	{ 0, 0, 0}, 0, 0 },
	{ "dark green",	{ 0.0, 0.3922, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "grey",	{ 0.0, 0.0, 0.0 },	{ 0, 0, 0}, 0, 0 },
	{ "darkgrey",	{ 0.1, 0.1, 0.1 },	{ 0, 0, 0}, 0, 0 },
	{ "",		{ 0.0, 0.0, 0.0 },	{ 0, 0, 0}, 0, 0 } };

static int		colorIndex, numberColors, saveColor;

#ifdef PNG
/* -------------------------------------------------------------------- */
void CheckByteSwap(XImage *image)
{
  int	i;
  bool	prog_byte_order;
  unsigned long	x;

  static bool firstTime = True;

  if (firstTime)	/* Only swap the colors on the first call. */
    firstTime = False;
  else
    return;


  prog_byte_order = (1 == ntohl(1)) ? MSBFirst : LSBFirst;
/* printf("%d %d %d\n", image->depth, prog_byte_order, image->byte_order); */

  for (i = 0; i < numberColors; ++i)
    {
    if (image->byte_order != prog_byte_order)
      {
      x = colorInfo[i].pixel;

      if (image->depth == 16)
        colorInfo[i].cpixel = ((x<<8)&0xff00) | ((x>>8)&0xff);
      else
        colorInfo[i].cpixel = ((((x<<8)&0xff00) | ((x>>8)&0xff)) << 16) |
			(((x>>8)&0xff00) | ((x>>24)&0xff)) ;
      }

    if (image->byte_order == LSBFirst)
      {
      x = colorInfo[i].x_rgb[0];
      colorInfo[i].x_rgb[0] = ((x<<8)&0xff00) | ((x>>8)&0xff);
      x = colorInfo[i].x_rgb[1];
      colorInfo[i].x_rgb[1] = ((x<<8)&0xff00) | ((x>>8)&0xff);
      x = colorInfo[i].x_rgb[2];
      colorInfo[i].x_rgb[2] = ((x<<8)&0xff00) | ((x>>8)&0xff);
      }
    }

}

/* -------------------------------------------------------------------- */
int GetColorIndex(uint32_t pixel)
{
  int	i;

  /* Check background color first, since it will be the most common.
   */
  if (pixel == colorInfo[BACKGROUND].cpixel)
    return(BACKGROUND);

  for (i = 0; i < numberColors; ++i)
    if (pixel == colorInfo[i].cpixel)
      return(i);

  return(BACKGROUND);

}

#endif

/* -------------------------------------------------------------------- */
int NumberOfColors()
{
  return(numberColors);
}

/* -------------------------------------------------------------------- */
uint32_t GetColor(int indx)
{
  return(colorInfo[indx].pixel);
}

/* -------------------------------------------------------------------- */
void ResetColors()
{
  colorIndex = 0;
}
 
/* -------------------------------------------------------------------- */
uint32_t NextColor()
{
  return(GetColor(++colorIndex));
}

/* -------------------------------------------------------------------- */
uint32_t CurrentColor()
{
  return(GetColor(colorIndex));
}
 
/* -------------------------------------------------------------------- */
void PushColor()
{
  saveColor = colorIndex;
}
 
/* -------------------------------------------------------------------- */
void PopColor()
{
  colorIndex = saveColor;
}
 
/* -------------------------------------------------------------------- */
float *GetColorRGB_PS(int indx)
{
  return(colorInfo[indx].ps_rgb);
}

/* -------------------------------------------------------------------- */
float *NextColorRGB_PS()
{
  return(GetColorRGB_PS(++colorIndex));
}

/* -------------------------------------------------------------------- */
float *CurrentColorRGB_PS()
{
  return(GetColorRGB_PS(colorIndex));
}
 
/* -------------------------------------------------------------------- */
uint16_t *GetColorRGB_X(int indx)
{
  return(colorInfo[indx].x_rgb);
}

/* -------------------------------------------------------------------- */
uint16_t *NextColorRGB_X()
{
  return(GetColorRGB_X(++colorIndex));
}

/* -------------------------------------------------------------------- */
uint16_t *CurrentColorRGB_X()
{
  return(GetColorRGB_X(colorIndex));
}

/* -------------------------------------------------------------------- */
void InitializeColors(PLOT_INFO *plot)
{
  int		i = 5, defaultDepth;
  Colormap	defaultCMap;
  int		screenNum;
  XVisualInfo	visInfo;
  XColor	screenDef, exactDef;

  Color = True;

  screenNum = XScreenNumberOfScreen(XtScreen(plot->canvas));
  defaultDepth = DefaultDepth(plot->dpy, screenNum);

  if (defaultDepth == 1)
    {
    Color = False;
    return;
    }

  while (!XMatchVisualInfo(plot->dpy, screenNum, defaultDepth, i, &visInfo))
    --i;

  if (i < StaticColor)
    {
    Color = False;
    return;
    }


  /* OK, we have a color screen.
   */
  defaultCMap = DefaultColormap(plot->dpy, screenNum);

  for (numberColors = 0; colorInfo[numberColors].name[0]; ++numberColors)
    {
    if (!XAllocNamedColor(plot->dpy, defaultCMap,
		colorInfo[numberColors].name, &screenDef, &exactDef))
      {
      fprintf(stderr, "color.c: Can't allocate color, reverting to B&W.\n");
      Color = False;
      return;
      }

    colorInfo[numberColors].pixel = exactDef.pixel;

#ifdef PNG
    XSetForeground(plot->dpy, plot->gc, exactDef.pixel);
    XDrawPoint(plot->dpy, plot->win, plot->gc, numberColors, 0);

    colorInfo[numberColors].cpixel = screenDef.pixel;
    colorInfo[numberColors].x_rgb[0] = screenDef.red;
    colorInfo[numberColors].x_rgb[1] = screenDef.green;
    colorInfo[numberColors].x_rgb[2] = screenDef.blue;

#endif
    colorInfo[numberColors].ps_rgb[0] = (float)exactDef.red / 65535;
    colorInfo[numberColors].ps_rgb[1] = (float)exactDef.green / 65535;
    colorInfo[numberColors].ps_rgb[2] = (float)exactDef.blue / 65535;
    }


#ifdef PNG
{
  /* Hack to fix the Exceed bug.  Exceed returns exactDef for both exactDef
   * and screenDef, so write each color (above), and read them back here.
   */
  XImage *image = XGetImage(plot->dpy, plot->win, 0, 0, 20, 1,
        (unsigned long)AllPlanes, ZPixmap);

  for (i = 0; i < numberColors; ++i)
    {
    if (image->depth == 16)
      colorInfo[i].cpixel = ((uint16_t *)image->data)[i];
    if (image->depth > 16)
      colorInfo[i].cpixel = ((uint32_t *)image->data)[i];
    }

}
#endif

}	/* END INITIALIZECOLORS */

/* -------------------------------------------------------------------- */
void SetColorNames(char *str)
{
  int	i;
  char	*p;

  p = strtok(str, " \t,");

  for (i = 1; colorInfo[i+1].name[0]; ++i)
    {
    strcpy(colorInfo[i].name, p);

    if ((p = strtok(NULL, " \t,\n")) == NULL)
      break;
    }

}	/* END SETCOLORNAMES */

/* -------------------------------------------------------------------- */
char *GetColorName(int indx)
{
  return(colorInfo[indx].name);

}
/* END COLOR.C */
