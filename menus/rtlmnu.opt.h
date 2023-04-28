
/*
#ifndef lint
static char sccs_id[] = "@(#)rtlmenu.options.h	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif
*/


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  rtlmenu.options.h: option flags for rtlmenu user
*/

/*

Copyright 1987 by
	Siemens Corporate Research and Support, Inc., Princeton, New Jersey

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is
hereby granted, provided that the above copyright notice
appear in all copies and that both that copyright notice and
this permission notice appear in supporting documentation, and
that the name of Siemens not be used in advertising or
publicity pertaining to distribution of the software without
specific, written prior permission.  Siemens makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied
warranty.

*/

#ifndef OPTION_FLAGS
#define OPTION_FLAGS

/* flags for menu operation */

typedef unsigned int MenuOptionsMask;

#define clickokay   (1L<<0) /* accept click-click as equivalent to press-release  */
#define savebits    (1L<<1) /* attempt to save/restore area under menu            */
#define fixedchild  (1L<<2) /* pull up child in fixed position relative to parent */
#define rightoffset (1L<<3) /* when pulling up on right edge, should child be     */
                            /* offset to left, allowing parent to remain visible  */
#define bigoffset   (1L<<4) /* valid only when rightoffset=1.  if bigoffset=1,    */
                            /* offset child far enough to see most of parent;     */
                            /* else, just far enough to get back to parent.       */

#define RTLMenu_Option_Set(mask, option) (mask |= option)
#endif
