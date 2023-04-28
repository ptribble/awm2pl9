


#ifndef lint
static char *rcsid_Raise_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Raise.c,v 1.2 89/02/07 21:23:28 jkh Exp $";
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
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- R. Kittell, DEC Storage A/D May 19, 1986
 *  Optionally warp mouse to upper right corner of window after raise.
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group
 *  Western Software Lab. Convert to X11.
 * 003 -- Jordan Hubbard, Ardent Computer
 *  Changed for awm.
 */

#include "awm.h"

/*ARGSUSED*/
Boolean Raise(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;           	                /* Button event detail. */
int x, y;                               /* Event mouse position. */

{
     XWindowAttributes winfo;			/* Window details for warp */
     AwmInfoPtr awi;
     int status, vpoint;
     
     Entry("Raise")
     
     /*
      * If the window is not the root window, raise the window and return.
      */

     awi = GetAwmInfo(window);
     if (!awi)
	  Leave(FALSE)
     if (window != RootWindow(dpy, scr)) {
	  window = awi->frame ? awi->frame : awi->client;
	  XRaiseWindow(dpy, window);
	  if (awi->frame)
	       vpoint = 10 + titleHeight + 2;
	  else
	       vpoint = 10;
	  /*
	   * Optionally warp the mouse to the upper left corner of the window.
	   */
	  if (WarpOnRaise) {
	       status = XGetWindowAttributes (dpy, window, &winfo);
	       if (status == FAILURE)
		    Leave(FALSE)
	       
	       status = XWarpPointer (dpy, None, window, 
				      0, 0, 0, 0,
				      winfo.width >= 7 ? winfo.width - 7 :
				      winfo.width / 2,
				      winfo.height >= vpoint ? vpoint :
				      winfo.height / 2);
	       if (status == FAILURE)
		    Leave(FALSE)
	  }
     }
     Leave(FALSE)
}
