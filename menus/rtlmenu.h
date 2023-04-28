
/*
#ifndef lint
static char sccs_id[] = "@(#)rtlmenu.h	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif
*/


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  rtlmenu.h: definitions for rtlmenu user
*/

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

#ifndef RTL_INCLUDE
#define RTL_INCLUDE
#include "std_defs.h"

struct _RTLPoint
{
	int x, y;
};

typedef struct _RTLPoint RTLPoint;
typedef pointer RTLMenu;
typedef pointer RTLMenuItem;    

RTLMenu RTLMenu_Create();
RTLMenuItem RTLMenu_Append_Call();
RTLMenuItem RTLMenu_Append_Checkback();
RTLMenuItem RTLMenu_Append_Submenu();

#include "rtlmnu.opt.h"
#endif RTL_INCLUDE
