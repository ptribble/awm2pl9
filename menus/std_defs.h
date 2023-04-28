
/*
#ifndef lint
static char sccs_id[] = "@(#)std_defs.h	2.5  4/13/87 Copyright (c) 1987, Siemens Corporate Research and Support, Inc.";
#endif
*/


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  std_defs.h: standard definitions shared among menu programs.
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

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Siemens not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
Siemens makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/


#ifndef STD_DEFS
#define STD_DEFS

#include <stdlib.h>
#include <X11/Intrinsic.h>

/*	useful macros
 */
typedef char *pointer;

#define  allocate(type, quan)	( (type *) malloc(sizeof(type) * (quan)) )

#define  FOREVER		for (;;)


/*	initial pointer value
 */
#ifndef  NULL
#define  NULL		0
#endif

#ifndef  TRUE
#define  TRUE		1
#endif

#ifndef  FALSE
#define  FALSE		0
#endif

/*	VoidFunc type definition
 */
typedef void		(*VoidFunc)();


/*	minimal error status return values
 */
#define  SUCCESS	 0
#define  FAILURE	-1

#define ABS(val)  \
    ( ( (val) < 0 ) ? -(val) : (val) ) 

#endif
