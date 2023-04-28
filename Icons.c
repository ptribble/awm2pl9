


#ifndef lint
static char *rcsid_Icons_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Icons.c,v 1.2 89/02/07 20:05:21 jkh Exp $";
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
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


 
/*
 * MODIFICATION HISTORY
 *
 * 000 -- L. Guarino Reid, DEC Ultrix Engineering Group
 * 001 -- Jordan Hubbard, Ardent Computer
 *  Many modifications for titled windows.
 * 1.2 -- Jordan Hubbard, Ardent Computer.
 * vPad and hPad usage corrected (Chris Thewalt fix).
 * Clipping now set on right GC. 
 * 1.3 -- Changed the way icon button events are handled.
 * 1.4 -- Isaac J. Salzman, RAND Corp.
 * reworked the way icons are handled - if IconsLabels is set, icons
 * with pixmaps are labled at bottom & icons w/o pixmaps use a default
 * pixmap w/a label instead of just a solid bg tile (i.e. twm style),
 * otherwise old uwm style icons are used (typin and all).
 * 1.5 -- Modified Isaac's changes to allow wm_option.icon.labels to
 * work. This allows individual clients/classes to determine their
 * icon style.  -jkh
 */
 
#include "awm.h"
#include "X11/Xutil.h"

extern XContext AwmContext;
     extern void Warning();

/* the client should pass us a bitmap (single-plane pixmap with background=0
 * and foreground = 1).  It is our responsibility to convert it to a pixmap
 * of the appropriate depth for a window tile and also color it with the
 * appropriate background and foreground pixels.
 *
 * we'll use the (global) IconGC for the fore/background pixels.
 */

static Pixmap MakePixmapFromBitmap( bitmap, clipmask, width_return, height_return )
Pixmap bitmap, clipmask;
unsigned int *width_return, *height_return;
{
     Pixmap tile;
     Window junkW;
     int junk;
     unsigned int width, height;
     GC iGC;
     XGCValues gcv;
     
     Entry("MakePixmapFromBitmap")
	  
     if (!XGetGeometry( dpy, bitmap, &junkW, &junk, &junk,
		       &width, &height, &junk, &junk )) {
	  Warning( "client passed invalid pixmap for icon." );
	  Leave( NULL )
     }
     tile = XCreatePixmap( dpy, RootWindow(dpy, scr), width, height,
			  (unsigned) DefaultDepth(dpy, scr) );
     gcv.foreground = ITextForeground;
     gcv.background = ITextBackground;
     iGC = XCreateGC(dpy, RootWindow(dpy, scr), (GCForeground | GCBackground),
		     &gcv);
     if (clipmask)
	  XSetClipMask(dpy, iGC, clipmask);
     XCopyPlane(dpy, bitmap, tile, iGC, 0, 0, width, height, 0, 0, 
		(unsigned long) 1 );
     XFreeGC(dpy, iGC);
     if (width_return)
	  *width_return = width;
     if (height_return)
	  *height_return = height;
     Leave(tile)
}

char *GetIconName(window)
Window window;
{
     unsigned char *name;
     Status status;
     unsigned long len, after;
     Atom a_type;
     int a_form;
     
     Entry("GetIconName")
     
     status = XGetWindowProperty(dpy, window, XA_WM_ICON_NAME, 0L, 256L,
				 False, XA_STRING, &a_type, &a_form, &len, &after,
				 &name);
     
     if (status != Success) {
	  status = XGetWindowProperty(dpy, window, XA_WM_NAME, 0L, NAME_LEN,
				      False, XA_STRING, &a_type, &a_form, &len,
				      &after, &name);
	  if (status != Success)
	       Leave( DEF_NAME ) /* use default name */
     }
     if (a_form == 0)	/* we have no bananas */
	  Leave((char *)NULL)
     if (a_form != 8) {
	  Warning("Window has icon_name in wrong format");
	  if (name)
	       free(name);
	  Leave((char *)NULL)
     }
     if (len > NAME_LEN) {
	  Warning("Window has icon_name greater than maximum length");
	  len = NAME_LEN;
     }
     if (!len)
	  Leave((char *)NULL)
     name[len] = 0;
     Leave((char *) name)
}

