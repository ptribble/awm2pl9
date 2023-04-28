


#ifndef lint
static char *rcsid_Lower_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Lock.c,v 1.2 89/02/07 21:22:31 jkh Exp $";
#endif	lint

/*
 *
 * Copyright 1987, 1988 by Ardent Computer Corporation, Sunnyvale, Ca.
 *
 * Copyright 1988 by Jordan Hubbard.
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
 * 000 -- J.Hubbard, PCS Computer systems.
 */

#include "awm.h"

#ifdef LOCKSCR
/*ARGSUSED*/
Boolean Lock(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
     void lockscreen();

     Entry("Lock")

     lockscreen(dpy);
     Leave(TRUE)
}
#else
Boolean Lock(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
     fprintf(stderr,
	"awm: You don't have lockscreen compiled in this version of awm.\n");
     Leave(FALSE)
}
#endif /* LOCKSCR */
