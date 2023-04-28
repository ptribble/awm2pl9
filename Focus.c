


#ifndef lint
static char *rcsid_Focus_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Focus.c,v 1.2 89/02/07 20:04:46 jkh Exp $";
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
    Western Software Lab. Converted to X11.
 */

#ifndef lint
static char *sccsid = "@(#)Focus.c	3.8	1/24/86";
#endif

#include "awm.h"

extern Window FocusWindow;

/*ARGSUSED*/
Boolean Focus(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
     AwmInfoPtr awi;

     Entry("Focus");

     awi = GetAwmInfo(window);
     if (awi)
	  window = awi->client;
     XSetInputFocus(dpy, window, RevertToPointerRoot, CurrentTime);
     if (window != RootWindow(dpy, scr))
	  FocusSetByUser = TRUE;
     else
	  FocusSetByUser = FALSE;
     FocusSetByWM = FALSE;
     FocusWindow = window;
     Leave(FALSE)
}

/*ARGSUSED*/
Boolean UnFocus(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
     Entry("UnFocus");

     XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
     FocusSetByUser = FALSE;
     FocusSetByWM = FALSE;
     FocusWindow = RootWindow(dpy, scr);
     Leave(FALSE)
}

