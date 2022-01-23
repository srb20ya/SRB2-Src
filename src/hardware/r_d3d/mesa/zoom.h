/* $Id: zoom.h,v 3.2 1998/03/28 03:57:13 brianp Exp $ */

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
 * $Log: zoom.h,v $
 * Revision 3.2  1998/03/28 03:57:13  brianp
 * added CONST macro to fix IRIX compilation problems
 *
 * Revision 3.1  1998/02/08 20:22:40  brianp
 * added gl_write_zoomed_rgb_span()
 *
 * Revision 3.0  1998/02/01 22:13:55  brianp
 * initial rev
 *
 */


#ifndef ZOOM_H
#define ZOOM_H


#include "types.h"


extern void
gl_write_zoomed_rgba_span( GLcontext *ctx,
                           GLuint n, GLint x, GLint y, const GLdepth z[],
                           CONST GLubyte rgba[][4], GLint y0 );


extern void
gl_write_zoomed_rgb_span( GLcontext *ctx,
                          GLuint n, GLint x, GLint y, const GLdepth z[],
                          CONST GLubyte rgb[][3], GLint y0 );


extern void
gl_write_zoomed_index_span( GLcontext *ctx,
                            GLuint n, GLint x, GLint y, const GLdepth z[],
                            const GLuint indexes[], GLint y0 );


extern void
gl_write_zoomed_stencil_span( GLcontext *ctx,
                              GLuint n, GLint x, GLint y,
                              const GLubyte stencil[], GLint y0 );


#endif
