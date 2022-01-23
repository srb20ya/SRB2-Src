/* $Id: asm_mmx.h,v 1.1 1998/03/27 04:40:07 brianp Exp $ */

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
 * $Log: asm_mmx.h,v $
 * Revision 1.1  1998/03/27 04:40:07  brianp
 * Initial revision
 *
 */


#ifndef ASM_MMX_H
#define ASM_MMX_H


extern void
gl_mmx_blend_transparency( GLcontext *ctx, GLuint n, const GLubyte mask[],
                           GLubyte rgba[][4], const GLubyte dest[][4] );


#endif
