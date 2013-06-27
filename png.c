/*
-------------------------------------------------------------------------
OBJECT NAME:	png.c

FULL NAME:	PNG callback

ENTRY POINTS:	SavePNG()
		savePNG()
		confirmPNG()

STATIC FNS:	_SavePNG()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2001
-------------------------------------------------------------------------
*/

#ifdef PNG

#include "define.h"
#include <unistd.h>
#include <png.h>

#include <X11/Xlib.h>


int	NumberOfColors(), GetColorIndex(uint32_t);
void	CheckByteSwap(XImage *image);
unsigned short	*GetColorRGB_X(int);
static	void _SavePNG(char file_name[], XImage *image);

static char	pngDirectory[256] = "*.png";
static int	saving = MAIN_CANVAS;

/* -------------------------------------------------------------------- */
void savePNG(Widget w, XtPointer client, XtPointer call)
{
  XImage	*image;
  PLOT_INFO	*plot = 0;
  char		*p;

  strcpy(pngDirectory, outFile);
  if ((p = strrchr(pngDirectory, '/')))
    strcpy(p+1, "*.png");

  switch (saving)
    {
    case SPEC_CANVAS:
      plot = &specPlot;

      image = XGetImage(plot->dpy, plot->win, 0, 0,
	plot->x.windowWidth, plot->x.windowHeight,
	(unsigned long)AllPlanes, ZPixmap);
      break;

    case DIFF_CANVAS:
      plot = &diffPlot;

      image = XGetImage(plot->dpy, plot->win, 0, 0,
	plot->x.windowWidth, plot->x.windowHeight,
	(unsigned long)AllPlanes, ZPixmap);
      break;

    default:
      switch (PlotType)
        {
        case TIME_SERIES:
          plot = &mainPlot[0];
          break;

        case XY_PLOT:
          plot = &xyyPlot[0];
          break;

        case XYZ_PLOT:
          plot = &xyzPlot;
          break;
        }

      image = XGetImage(plot->dpy, plot->win, 0, 20,
	plot->x.windowWidth, plot->x.windowHeight-20,
	(unsigned long)AllPlanes, ZPixmap);
    }


  CheckByteSwap(image);
  _SavePNG(outFile, image);

  XDestroyImage(image);

/*	To save PNG via Imlib.
  idata = Imlib_init(plot->dpy);
  image = Imlib_create_image_from_drawable(idata, plot->win, 0, 0, 0,
			plot->x.windowWidth, plot->x.windowHeight);

  Imlib_save_image(idata, image, outFile, (ImlibSaveInfo *)NULL);
  Imlib_kill_image(idata, image);
*/
}

/* -------------------------------------------------------------------- */
void confirmPNG(Widget w, XtPointer client, XtPointer call)
{
  FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
  ExtractFileName(((XmFileSelectionBoxCallbackStruct *)call)->value, &outFile);

  if (strstr(outFile, ".png") == NULL)
    strcat(outFile, ".png");

  if (access(outFile, F_OK) == 0)
    {
    sprintf(buffer, "Overwrite file %s", outFile);
    WarnUser(buffer, savePNG, NULL);
    }
  else
    savePNG((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

}

/* -------------------------------------------------------------------- */
void SavePNG(Widget w, XtPointer client, XtPointer call)
{
  saving = MAIN_CANVAS;
  QueryFile("Enter filename to write PNG to:", pngDirectory, confirmPNG);

}

/* -------------------------------------------------------------------- */
void SavePNGspec(Widget w, XtPointer client, XtPointer call)
{
  saving = SPEC_CANVAS;
  QueryFile("Enter filename to write PNG to:", pngDirectory, confirmPNG);

}

/* -------------------------------------------------------------------- */
void SavePNGdiff(Widget w, XtPointer client, XtPointer call)
{
  saving = DIFF_CANVAS;
  QueryFile("Enter filename to write PNG to:", pngDirectory, confirmPNG);

}

/* -------------------------------------------------------------------- */
static void _SavePNG(char file_name[], XImage *image)
{
  int		i, j;
  FILE		*outFP;
  unsigned short *s;
  png_structp	png_ptr;
  png_infop	info_ptr;
  png_bytep	row_pointers[2000];

  png_color	*palette;


  if ((outFP = fopen(file_name, "wb")) == NULL)
    return;


  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			(png_voidp)NULL, NULL, NULL);

  if (!png_ptr) {
    fclose(outFP);
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose(outFP);
    return;
    }


#ifdef PNG15
  if (setjmp(png_jmpbuf(png_ptr))) {
#else
   if (setjmp(png_ptr->jmpbuf)) {
#endif
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outFP);
    return;
    }


  png_init_io(png_ptr, outFP);


  png_set_IHDR(png_ptr, info_ptr,
	image->width, image->height, 8,
	PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
	PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


  if ((palette = new png_color[NumberOfColors()]) == NULL)
      return;

  for (i = 0; i < NumberOfColors(); i++) {
    s = GetColorRGB_X(i);
    palette[i].red   = s[0];
    palette[i].green = s[1];
    palette[i].blue  = s[2];
    }

  /* Force background to white.
   */
  palette[i-1].red = palette[i-1].green = palette[i-1].blue = 0xffff;

  png_set_PLTE(png_ptr, info_ptr, palette, NumberOfColors());


  /* Write header info to PNG file. */
  png_write_info(png_ptr, info_ptr);

  for (i = 0; i < image->height; ++i)
    {
    char *p = &(image->data[i * image->bytes_per_line]);

    row_pointers[i] = new png_byte[image->width];

    switch (image->depth)
      {
      case 8:
        for (j = 0; j < image->width; ++j)
          row_pointers[i][j] = (png_byte)GetColorIndex(((char *)p)[j]);
        break;

      case 16:
        for (j = 0; j < image->width; ++j)
          row_pointers[i][j] = (png_byte)GetColorIndex(((uint16_t *)p)[j]);

        break;

      case 24: case 32:
        for (j = 0; j < image->width; ++j)
          row_pointers[i][j] = (png_byte)GetColorIndex(((uint32_t *)p)[j]);

        break;
      }
    }


  /* Write the data! */
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  for (i = 0; i < image->height; ++i)
    delete [] row_pointers[i];

  delete [] palette;
  fclose(outFP);

}
#endif

/* END PNG.C */


