/* $Id: alpha.h,v 3.1 1998/03/28 03:57:13 brianp Exp $ */

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
 * $Log: alpha.h,v $
 * Revision 3.1  1998/03/28 03:57:13  brianp
 * added CONST macro to fix IRIX compilation problems
 *
 * Revision 3.0  1998/01/31 20:44:19  brianp
 * initial rev
 *
 */


#ifndef ALPHA_H
#define ALPHA_H


#include "types.h"


extern GLint gl_alpha_test( const GLcontext *ctx, GLuint n,
                            CONST GLubyte rgba[][4], GLubyte mask[] );


extern void gl_AlphaFunc( GLcontext *ctx, GLenum func, GLclampf ref );


#endif
