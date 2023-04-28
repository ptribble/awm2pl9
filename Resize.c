


#ifndef lint
static char *rcsid_Resize_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Resize.c,v 1.2 89/02/07 21:23:37 jkh Exp $";
#endif	lint

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
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group
 *        Convert to X11
 * 002 -- Jordan Hubbard, U.C. berkeley.
 *	Added alternate placement of resize window, code for title bar
 * 	support.
 */

#include "awm.h"
#include "X11/Xutil.h"

#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define min(a,b) ( (a) > (b) ? (b) : (a) )
#define makemult(a, b) ((b==1) ? (a) : (((int)((a) / (b))) * (b)) )

/*ARGSUSED*/
Boolean Resize(window, mask, button, x0, y0)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x0, y0;                             /* Event mouse position. */
{
     XWindowAttributes client_info;	/* client window info. */
     XWindowAttributes frame_info;	/* frame window info */
     int x1, y1;                        /* fixed box corner   */
     int x2, y2;                        /* moving box corner   */
     int x, y;
     int xinc, yinc;
     int minwidth, minheight;
     int maxwidth, maxheight;
     int ox, oy;			/* which quadrant of window */
     int pop_x, pop_y;			/* location of pop window */
     int hsize, vsize;			/* dynamic size */
     int delta;	
     int junk_x, junk_y;
     unsigned int ptrmask;		/* pointer status word */
     int num_vectors;			/* Number of vectors to XDraw. */
     Window sub_win;			/* Mouse query sub window. */
     Window root;			/* Root query window. */
     XEvent button_event, *b_ev;	/* Button event packet. */
     XSegment box[MAX_BOX_VECTORS];	/* Box drawing vertex buffer. */
     XSegment zap[MAX_ZAP_VECTORS];	/* Zap drawing vertex buffer. */
     Boolean stop;			/* Should the window stop changing? */
     XSizeHints sizehints;
     XWindowChanges values;
     int width_offset, height_offset;	/* to subtract if resize increments */
     int x_offset, y_offset;		/* add to pointer to get anchor */
     AwmInfoPtr awi;
     int (*storegrid_func)();		/* which StoreGrid routine to use */
     int (*storebox_func)();		/* which StoreBox routing to use */
     int buttonConfirmEvent;
     extern void ungrab_pointer();
     extern void grab_pointer();

     Entry("Resize")

     /*
      * Do nothing if the event window is the root window.
      */
     if (window == RootWindow(dpy, scr))
	  Leave(FALSE)

     /*
      * Gather info about the event window.
      */
     awi = GetAwmInfo(window);
     if (!awi)
	  Leave(FALSE)
     /*
      * Do not resize an icon window.
      */
     if (window == awi->icon)
	  Leave(FALSE)

     window = awi->client;
     b_ev = &button_event;
     buttonConfirmEvent = ButtonRelease;

     status = XGetWindowAttributes(dpy, window, &client_info);
     if (status == FAILURE)
	  Leave(FALSE)

     if (awi->frame) { /* we have to compensate */
	  status = XGetWindowAttributes(dpy, awi->frame, &frame_info);
	  client_info.y = frame_info.y;
	  client_info.x = frame_info.x;
	  if (status == FAILURE)
	       Leave(FALSE)
          if (DECORATED(awi)) {
	       if (awi->title)
		    client_info.y += titleHeight + 2;
	       if (awi->attrs & AT_BORDER) {
		    client_info.x += BContext + 1;
		    if (!awi->title)
			 client_info.y += BContext + 1;
	       }
	  }
     }

     /*
      * Clear The vector buffers.
      */
     bzero(box, sizeof(box));
     if (Zap)
	  bzero(zap, sizeof(zap));
     storegrid_func = StoreGridBox;
     storebox_func = StoreBox;

     /*
      * If we are here then we have a resize operation in progress.
      */
     
     /*
      * Turn on the resize cursor.
      */
     grab_pointer();
     /*
      * calculate fixed point (x1, y1) and varying point (x2, y2).
      */
     hsize = client_info.width;
     vsize = client_info.height;
     x1 = client_info.x;
     y1 = client_info.y;
     x2 = x1 + hsize;
     y2 = y1 + vsize;

     /*
      * Get the event window resize hint.
      */
     sizehints.flags = 0;
     XGetSizeHints(dpy, window, &sizehints, XA_WM_NORMAL_HINTS); 
     CheckConsistency(&sizehints);

     /* until there are better WM_HINTS, we'll assume that the client's
      * minimum width and height are the appropriate offsets to subtract
      * when resizing with an explicit resize increment.
      */
     if ((sizehints.flags & PMinSize) && (sizehints.flags & PResizeInc)) {
	  width_offset = sizehints.min_width;
	  height_offset = sizehints.min_height;
     } else
	  width_offset = height_offset = 0;
 
     /*
      * decide what resize mode we are in. Always rubberband if window
      * is too small.
      */
     if (client_info.width > 2 && client_info.height > 2) {
	  ox = ((x0 - client_info.x) * 3) / client_info.width;
	  oy = ((y0 - client_info.y) * 3) / client_info.height;
	  if ((ox + oy) & 1) {
	       if (ox & 1) {
		    /* fix up size hints so that we will never change width */
		    sizehints.min_width = sizehints.max_width =
			 client_info.width;
		    if ((sizehints.flags & PMinSize) == 0) {
			 sizehints.min_height = 0;
			 sizehints.flags |= PMinSize;
		    }
		    if ((sizehints.flags & PMaxSize) == 0) {
			 sizehints.max_height = DisplayHeight(dpy, scr);
			 sizehints.flags |= PMaxSize;
		    }
	       }
	       if (oy & 1) {
		    /* fix up size hints so that we will never change height */
		    sizehints.min_height = sizehints.max_height =
			 client_info.height;
		    if ((sizehints.flags & PMinSize)==0) {
			 sizehints.min_width = 0;
			 sizehints.flags |= PMinSize;
		    }
		    if ((sizehints.flags & PMaxSize)==0) {
			 sizehints.max_width = DisplayWidth(dpy, scr);
			 sizehints.flags |= PMaxSize;
		    }
	       }
	  }
     }
     else ox = oy = 2;
     /* change fixed point to one that shouldn't move */
     if (oy == 0) { 
	  y = y1; y1 = y2; y2 = y;
     }
     if (ox == 0) { 
	  x = x1; x1 = x2; x2 = x;
     }
     if (sizehints.flags & PMinSize) {
	  minwidth = sizehints.min_width;
	  minheight = sizehints.min_height;
     } else {
	  minwidth = 0;
	  minheight = 0;
     }
     if (sizehints.flags & PMaxSize) {
	  maxwidth = max(sizehints.max_width, minwidth);
	  maxheight = max(sizehints.max_height, minheight);
     } else {
	  maxwidth = DisplayWidth(dpy, scr);
	  maxheight = DisplayHeight(dpy, scr);
     }
     if (sizehints.flags & PResizeInc) {
	  xinc = sizehints.width_inc;
	  yinc = sizehints.height_inc;
     } else {
	  xinc = 1;
	  yinc = 1;
     }
     switch (ox) {
     case 0:
	  pop_x = x1 - PWidth;
	  break;
     case 1:
	  pop_x = x1 + (hsize - PWidth) / 2;
	  break;
     case 2:
	  pop_x = x1;
	  break;
     }
     switch (oy) {
     case 0:
	  pop_y = y1 - PHeight;
	  break;
     case 1:
	  pop_y = y1 + (vsize - PHeight) / 2;
	  break;
     case 2:
	  pop_y = y1;
	  break;
     }
     /*
      * Double expose on the target window is too expensive for some reason
      * or another. Paint the popup window in the upper left hand
      * corner of the screen if RootResizeBox is FALSE. This is also
      * more-or-less consistent with the position of the map request popup.
      */
     if (RootResizeBox == TRUE)
	  values.x = values.y = 0;
     else {
	  if (pop_x < 0 || pop_x > (DisplayWidth(dpy, scr) - PWidth))
	       pop_x = 0;
	  if (pop_y < 0 || pop_y > (DisplayHeight(dpy, scr) - PHeight))
	       pop_y = 0;
	  values.x = pop_x;
	  values.y = pop_y;
     }
     values.stack_mode = Above;
     XConfigureWindow(dpy, Pop, (unsigned int) CWX|CWY|CWStackMode, &values);
     XMapWindow(dpy, Pop);
     if (Grid) {
	  num_vectors = (*storegrid_func)(box,
					  MIN(x1, x2), MIN(y1, y2),
					  MAX(x1, x2), MAX(y1, y2));
     }
     else {
	  num_vectors = (*storebox_func)(box,
					 MIN(x1, x2), MIN(y1, y2),
					 MAX(x1, x2), MAX(y1, y2));
     }

     /*
      * If we freeze the server, then we will draw solid
      * lines instead of flickering ones during resizing.
      */
     if (Freeze)
	  XGrabServer(dpy);
     /* protect us from ourselves */
     Snatched = True;
     /*
      * Process any pending exposure events before drawing the box.
      */
     while (QLength(dpy) > 0) {
	  XPeekEvent(dpy, b_ev);
	  if (b_ev->xany.window == RootWindow(dpy, scr))
	       break;
	  GetButton(b_ev);
     }
     if (ResizeRelative) {
	  x_offset = x2 - x0;
	  y_offset = y2 - y0;
     }
     else
	  x_offset = y_offset = 0;

     /*
      * Now draw the box.
      */
     DrawBox();
     Frozen = window;
 
     stop = FALSE;
     x = -1; y = -1;
     
     while (!stop) {
	  if (x != x2 || y != y2) {
	       x = x2; y = y2;

	       /*
		* If we've frozen the server, then erase
		* the old box.
		*/
	       if (Freeze)
		    DrawBox();
	       
	       if (Grid) {
		    num_vectors = (*storegrid_func)(box,
						    MIN(x1, x), MIN(y1, y),
						    MAX(x1, x), MAX(y1, y));
	       }
	       else {
		    num_vectors = (*storebox_func)(box,
						   MIN(x1, x), MIN(y1, y),
						   MAX(x1, x), MAX(y1, y));
	       }
	       
	       if (Freeze)
		    DrawBox();
	       
	       {
		    int Hsize = (hsize - width_offset) / xinc;
		    int Vsize = (vsize - height_offset) / yinc;
		    int pos = 4;
		    PText[0] = (Hsize>99) ? (Hsize / 100 + '0')	  : ' ';
		    PText[1] = (Hsize>9)  ? ((Hsize / 10) % 10 + '0') : ' ';
		    PText[2] = Hsize % 10 + '0';
		    if (Vsize>99) PText[pos++] = Vsize / 100 + '0';
		    if (Vsize>9)  PText[pos++] = (Vsize / 10) % 10 + '0';
		    PText[pos++] = Vsize % 10 + '0';
		    while (pos<7) PText[pos++] = ' ';
	       }
	       /*
		* If the font is not fixed width we have to
		* clear the window to guarantee that the characters
		* that were there before are erased.
		*/
	       if (!(PFontInfo->per_char))
		    XClearWindow(dpy, Pop);
	       XDrawImageString(
				dpy, Pop, PopGC,
				PPadding, PPadding+PFontInfo->ascent,
				PText, PTextSize);
	  }
	  if (!Freeze) {
	       DrawBox();
	       DrawBox();
	  }

	  if (XPending(dpy) && !ProcessRequests(box, num_vectors) &&
	      GetButton(b_ev)) {
	       if ((b_ev->xany.type != ButtonPress) && 
		   (b_ev->xany.type != ButtonRelease))
		    continue; /* spurious menu event... */

	       if (Freeze) {
		    DrawBox();
		    Frozen = (Window)0;
		    XUngrabServer(dpy);
	       }

	       if (b_ev->xany.type == buttonConfirmEvent &&
		   b_ev->xbutton.button == button)
		    stop = TRUE;
	       else {
		    XUnmapWindow(dpy, Pop);
		    ResetCursor(button);
		    Snatched = False;
		    ungrab_pointer();
		    Leave(TRUE)
	       }
	  }
	  else {
	       XQueryPointer(dpy, RootWindow(dpy, scr), &root, 
			     &sub_win, &x2, &y2, &junk_x, &junk_y, &ptrmask);
	       x2 += x_offset;	/* get to anchor point */
	       y2 += y_offset;
	  }
	  hsize = max(min(abs (x2 - x1), maxwidth), minwidth);
	  hsize = makemult(hsize - minwidth, xinc) + minwidth;

	  vsize = max(min(abs(y2 - y1), maxheight), minheight);
	  vsize = makemult(vsize - minheight, yinc) + minheight; 
	  if (sizehints.flags & PAspect) {
	       if ((hsize * sizehints.max_aspect.y > 
		    vsize * sizehints.max_aspect.x)) {
		    delta = makemult((hsize * sizehints.max_aspect.y /
				      sizehints.max_aspect.x) - vsize, 
				     yinc);
		    if ((vsize + delta <= maxheight))
			 vsize += delta;
		    else {
			 delta = makemult(hsize - 
					  (sizehints.max_aspect.x *
					   vsize/sizehints.max_aspect.y), 
					  xinc);
			 if (hsize - delta >= minwidth)
			      hsize -= delta; 
		    }
	       }  
	       if (hsize * sizehints.min_aspect.y < vsize *
		   sizehints.min_aspect.x) {
		    delta = makemult((sizehints.min_aspect.x * 
				      vsize/sizehints.min_aspect.y) - hsize, 
				     xinc);
		    if (hsize + delta <= maxwidth)
			 hsize += delta;
		    else {
			 delta = makemult(vsize - 
					  (hsize*sizehints.min_aspect.y /
					   sizehints.min_aspect.x), 
					  yinc); 
			 if (vsize - delta >= minheight)
				vsize -= delta; 
		    }
	       }
	       
	  }
	  if (ox == 0)
	       x2 = x1 - hsize;
	  else
	       x2 = x1 + hsize;
	  
	  if (oy == 0)
	       y2 = y1 - vsize;
	  else
	       y2 = y1 + vsize;
	  
     }
     if (x2 < x1) {
	  x = x1; x1 = x2; x2 = x;
     }
     if (y2 < y1) {
	  y = y1; y1 = y2; y2 = y;
     }
     XUnmapWindow(dpy, Pop);
     if ((x1 !=client_info.x) || (y1 != client_info.y) || 
	 (hsize != client_info.width) ||
	 (vsize != client_info.height)) {
	  XWindowChanges xwc;
	       
	  xwc.x = x1;
	  xwc.y = y1;
	  xwc.width = hsize;
	  xwc.height = vsize;
	  ConfigureWindow(window, 
			  (unsigned int) CWX | CWY | CWHeight | CWWidth, &xwc);
     }
     Snatched = False;
     ungrab_pointer();
     Leave(TRUE)
}

CheckConsistency(hints)
XSizeHints *hints;
{
     Entry("CheckConsistency")

     if (hints->min_height < 0)
	  hints->min_height = 0;
     if (hints->min_width < 0)
	  hints->min_width = 0;
     
     if (hints->max_height <= 0 || hints->max_width <= 0)
	  hints->flags &= ~PMaxSize;
     
     hints->min_height = min(DisplayHeight(dpy, scr), hints->min_height);
     hints->min_width =  min(DisplayWidth(dpy, scr),  hints->min_width);
     
     hints->max_height = min(DisplayHeight(dpy, scr), hints->max_height);
     hints->max_width =  min(DisplayWidth(dpy, scr),  hints->max_width);
     
     if ((hints->flags & PMinSize) && (hints->flags & PMaxSize) && 
	 ((hints->min_height > hints->max_height) ||
	  (hints->min_width > hints->max_width)))
	  hints->flags &= ~(PMinSize|PMaxSize);
     
     if ((hints->flags & PAspect) && 
	 (hints->min_aspect.x * hints->max_aspect.y > 
	  hints->max_aspect.x * hints->min_aspect.y))
	  hints->flags &= ~(PAspect);
     Leave_void
}
