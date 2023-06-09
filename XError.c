


#ifndef lint
static char *rcsid_XError_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/XError.c,v 1.2 89/02/07 21:24:08 jkh Exp $";
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
 */

#include "awm.h"

/*ARGSUSED*/
XError(dpy, rep)
Display *dpy;
XErrorEvent *rep;
{
#ifdef DEBUG
    char buffer[BUFSIZ];
    XGetErrorText(dpy, rep->error_code, buffer, BUFSIZ);
    fprintf(stderr, "awm: X error occurred during a awm operation.\n");
    fprintf(stderr, "     Description: '%s'\n", buffer);
    fprintf(stderr, "     Request code: %d\n", rep->request_code);
    fprintf(stderr, "     Request function: %d\n", rep->func);
    fprintf(stderr, "     Request window 0x%x\n", rep->window);
    fprintf(stderr, "     Error Serial #%d\n", rep->serial);
    fprintf(stderr, "     Current serial #%d\n", dpy->request);
#endif
}
