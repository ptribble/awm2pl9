


#ifndef lint
static char *rcsid_neaten_ext_h = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/neaten.ext.h,v 1.2 89/02/07 21:25:41 jkh Exp $";
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

#ifndef NEATEN_DEFINE
#include "neaten.def.h"
#endif

extern void Neaten_Desktop();
extern void Neaten_Initialize();
extern void Neaten_Icon_Placement();
extern void Neaten_Identify();
extern int Neaten_Set_Desired();
extern int Neaten_Set_Min();
extern int Neaten_Set_Max();
extern void Neaten_Prorate();
extern int Neaten_Get_Geometry();
extern void Neaten_Set_Priorities();
extern void Neaten_Set_Options();
