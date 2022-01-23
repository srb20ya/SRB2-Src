/* $Id: shade.h,v 3.1 1998/02/02 03:09:34 brianp Exp $ */

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
 * $Log: shade.h,v $
 * Revision 3.1  1998/02/02 03:09:34  brianp
 * added GL_LIGHT_MODEL_COLOR_CONTROL (separate specular color interpolation)
 *
 * Revision 3.0  1998/01/31 21:03:42  brianp
 * initial rev
 *
 */


#ifndef SHADE_H
#define SHADE_H


#include "types.h"


extern void gl_shade_rgba( GLcontext *ctx,
                           GLuint side,
                           GLuint n,
                           /*const*/ GLfloat vertex[][4],
                           /*const*/ GLfloat normal[][3],
                           GLubyte color[][4] );


extern void gl_shade_rgba_spec( GLcontext *ctx,
                                GLuint side,
                                GLuint n,
                                /*const*/ GLfloat vertex[][4],
                                /*const*/ GLfloat normal[][3],
                                GLubyte baseColor[][4], GLubyte specColor[][4] );


extern void gl_shade_rgba_fast( GLcontext *ctx,
                                GLuint side,
                                GLuint n,
                                /*const*/ GLfloat normal[][3],
                                GLubyte color[][4] );


extern void gl_shade_ci( GLcontext *ctx,
                         GLuint side,
                         GLuint n,
                         GLfloat vertex[][4],
                         GLfloat normal[][3],
                         GLuint indexResult[] );

#endif