Boolean IsIcon(icon, win)
Window icon;
Window *win;
{
     AwmInfoPtr awi;

     Entry("IsIcon")

     if (win)
	  *win = 0;
     awi = GetAwmInfo(icon);
     if (!awi)
	  Leave(FALSE)
     if (awi->icon == icon) {
	  if (win)
	       *win = awi->client;
	  Leave(TRUE)
     }
     else if (awi->client == icon || awi->frame == icon) {
	  if (win)
	       *win = awi->icon;
	  Leave(FALSE)
     }
     Leave(FALSE)
}

RemoveIcon(window)
Window window;
{
     AwmInfoPtr awi;

     Entry("RemoveIcon")
     
     if ((awi = GetAwmInfo(window)) != NULL) {
	  XDeleteContext(dpy, awi->icon, AwmContext);
	  if (awi->own) {
	       XDestroyWindow(dpy, awi->icon);
	       if ((awi->iconPixmap != IBackPixmap) && 
		   (awi->iconPixmap != IDefPixmap))
		   XFreePixmap(dpy, awi->iconPixmap);
	  }
	  awi->icon = (Drawable)NULL;
	  awi->state ^= ST_ICON;
	  awi->state |= ST_WINDOW;
#ifdef WMSTATE
	  awi->wm_state.icon=0;
	  awi->wm_state.state=NormalState;
	  XChangeProperty(dpy,awi->client,wm_state_atom,wm_state_atom,32,
			  PropModeReplace,(char *) &awi->wm_state,2);
#endif /* WMSTATE */
     }
     Leave_void
}

GetDefaultSize(window, icon_w, icon_h)
Window window;
int *icon_w, *icon_h;
{
     char *name;				/* Event window name. */

     Entry("GetDefaultSize")

     /*
      * Determine the size of the icon window.
      */ 
     name = GetIconName(window);
     *icon_h = IFontInfo->ascent + IFontInfo->descent;
     if (name) {
	  *icon_w = XTextWidth(IFontInfo, name, strlen(name));
	  if (*icon_w == 0)
	       *icon_w = *icon_h;
     }
     else 
	  *icon_w = *icon_h = 0; /* set to zilch if bogus name! */
     Leave_void
}

