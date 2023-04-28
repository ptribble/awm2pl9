


#ifndef lint
static char *rcsid_Move_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Move.c,v 1.2 89/02/07 21:22:55 jkh Exp $";
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
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group, 
 *	  Western Software Lab. Port to X11
 */

#include "awm.h"

/*ARGSUSED*/
Boolean Move(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
     register int prev_x;		/* Previous event window X location. */
     register int prev_y;		/* Previous event window Y location. */
     XWindowAttributes window_info;	/* Event window information. */
     int cur_x;				/* Current event window X location. */
     int cur_y;				/* Current event window Y location. */
     int root_x;				/* Root window X location. */
     int root_y;				/* Root window Y location. */
     int ulx, uly;			/* Event window upper left X and Y. */
     int lrx, lry;			/* Event window lower right X and Y. */
     int init_ulx, init_uly;		/* Init window upper left X and Y. */
     int init_lrx, init_lry;		/* Init window lower right X and Y. */
     int num_vectors;			/* Number of vectors in box. */
     unsigned int ptrmask;		/* state of ptr when queried */
     Window sub_window;			/* Query mouse event sub-window. */
     Window root;			/* Query mouse event root. */
     AwmInfoPtr awi;
     XEvent button_event;		/* Button event packet. */
     XSegment box[MAX_BOX_VECTORS];	/* Box vertex buffer. */
     XSegment zap[MAX_ZAP_VECTORS];	/* Zap effect verted buffer. */
     int (*storegrid_func)();		/* Function to use for grid */
     int (*storebox_func)();		/* Function to use for box */
     extern void ungrab_pointer();
     extern void grab_pointer();
     
     Entry("Move")
	  
     /*
      * Do not try to move the root window.
      */
     if (window == RootWindow(dpy, scr))
	  Leave(FALSE)
		    
     /*
      * Change the cursor.
      */
     grab_pointer();
     /*
      * Clear the vector buffers.
      */
     bzero(box, sizeof(box));
     if (Zap)
	  bzero(zap, sizeof(zap));
     
     /*
      * Gather info on the event window.
      */
     awi = GetAwmInfo(window);
     if (!awi)
	  Leave(FALSE)
     status = XGetWindowAttributes(dpy, window, &window_info);
     if (status == FAILURE)
	  Leave(FALSE)

     if (awi->title && !IsIcon(window, None)) {
	  storegrid_func = StoreTitleGridBox;
	  storebox_func = StoreTitleBox;
     }
     else {
	  storegrid_func = StoreGridBox;
	  storebox_func = StoreBox;
     }
     /*
      * Initialize movement variables.
      */
     init_ulx = ulx = window_info.x;
     init_uly = uly = window_info.y;
     init_lrx = lrx = window_info.x + window_info.width +
	  (window_info.border_width << 1) - 1;
     init_lry = lry = window_info.y + window_info.height +
	  (window_info.border_width << 1) - 1;
     
     /*
      * Store the box.
      */
     if (Grid)
	  num_vectors = (*storegrid_func)(box, ulx, uly, lrx, lry);
     else
	  num_vectors = (*storebox_func)(box, ulx, uly, lrx, lry);
     
     /*
      * Initialize the previous location variables.
      */
     prev_x = x;
     prev_y = y;
     
     /*
      * Freeze the server, if requested by the user.
      * This results in a solid box instead of a flickering one.
      */
     if (Freeze)
	  XGrabServer(dpy);
     Snatched = True;
     /*
      * Process any pending exposure events before drawing the box.
      */
     while (QLength(dpy) > 0) {
	  XPeekEvent(dpy, &button_event);
	  if (((XAnyEvent *)&button_event)->window == RootWindow(dpy, scr))
	       break;
	  GetButton(&button_event);
     }
     
     /*
      * Now draw the box.
      */
     
     DrawBox();
     Frozen = window;
     
     /*
      * Main loop.
      */
     while (TRUE) {
	  
	  /*
	   * Check to see if we have a change in mouse button status.
	   * This is how we get out of this "while" loop.
	   */
	  if (XPending(dpy) && !ProcessRequests(box, num_vectors) &&
	      GetButton(&button_event)) {
	       /*
		* Process the pending events, this sequence is the only
		* way out of the loop and the routine.
		*/
	       
	       if ((button_event.type != ButtonPress) && 
		   (button_event.type != ButtonRelease)) {
		    continue; /* spurious menu event... */
	       }
	       
	       /*
		* If we froze the server, then erase the last lines drawn.
		*/
	       if (Freeze) {
		    DrawBox();
		    Frozen = (Window)0;
		    XUngrabServer(dpy);
	       }
	       
	       if ((button_event.type == ButtonRelease) &&
		   (((XButtonReleasedEvent *)&button_event)->button == button)) {
		    
		    /*
		     * The button was released, so move the window.
		     */
		    
		    if (Zap) {
			 num_vectors = StoreZap(zap,
						init_ulx, init_uly,
						init_lrx, init_lry,
						ulx, uly,
						lrx, lry);
			 DrawZap();
			 DrawZap();
		    }
#ifdef titan /* 5x4 alignment */
		    ulx = ((ulx + 3) / 5) * 5;
		    uly = ((uly + 2) / 4) * 4;
#endif /* titan */
		    if (Wall) {
			 if (ulx < 0)
			      ulx = 0;
			 if (ulx > (ScreenWidth - window_info.width))
			      ulx = (ScreenWidth - window_info.width);
			 if (uly < 0)
			      uly = 0;
			 if (uly > (ScreenHeight - window_info.height))
			      uly = (ScreenHeight - window_info.height);
		    }
		    XMoveWindow(dpy, window, ulx, uly);
		    Snatched = False;
		    ungrab_pointer();
		    Leave(TRUE)
	       }
	       else {
		    /*
		     * Some other button event occured, this aborts the
		     * current operation.
		     */
		    
		    ResetCursor(button);
		    Snatched = False;
		    ungrab_pointer();
		    Leave(TRUE)
	       }
	  }
	  
	  /*
	   * Take care of all the little things that have changed.
	   */
	  XQueryPointer(dpy, 
			RootWindow(dpy, scr), &root,  &sub_window,
			&root_x, &root_y, &cur_x, &cur_y, &ptrmask);
	  if ((cur_x != prev_x) || (cur_y != prev_y)) {
	       
	       /*
		* If we've frozen the server, then erase the old box first!
		*/
	       if (Freeze)
		    DrawBox();
	       
	       /*
		* Box position has changed.
		*/
	       ulx += cur_x - prev_x;
	       uly += cur_y - prev_y;
	       lrx += cur_x - prev_x;
	       lry += cur_y - prev_y;
	       
	       /*
		* Box needs to be restored.
		*/
	       if (Grid)
		    num_vectors = (*storegrid_func)(box, ulx, uly, lrx, lry);
	       else
		    num_vectors = (*storebox_func)(box, ulx, uly, lrx, lry);
	       
	       
	       /*
		* Draw the new box.
		*/
	       if (Freeze)
		    DrawBox();
	  }
	  
	  /* 
	   * Save old box position.
	   */
	  prev_x = cur_x;
	  prev_y = cur_y;
	  
	  /*
	   * If server is not frozen, then draw the "flicker" box.
	   */
	  if (!Freeze) {
	       DrawBox();
	       DrawBox();
	  }
     }
}
