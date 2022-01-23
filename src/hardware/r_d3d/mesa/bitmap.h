/* $Id: bitmap.h,v 3.2 1998/02/15 01:32:59 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.0
 * Copyright (C) 1995-1998  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * $Log: bitmap.h,v $
 * Revision 3.2  1998/02/15 01:32:59  brianp
 * updated driver bitmap function
 *
 * Revision 3.1  1998/02/08 20:23:49  brianp
 * lots of bitmap rendering changes
 *
 * Revision 3.0  1998/01/31 20:47:29  brianp
 * initial rev
 *
 */


#ifndef BITMAP_H
#define BITMAP_H


#include "types.h"


extern GLboolean gl_direct_bitmap( GLcontext *ctx, 
                                   GLsizei width, GLsizei height,
                                   GLfloat xorig, GLfloat yorig,
                                   GLfloat xmove, GLfloat ymove,
                                   const GLubyte *bitmap );


extern void gl_Bitmap( GLcontext* ctx,
                       GLsizei width, GLsizei height,
                       GLfloat xorig, GLfloat yorig,
                       GLfloat xmove, GLfloat ymove,
                       struct gl_image *bitmap );


#endif