Window MakeIcon(window, x, y, mousePositioned)
Window window;                          /* associated window. */
int x, y;                               /* Event mouse position. */
Boolean mousePositioned;
{
     int icon_x, icon_y;		/* Icon U. L. X and Y coordinates. */
     unsigned int icon_w, icon_h;	/* Icon width and height. */
     unsigned int icon_bdr;		/* Icon border width. */
     long mask;				/* Icon event mask */
     XSetWindowAttributes iconValues;	/* for icon window creation */
     unsigned long iconValues_mask;	/* for above */
     XWMHints *wmhints;			/* see if icon position provided */
     XWMHints *XGetWMHints();
     Pixmap clip = 0;
     Window AddIcon();
     AwmInfoPtr awi;
     int junk;

     Entry("MakeIcon")

     awi = GetAwmInfo(window);

     if (awi->attrs & AT_ICONLABEL) {	/* check for iconLables - twm style */
	  iconValues.background_pixmap = IDefPixmap;
	  mask = StructureNotifyMask;
     }
     else {			        /* uwm style */
	  iconValues.background_pixmap = IBackPixmap;
	  mask = (ExposureMask | StructureNotifyMask | KeyPressMask);
     }

     /*
      * Process window manager hints.
      */ 
     if (wmhints = XGetWMHints(dpy, window)) {
	  if (wmhints->flags & IconWindowHint)
	       Leave(AddIcon(window, wmhints->icon_window, FALSE,
			     (StructureNotifyMask), (Pixmap)NULL));

	   if (wmhints->flags & IconPixmapHint) {
		if (wmhints->flags & IconMaskHint)
		     clip = wmhints->icon_mask;
		iconValues.background_pixmap =
		     MakePixmapFromBitmap(wmhints->icon_pixmap, clip,
					  &icon_w, &icon_h );
		if (iconValues.background_pixmap)
		     mask = (StructureNotifyMask);
		 else {
		      iconValues.background_pixmap = IBackPixmap;
		      wmhints->flags &= ~IconPixmapHint;
		 }
	   }
     }
     
     /*
      * we now have a pixmap of some sort - either a background
      *  tile or an actual image - get the dimensions...
      */

     if (!XGetGeometry(dpy, iconValues.background_pixmap,
		       &junk, &junk, &junk,
		       &icon_w, &icon_h, &junk, &junk )) {
	  Warning( "can't get geom of pixmap in MakeIcon" );
	  Leave( NULL );
     }
     
     if (awi->attrs & AT_ICONLABEL) {
	  char *s;
	  Pixmap p;
	  unsigned int nw, nh;
	  int tw, th, hoff, voff;
	   
	   if (s = GetIconName(window)) {
		nw = icon_w;
		nh = icon_h;

		/* find out dimensions of text, add padding */
		tw = XTextWidth(IFontInfo, s, strlen(s)) + HIconPad;
		th = IFontInfo->ascent + IFontInfo->descent + VIconPad;
		
		
		/* calculate the size including the icon label */
		if (nw < tw)	/* width >= text width */
		     nw = tw;
		
		/* always tack on extra for the label height */
		nh += th;
		
		/* now that we have dimensions, create the pixmap
		   we want to eventually use as the icon window
		   */
		
		if (!(p = XCreatePixmap(dpy, RootWindow(dpy, scr), nw, nh,
					(unsigned) DefaultDepth(dpy, scr)))) {
		     Warning("can't create pixmap in MakeIcon.");
		     Leave( NULL )
		}

		/* calculate horizontal offset of pixmap (center it!) */
		if (hoff = (nw - icon_w))
		     hoff = (hoff+1)/2;
		
		/* if the label goes at the top, push the thing to the bottom
		 * otherwise leave it at the top
	         */
		
		voff = ILabelTop ? (th - (VIconPad+1)/2) : (VIconPad+1)/2;
		
		XCopyArea(dpy, iconValues.background_pixmap, p,
			  IconGC, 0, 0, icon_w, icon_h, 
			  hoff, voff);
		
		/* now that we have a centered pixmap draw
		 * some text on it....
		 */
		
		if (hoff = (nw - tw))
		     hoff = (hoff+1)/2;
		else
		     hoff = (HIconPad ? (HIconPad+1)/2 : 1);
		
		/* see if string goes to top or bottom */
		if (ILabelTop)
		     voff = th - (IFontInfo->descent-((VIconPad+1)/2));
		else
		     voff = nh-(IFontInfo->descent+((VIconPad+1)/2));
		
		XDrawImageString(dpy, p, IconGC, hoff, voff, s, strlen(s));
		
		/*
		 * Now free up original pixmap and replace
		 * with this new one.
		 */
		
		icon_h = nh;
		icon_w = nw;
		
		if ((iconValues.background_pixmap != IBackPixmap) &&
		    (iconValues.background_pixmap != IDefPixmap))
		     XFreePixmap(dpy, iconValues.background_pixmap);
		iconValues.background_pixmap = p;
	   }
     }		 
     else {	 		/* do it the old way.... */
	  int h, w;
	  
	  if (!wmhints || !(wmhints->flags & IconPixmapHint)) {
	       GetDefaultSize(window, &w, &h);
		    
	       if ((w==0) || (h==0)) { /* no label, use IDefPixmap */
		    iconValues.background_pixmap = IDefPixmap;
			 
		    /* get geom of ipixmap */
		    if (!XGetGeometry(dpy, iconValues.background_pixmap,
				      &junk, &junk, &junk,
				      &icon_w, &icon_h, &junk, &junk)) {
			 Warning("can't get geom of pixmap in MakeIcon");
			 Leave(NULL)
		    }
	       }
	       else { /* use small label icon + some padding */
		    icon_h = h+VIconPad;
		    icon_w = w+HIconPad;
	       }
	  }
     }
     /*
      * Set the icon border attributes.
      */ 
     if (!wmhints || !(wmhints->flags & IconWindowHint)) {
	  icon_bdr = IBorderWidth;
	  iconValues.border_pixel = IBorder;
     }
     /*
      * Determine icon position....
      */
     if (wmhints && (wmhints->flags & IconPositionHint)) {
	  icon_x = wmhints->icon_x;
	  icon_y = wmhints->icon_y;
     } else {
	  if (mousePositioned) {
	       /*
		* Determine the coordinates of the icon window;
		* normalize so that we don't lose the icon off the
		* edge of the screen.
		*/
	       icon_x = x - (icon_w >> 1) + 1;
	       if (icon_x < 0) icon_x = 0;
	       icon_y = y - (icon_h >> 1) + 1;
	       if (icon_y < 0) icon_y = 0;
	       if ((icon_x - 1 + icon_w + (icon_bdr << 1)) > ScreenWidth) {
		    icon_x = ScreenWidth - icon_w - (icon_bdr << 1) + 1;
	       }
	       if ((icon_y - 1 + icon_h + (icon_bdr << 1)) > ScreenHeight) {
		    icon_y = ScreenHeight - icon_h - (icon_bdr << 1) + 1;
	       }
	  }
	  else {
	       icon_x = x + (icon_w >> 1);
	       icon_y = y + (icon_y >> 1);
	  }
	  
     }

     /*
      * Create the icon window.
      */
     iconValues_mask = CWBorderPixel | CWBackPixmap;
     /*
      * Here we assume that if they want save unders, they'll also want
      * backing store on the icons. Perhaps a dangerous assumption, but
      * one we have to make at this point.
      */
     if (SaveUnder) {
	  iconValues.backing_store = WhenMapped;
	  iconValues_mask |= CWBackingStore;
     }
     Leave(AddIcon(window,
		   XCreateWindow(
				 dpy, RootWindow(dpy, scr),
				 icon_x, icon_y,
				 icon_w, icon_h,
				 icon_bdr, 0, CopyFromParent, CopyFromParent,
				 iconValues_mask, &iconValues),
		   TRUE, mask, iconValues.background_pixmap))
}

Window AddIcon(window, icon, own, mask, background)
Window window, icon;
Boolean own;
long mask;
Pixmap background;
{
     AwmInfoPtr awi;

     Entry("AddIcon")

     if (icon == NULL)
	  Leave(NULL)
     /*
      * Use the text cursor whenever the mouse is in the icon window, if
      * it's a typein icon, otherwise use gumby....
      */

     if (background == IBackPixmap)
	 XDefineCursor(dpy, icon, TextCursor);
     else
	 XDefineCursor(dpy, icon, GumbyCursor);
     
     /*
      * Select "key pressed", "window exposure" and "unmap window"
      * events for the icon window.
      */
     XSelectInput(dpy, icon, mask | ButtonPressMask | ButtonReleaseMask);

     awi = GetAwmInfo(window);
     awi->icon = icon;
     awi->own = own;
     awi->iconPixmap = background;
#ifdef WMSTATE
     awi->wm_state.icon=icon;
     XChangeProperty(dpy,awi->client,wm_state_atom,wm_state_atom,32,
		     PropModeReplace,(char *) &awi->wm_state,2);
#endif /* WMSTATE */
     XSaveContext(dpy, icon, AwmContext, (caddr_t) awi);
     Leave(icon)
}
