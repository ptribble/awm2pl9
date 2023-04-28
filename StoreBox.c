


#ifndef lint
static char *rcsid_StoreBox_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/StoreBox.c,v 1.1 89/01/23 15:34:53 jkh Exp $";
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
 Western Software Lab. Convert to X11.
 * 002 -- Jordan Hubbard, Ardent Computer
 *  Additional boxes for titled windows.
 */

/*
 *	StoreBox - This subroutine is used by the X Window Manager (xwm)
 *	to store the vertices for the resize / movement box in a vertex list.
 */

#include "awm.h"

/*
 * Store the vertices for the resize movement box (on a titled window)
 * in a vertex list.
 */
int StoreTitleBox(box, ulx, uly, lrx, lry)
register XSegment box[];
int ulx;			/* Upper left X coordinate. */
int uly;			/* Upper left Y coordinate. */
int lrx;			/* Lower right X coordinate. */
int lry;			/* Lower right Y coordinate. */
{
     Entry("StoreTitleBox");
     
     /*
      * Xor in.
      */
     
     box[0].x1 = ulx; box[0].y1 = uly;
     box[0].x2 = lrx; box[0].y2 = uly;

     box[1].x1 = ulx; box[1].y1 = uly + titleHeight + 2;
     box[1].x2 = lrx; box[1].y2 = uly + titleHeight + 2;
     
     box[2].x1 = lrx; box[2].y1 = uly;
     box[2].x2 = lrx; box[2].y2 = lry;
     
     box[3].x1 = lrx; box[3].y1 = lry;
     box[3].x2 = ulx; box[3].y2 = lry;
     
     box[4].x1 = ulx; box[4].y1 = lry;
     box[4].x2 = ulx; box[4].y2 = uly;

     
     /*
      * If we freeze the screen, don't bother to xor out.
      */
     if (Freeze)
	  Leave(5)
     
     /*
      * Xor out.
      */
     box[4].x1 = ulx; box[4].y1 = uly;
     box[4].x2 = lrx; box[4].y2 = uly;
     
     box[5].x1 = ulx; box[5].y1 = uly + titleHeight + 2;
     box[5].x2 = lrx; box[5].y2 = uly + titleHeight + 2;
     
     box[6].x1 = lrx; box[6].y1 = uly;
     box[6].x2 = lrx; box[6].y2 = lry;
     
     box[7].x1 = lrx; box[7].y1 = lry;
     box[7].x2 = ulx; box[7].y2 = lry;
     
     box[8].x1 = ulx; box[8].y1 = lry;
     box[8].x2 = ulx; box[8].y2 = uly;
     
     
     /*
      * Total number of vertices is 9.
      */
     Leave(9)
}

/*
 * Store the vertices for the resize movement box in a vertex list.
 */
int StoreBox(box, ulx, uly, lrx, lry)
register XSegment box[];
int ulx;			/* Upper left X coordinate. */
int uly;			/* Upper left Y coordinate. */
int lrx;			/* Lower right X coordinate. */
int lry;			/* Lower right Y coordinate. */
{
     Entry("StoreBox");
     
     /*
      * Xor in.
      */
     
     box[0].x1 = ulx; box[0].y1 = uly;
     box[0].x2 = lrx; box[0].y2 = uly;
     
     box[1].x1 = lrx; box[1].y1 = uly;
     box[1].x2 = lrx; box[1].y2 = lry;
     
     box[2].x1 = lrx; box[2].y1 = lry;
     box[2].x2 = ulx; box[2].y2 = lry;
     
     box[3].x1 = ulx; box[3].y1 = lry;
     box[3].x2 = ulx; box[3].y2 = uly;
     
     
     /*
      * If we freeze the screen, don't bother to xor out.
      */
     if (Freeze)
	  Leave(4)
     
     /*
      * Xor out.
      */
     box[4].x1 = ulx; box[4].y1 = uly;
     box[4].x2 = lrx; box[4].y2 = uly;
     
     box[5].x1 = lrx; box[5].y1 = uly;
     box[5].x2 = lrx; box[5].y2 = lry;
     
     box[6].x1 = lrx; box[6].y1 = lry;
     box[6].x2 = ulx; box[6].y2 = lry;
     
     box[7].x1 = ulx; box[7].y1 = lry;
     box[7].x2 = ulx; box[7].y2 = uly;
     
     
     /*
      * Total number of vertices is 8.
      */
     Leave(8)
}
