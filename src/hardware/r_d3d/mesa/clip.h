/* $Id: clip.h,v 3.2 1998/06/21 02:00:55 brianp Exp $ */

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
 * $Log: clip.h,v $
 * Revision 3.2  1998/06/21 02:00:55  brianp
 * cleaned up clip interpolation function code
 *
 * Revision 3.1  1998/02/03 23:46:40  brianp
 * added 'space' parameter to clip interpolation functions
 *
 * Revision 3.0  1998/01/31 20:48:09  brianp
 * initial rev
 *
 */


#ifndef CLIP_H
#define CLIP_H


#include "types.h"



#ifdef DEBUG
#  define GL_VIEWCLIP_POINT( V )   gl_viewclip_point( V )
#else
#  define GL_VIEWCLIP_POINT( V )			\
     (    (V)[0] <= (V)[3] && (V)[0] >= -(V)[3]		\
       && (V)[1] <= (V)[3] && (V)[1] >= -(V)[3]		\
       && (V)[2] <= (V)[3] && (V)[2] >= -(V)[3] )
#endif




extern GLuint gl_viewclip_point( const GLfloat v[] );

extern GLuint gl_viewclip_line( GLcontext* ctx, GLuint *i, GLuint *j );

extern GLuint gl_viewclip_polygon( GLcontext* ctx, GLuint n, GLuint vlist[] );



extern GLuint gl_userclip_point( GLcontext* ctx, const GLfloat v[] );

extern GLuint gl_userclip_line( GLcontext* ctx, GLuint *i, GLuint *j );

extern GLuint gl_userclip_polygon( GLcontext* ctx, GLuint n, GLuint vlist[] );


extern void gl_ClipPlane( GLcontext* ctx,
                          GLenum plane, const GLfloat *equation );

extern void gl_GetClipPlane( GLcontext* ctx,
                             GLenum plane, GLdouble *equation );


/*
 * Clipping interpolation functions
 */

extern void gl_clip_interp_all( GLcontext *ctx, GLuint space,
                                GLuint dst, GLfloat t, GLuint in, GLuint out );

extern void gl_clip_interp_color_tex( GLcontext *ctx, GLuint space,
                                GLuint dst, GLfloat t, GLuint in, GLuint out );

extern void gl_clip_interp_tex( GLcontext *ctx, GLuint space,
                                GLuint dst, GLfloat t, GLuint in, GLuint out );

extern void gl_clip_interp_color( GLcontext *ctx, GLuint space,
                                GLuint dst, GLfloat t, GLuint in, GLuint out );


#endif
