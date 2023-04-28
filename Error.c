#include <unistd.h>


#ifndef lint
static char *rcsid_Error_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Error.c,v 1.1 89/01/23 15:34:11 jkh Exp $";
#endif	lint

#include <signal.h>
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
 * 1.3 -- Cleanup. Fixed bug.
 */

#ifndef lint
static char *sccsid = "@(#)Error.c	3.8	1/24/86";
#endif

#ifdef titan
#include <unistd.h>
#endif

#include "awm.h"

extern Boolean desktop_execd;
extern char execfile[];

/*
 * Default error reporting routine.  Called when a random awm error
 * is encountered.
 */
Error(s)
char *s;	/* Error description string */
{
    Entry("Error")

    fprintf(stderr, "awm: %s\n", s);
    if (!desktop_execd) {
	 if (access(execfile, X_OK) == 0) {
	      if (fork() == 0) {
#ifdef SYSV
		   setpgrp();
#else
		   setpgrp(0, getpid());
#endif
		   signal(SIGHUP, SIG_DFL);
		   signal(SIGQUIT, SIG_DFL);
		   signal(SIGINT, SIG_DFL);
		   execl("/bin/sh", "sh", "-c", execfile, 0);
		   _exit(127);
	      }
	      else
		   desktop_execd = TRUE;
	 }
    }
    Cleanup();
    exit(1);
}

void Warning(s)
char *s;	/* Error description string */
{
    Entry("Warning")

    fprintf(stderr, "awm: warning: %s\n", s);
    Leave_void
}
