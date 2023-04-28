


#ifndef lint
static char *rcsid_Gadget_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Gadget.c,v 1.2 89/02/07 20:04:53 jkh Exp $";
#endif  lint

/*
 *
 * Copyright 1987, 1988 by Ardent Computer Corporation, Sunnyvale, Ca.
 *
 * Copyright 1987 by Jordan Hubbard.
 *
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Ardent Computer
 * Corporation or Jordan Hubbard not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.
 *
 */

/*
 * MODIFICATION HISTORY
 *
 * 001 -- Jortan Hubbard, Ardent Computer.
 * Gadget boxes in title bars.
 * 1.2 -- Jordan Hubbard, Ardent Computer.
 *  GadgetBorders used in placement calculations, initial
 *  gadget positions calculated correcly. (Chris Thewalt fixes).
 * 1.3 -- Color handling changed to deal with multiple colormaps.
 * 1.4 -- Fixed really silly bug with gadget offset.
 *
 */

#include "awm.h"

#include "X11/Xutil.h"

extern XContext AwmContext;
void PaintGadgets();

AwmInfoPtr IsGadgetWin(w, gnum)
Window w;
int *gnum;
{
     AwmInfoPtr awi;
     int i;

     Entry("IsGadgetWin")

     if (!XFindContext(dpy, w, AwmContext, (caddr_t *) &awi)) {
          if (!awi->gadgets)
               Leave((AwmInfoPtr)NULL)
	  for (i = 0; i < NumGadgets; i++)
	       if (awi->gadgets[i] == w) {
                    if (gnum)
                    *gnum = i;
	            Leave(awi)
               }
     }
     Leave((AwmInfoPtr)NULL)
}

/*
 * This routine adds and/or places  gadgets onto a title bar.
 */
int PutGadgets(w)
Window w;
{
     int i, dir;
     int rx, lx, nx, ny;
     XWindowAttributes xwa;
     XSetWindowAttributes swa;
     Colormap cmap;
     unsigned long valuemask;
     GadgetDecl *gdec;
     AwmInfo *awi;
     Boolean change = FALSE;
     Window tmp;

     Entry("PutGadgets")

     /* are there any gadgets declared? */
     if (NumGadgets < 1)
	  Leave_void
     /* does this window have a title? */
     if (!(awi = IsTitled(w)))
	  Leave_void

     if (!(awi->attrs & AT_GADGETS))
	  Leave_void
     /* if the window already has gadgets, this is a reshuffle */
     if (awi->gadgets)
	  change = TRUE;
     else
	  awi->gadgets = (Window *)malloc(sizeof(Window) * NumGadgets + 1);
     if (!awi->gadgets) {
	  fprintf(stderr, "PutGadgets: Can't allocate memory for gadgets!\n");
          Leave_void
     }
     if (XGetWindowAttributes(dpy, awi->title, &xwa) == BadWindow) {
	  fprintf(stderr, "PutGadgets: Can't get attrs for client window %x\n",
		  w);
	  Leave_void
     }
     /* define some initial positioning information */
     dir = RightGadgetGravity;
     lx = 0;
     rx = xwa.width;
     cmap = xwa.colormap;
     valuemask = CWBackPixel | CWEventMask | CWColormap;
     swa.event_mask = (ButtonPressMask | ButtonReleaseMask | ExposureMask);
     swa.colormap = cmap;
     for (i = 0; i < NumGadgets && Gadgets[i]; i++) {
	  gdec = Gadgets[i];

	  /* Does gadget have its own gravity? */
	  if (gdec->gravity != NoGadgetGravity)
	       dir = gdec->gravity;
	  ny = xwa.height - gdec->high;
	  if (ny <= 0)
               gdec->high = xwa.height;
	  else
	       ny = (ny / 2) - 1;
	  if (ny < 0)
	       ny = 0;
	  if (dir == LeftGadgetGravity) {
	       nx = lx + gdec->offset;
	       if (lx != 0) /* not first time through, add pad */
		    nx += GadgetPad;
	       if (nx < 0)
		    nx = 0;
	       lx = nx + gdec->wide;
               dir = RightGadgetGravity;
	  }
	  else if (dir == RightGadgetGravity) {	
	       nx = rx - gdec->wide - gdec->offset - (2 * GadgetBorder);
	       if (rx != xwa.width)
		    nx -= GadgetPad;
	       rx = nx - GadgetBorder;
               dir = LeftGadgetGravity;
	  }
	  else /* Center Gravity */
	       nx = ((xwa.width - gdec->wide) / 2) + gdec->offset;
	  if (change == FALSE || !awi->gadgets[i]) {
	       swa.background_pixel = GetPixel(gdec->backcolor, cmap);
	       tmp = awi->gadgets[i] = XCreateWindow(dpy, awi->title, nx, ny,
						     gdec->wide, gdec->high,
						     GadgetBorder,
						     xwa.depth, xwa.class,
						     xwa.visual, valuemask,
						     &swa);
	       if (!tmp) {
		    fprintf(stderr, "PutGadgets: Can't create gadget #%d\n",
			    i);
		    Leave_void
	       }
	       if (gdec->data) {
		    Pixmap pix;

		    pix = XCreatePixmapFromBitmapData(dpy, tmp, gdec->data,
						      gdec->wide, gdec->high,
						      GetPixel(gdec->forecolor, cmap),
						      GetPixel(gdec->backcolor, cmap),
						      xwa.depth);
		    if (!pix) {
			 fprintf(stderr, "PutGadgets: Unable to create pixmap for #%d\n",
				 i);
                         fprintf(stderr, "Using grey...\n");
#ifndef RAINBOW
                         pix = GrayPixmap;
#else
                         pix = awi->grayPixmap;
#endif
		    }
		    XSetWindowBackgroundPixmap(dpy, tmp, pix);
		    XFreePixmap(dpy, pix);
	       }
	       XMapWindow(dpy, tmp);
	       XSaveContext(dpy, tmp, AwmContext, (caddr_t) awi);
	  }
	  else
	       XMoveWindow(dpy, awi->gadgets[i], nx, ny);
     }
     if (change == FALSE) /* make sure the window array is null terminated */
          awi->gadgets[i] = (Window)NULL;
     PaintGadgets(w);
     Leave_void
}

