
#ifndef lint
static char *rcsid_support_h = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/support.h,v 1.2 89/02/07 21:25:51 jkh Exp $";
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

#undef STACK_RTNAMES
#undef TRACE

#ifdef TRACE
#define Entry(woo) { printf("Pushing to %s\n", woo); push_rtn(woo); }
#define Leave(woo) { printf("Pop from %s with %x\n", curr_rtn(), woo); pop_rtn(); return(woo); }
#define Leave_void { printf("Pop from %s (void)\n", curr_rtn()); pop_rtn(); return; }
#define Trace_on _rtn_trace = 1;
#define Trace_off _rtn_trace = 0;
extern char *curr_rtn();
extern int _rtn_level;
extern int _rtn_trace;
#else
#ifdef SAVERTNAME
#define Entry(woo) push_rtn(woo);
#define Leave(woo) { pop_rtn(); return(woo); }
#define Leave_void { pop_rtn(); return; }
#define Trace_on _rtn_trace = 1;
#define Trace_off _rtn_trace = 0;
extern char *curr_rtn();
extern int _rtn_level;
extern int _rtn_trace;
#else
#define Entry(s)
#define Leave(s) return(s);
#define Leave_void return;
#define Trace_on
#define Trace_off
#define Curr_rtn()
#endif /* SAVERTNAME */
#endif /* TRACE */