int FreeGadget(n)
int n;
{
     Entry("FreeGadget")

     if (Gadgets[n]->data)
	  XFree(Gadgets[n]->data);
     if (Gadgets[n]->name)
	  free(Gadgets[n]->name);
     if (Gadgets[n])
	  free(Gadgets[n]);
}

int CheckGadgets()
{
     int i, status = 0;

     Entry("CheckGadgets")

     for (i = 0; i < NumGadgets; i++)
	  if (!Gadgets[i]) {
	       fprintf(stderr, "awm: Gadget #%d is not declared!\n",
		       i);
	       fprintf(stderr, "All gadgets from 0 to %d must be declared.\n",
		       NumGadgets - 1);
	       status = 1;
	  }
     Leave(status)
}

Boolean ResetGadgets()
{
     int i;

     Entry("ResetGadgets")

     for (i = 0; i < NumGadgets; i++) {
	  FreeGadget(i);
	  Gadgets[i] = 0;
     }
     NumGadgets = 0;
     if (Gadgets) {
	  free(Gadgets);
	  Gadgets = 0;
     }
}
     
void DestroyGadgets(w)
Window w;
{
     int i;
     AwmInfoPtr awi;

     Entry("DestroyGadgets")

     if (!(awi = IsTitled(w)))
	  Leave_void
     if (!awi->gadgets)
	  Leave_void
     for (i = 0; i < NumGadgets && Gadgets[i]; i++) {
	  XDeleteContext(dpy, awi->gadgets[i], AwmContext);
	  XDestroyWindow(dpy, awi->gadgets[i]);
     }
     free(awi->gadgets);
     awi->gadgets = 0;
     Leave_void
}

void PaintGadgets(w)
Window w;
{
     int i, y, x = 0;
     AwmInfoPtr awi;
     GadgetDecl *gdec;
     XWindowAttributes xwa;

     Entry("PaintGadgets")

     if (!(awi = IsTitled(w)))
	  Leave_void
     if (!awi->gadgets)
	  Leave_void
     XGetWindowAttributes(dpy, awi->client, &xwa);
     for (i = 0; i < NumGadgets && Gadgets[i]; i++) {
	  gdec = Gadgets[i];
	  if (!gdec->data) {
	       XGCValues gcv;

	       gcv.font = gdec->fontInfo->fid;
	       gcv.foreground = GetPixel(gdec->forecolor, xwa.colormap);
	       gcv.background = GetPixel(gdec->backcolor, xwa.colormap);
	       XChangeGC(dpy, awi->winGC, (GCFont | GCForeground |
					   GCBackground), &gcv);
	       if (strlen(gdec->name) == 1) { /* it's a glyph (we think) */
		    XFontStruct *real_font;
		    XCharStruct char_info;
		    int char_number, index, byte1, byte2;

		    real_font = gdec->fontInfo;
		    char_number = gdec->name[0];
		    char_info = real_font->max_bounds;
		    index = char_number;

		    if (real_font->per_char) {
			 if (!real_font->min_byte1 && !real_font->max_byte1) {
			      if (char_number < real_font->min_char_or_byte2 ||
				  char_number > real_font->max_char_or_byte2)
				   index = real_font->default_char;
			      index -= real_font->min_char_or_byte2;
			 } else {
			      byte2 = index & 0xff;
			      byte1 = (index>>8) & 0xff;
			      if (byte1 < real_font->min_byte1 ||
				  byte1 > real_font->max_byte1 ||
				  byte2 < real_font->min_char_or_byte2 ||
				  byte2 > real_font->max_char_or_byte2) {
				   byte2 = real_font->default_char & 0xff;
				   byte1 = (real_font->default_char>>8)&0xff;
			      }
			      byte1 -= real_font->min_byte1;
			      byte2 -= real_font->min_char_or_byte2;
			      index = byte1 * (real_font->max_char_or_byte2 -
					       real_font->min_char_or_byte2 + 1) +
						    byte2;
			 }
			 char_info = real_font->per_char[index];
		    }
		    x = 1 - GadgetBorder - char_info.lbearing;
		    y = char_info.ascent;
	       }
	       else {
		    y = (gdec->high + (gdec->fontInfo->max_bounds.ascent -
				       gdec->fontInfo->max_bounds.descent)) / 2;
		    x = GadgetBorder;
	       }
	       XDrawString(dpy, awi->gadgets[i], awi->winGC, x,
			   y, (char *) gdec->name, strlen(gdec->name));
	  }
     }
     Leave_void
}
