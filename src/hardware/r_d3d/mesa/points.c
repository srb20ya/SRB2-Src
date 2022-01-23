/* $Id: points.c,v 3.5 1998/07/29 04:08:09 brianp Exp $ */

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
 * $Log: points.c,v $
 * Revision 3.5  1998/07/29 04:08:09  brianp
 * feedback returned wrong point color
 *
 * Revision 3.4  1998/06/07 22:18:52  brianp
 * implemented GL_EXT_multitexture extension
 *
 * Revision 3.3  1998/03/27 04:17:31  brianp
 * fixed G++ warnings
 *
 * Revision 3.2  1998/02/08 20:19:12  brianp
 * removed unneeded headers
 *
 * Revision 3.1  1998/02/04 00:44:29  brianp
 * fixed casts and conditional expression problems for Amiga StormC compiler
 *
 * Revision 3.0  1998/01/31 21:01:27  brianp
 * initial rev
 *
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "context.h"
#include "feedback.h"
#include "macros.h"
#include "pb.h"
#include "span.h"
#include "texstate.h"
#include "types.h"
#include "vb.h"
#include "mmath.h"
#endif



void gl_PointSize( GLcontext *ctx, GLfloat size )
{
   if (size<=0.0) {
      gl_error( ctx, GL_INVALID_VALUE, "glPointSize" );
      return;
   }
   if (INSIDE_BEGIN_END(ctx)) {
      gl_error( ctx, GL_INVALID_OPERATION, "glPointSize" );
      return;
   }
   ctx->Point.Size = size;
   ctx->NewState |= NEW_RASTER_OPS;
}



void gl_PointParameterfvEXT( GLcontext *ctx, GLenum pname,
                                    const GLfloat *params)
{
   if (INSIDE_BEGIN_END(ctx)) {
      gl_error( ctx, GL_INVALID_OPERATION, "glPointParameterfvEXT" );
      return;
   }
   if(pname==GL_DISTANCE_ATTENUATION_EXT) {
         COPY_3V(ctx->Point.Params,params);
   } else {
        if (*params<0.0 ) {
            gl_error( ctx, GL_INVALID_VALUE, "glPointParameterfvEXT" );
            return;
        }
        switch (pname) {
            case GL_POINT_SIZE_MIN_EXT:
                ctx->Point.MinSize=*params;
                break;
            case GL_POINT_SIZE_MAX_EXT:
                ctx->Point.MaxSize=*params;
                break;
            case GL_POINT_FADE_THRESHOLD_SIZE_EXT:
                ctx->Point.Threshold=*params;
                break;
            default:
                gl_error( ctx, GL_INVALID_ENUM, "glPointParameterfvEXT" );
                return;
        }
   }
   ctx->NewState |= NEW_RASTER_OPS;
}


/**********************************************************************/
/*****                    Rasterization                           *****/
/**********************************************************************/


/*
 * There are 3 pairs (RGBA, CI) of point rendering functions:
 *   1. simple:  size=1 and no special rasterization functions (fastest)
 *   2. size1:  size=1 and any rasterization functions
 *   3. general:  any size and rasterization functions (slowest)
 *
 * All point rendering functions take the same two arguments: first and
 * last which specify that the points specified by VB[first] through
 * VB[last] are to be rendered.
 */



/*
 * Put points in feedback buffer.
 */
static void feedback_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint texSet = ctx->Texture.CurrentTransformSet;
   GLuint i;

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLfloat x, y, z, w, invq;
         GLfloat color[4], texcoord[4];
         x = VB->Win[i][0];
         y = VB->Win[i][1];
         z = VB->Win[i][2] / DEPTH_SCALE;
         w = VB->Clip[i][3];

         /* convert color from integer back to a float in [0,1] */
         color[0] = VB->Color[i][0] * (1.0F / 255.0F);
         color[1] = VB->Color[i][1] * (1.0F / 255.0F);
         color[2] = VB->Color[i][2] * (1.0F / 255.0F);
         color[3] = VB->Color[i][3] * (1.0F / 255.0F);

         invq = 1.0F / VB->MultiTexCoord[texSet][i][3];
         texcoord[0] = VB->MultiTexCoord[texSet][i][0] * invq;
         texcoord[1] = VB->MultiTexCoord[texSet][i][1] * invq;
         texcoord[2] = VB->MultiTexCoord[texSet][i][2] * invq;
         texcoord[3] = VB->MultiTexCoord[texSet][i][3];

         FEEDBACK_TOKEN( ctx, (GLfloat) (GLint) GL_POINT_TOKEN );
         gl_feedback_vertex( ctx, x, y, z, w, color,
                             (GLfloat) VB->Index[i], texcoord );
      }
   }
}



/*
 * Put points in selection buffer.
 */
static void select_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint i;

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         gl_update_hitflag( ctx, VB->Win[i][2] / DEPTH_SCALE );
      }
   }
}


/*
 * CI points with size == 1.0
 */
void size1_ci_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLfloat *win;
   GLint *pbx = PB->x, *pby = PB->y;
   GLdepth *pbz = PB->z;
   GLuint *pbi = PB->i;
   GLuint pbcount = PB->count;
   GLuint i;

   win = &VB->Win[first][0];
   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         pbx[pbcount] = (GLint)  win[0];
         pby[pbcount] = (GLint)  win[1];
         pbz[pbcount] = (GLint) (win[2] + ctx->PointZoffset);
         pbi[pbcount] = VB->Index[i];
         pbcount++;
      }
      win += 3;
   }
   PB->count = pbcount;
   PB_CHECK_FLUSH(ctx, PB)
}



/*
 * RGBA points with size == 1.0
 */
static void size1_rgba_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint red, green, blue, alpha;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

         red   = VB->Color[i][0];
         green = VB->Color[i][1];
         blue  = VB->Color[i][2];
         alpha = VB->Color[i][3];

         PB_WRITE_RGBA_PIXEL( PB, x, y, z, red, green, blue, alpha );
      }
   }
   PB_CHECK_FLUSH(ctx,PB)
}



/*
 * General CI points.
 */
static void general_ci_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLint isize;

   isize = (GLint) (CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         PB_SET_INDEX( ctx, PB, VB->Index[i] );

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_PIXEL( PB, ix, iy, z );
            }
         }
         PB_CHECK_FLUSH(ctx,PB)
      }
   }
}


/*
 * General RGBA points.
 */
static void general_rgba_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLint isize;

   isize = (GLint) (CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         PB_SET_COLOR( ctx, PB,
                       VB->Color[i][0],
                       VB->Color[i][1],
                       VB->Color[i][2],
                       VB->Color[i][3] );

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_PIXEL( PB, ix, iy, z );
            }
         }
         PB_CHECK_FLUSH(ctx,PB)
      }
   }
}




/*
 * Textured RGBA points.
 */
static void textured_rgba_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;
         GLint isize;
         GLint red, green, blue, alpha;
         GLfloat s, t, u;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

         isize = (GLint)
                   (CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);
         if (isize<1) {
            isize = 1;
         }

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         red   = VB->Color[i][0];
         green = VB->Color[i][1];
         blue  = VB->Color[i][2];
         alpha = VB->Color[i][3];
         s = VB->TexCoord[i][0] / VB->TexCoord[i][3];
         t = VB->TexCoord[i][1] / VB->TexCoord[i][3];
         u = VB->TexCoord[i][2] / VB->TexCoord[i][3];

/*    don't think this is needed
         PB_SET_COLOR( red, green, blue, alpha );
*/

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_TEX_PIXEL( PB, ix, iy, z, red, green, blue, alpha, s, t, u );
            }
         }
         PB_CHECK_FLUSH(ctx,PB)
      }
   }
}



/*
 * Antialiased points with or without texture mapping.
 */
static void antialiased_rgba_points( GLcontext *ctx,
                                     GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLfloat radius, rmin, rmax, rmin2, rmax2, cscale;

   radius = CLAMP( ctx->Point.Size, MIN_POINT_SIZE, MAX_POINT_SIZE ) * 0.5F;
   rmin = radius - 0.7071F;  /* 0.7071 = sqrt(2)/2 */
   rmax = radius + 0.7071F;
   rmin2 = rmin*rmin;
   rmax2 = rmax*rmax;
   cscale = 256.0F / (rmax2-rmin2);

   if (ctx->Texture.Enabled) {
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            GLint xmin, ymin, xmax, ymax;
            GLint x, y, z;
            GLint red, green, blue, alpha;
            GLfloat s, t, u;

            xmin = (GLint) (VB->Win[i][0] - radius);
            xmax = (GLint) (VB->Win[i][0] + radius);
            ymin = (GLint) (VB->Win[i][1] - radius);
            ymax = (GLint) (VB->Win[i][1] + radius);
            z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

            red   = VB->Color[i][0];
            green = VB->Color[i][1];
            blue  = VB->Color[i][2];
            s = VB->TexCoord[i][0] / VB->TexCoord[i][3];
            t = VB->TexCoord[i][1] / VB->TexCoord[i][3];
            u = VB->TexCoord[i][2] / VB->TexCoord[i][3];

            for (y=ymin;y<=ymax;y++) {
               for (x=xmin;x<=xmax;x++) {
                  GLfloat dx = x/*+0.5F*/ - VB->Win[i][0];
                  GLfloat dy = y/*+0.5F*/ - VB->Win[i][1];
                  GLfloat dist2 = dx*dx + dy*dy;
                  if (dist2<rmax2) {
                     alpha = VB->Color[i][3];
                     if (dist2>=rmin2) {
                        GLint coverage = (GLint) (256.0F-(dist2-rmin2)*cscale);
                        /* coverage is in [0,256] */
                        alpha = (alpha * coverage) >> 8;
                     }
                     PB_WRITE_TEX_PIXEL( PB, x,y,z, red, green, blue, alpha, s, t, u );
                  }
               }
            }
            PB_CHECK_FLUSH(ctx,PB)
         }
      }
   }
   else {
      /* Not texture mapped */
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            GLint xmin, ymin, xmax, ymax;
            GLint x, y, z;
            GLint red, green, blue, alpha;

            xmin = (GLint) (VB->Win[i][0] - radius);
            xmax = (GLint) (VB->Win[i][0] + radius);
            ymin = (GLint) (VB->Win[i][1] - radius);
            ymax = (GLint) (VB->Win[i][1] + radius);
            z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

            red   = VB->Color[i][0];
            green = VB->Color[i][1];
            blue  = VB->Color[i][2];

            for (y=ymin;y<=ymax;y++) {
               for (x=xmin;x<=xmax;x++) {
                  GLfloat dx = x/*+0.5F*/ - VB->Win[i][0];
                  GLfloat dy = y/*+0.5F*/ - VB->Win[i][1];
                  GLfloat dist2 = dx*dx + dy*dy;
                  if (dist2<rmax2) {
                     alpha = VB->Color[i][3];
                     if (dist2>=rmin2) {
                        GLint coverage = (GLint) (256.0F-(dist2-rmin2)*cscale);
                        /* coverage is in [0,256] */
                        alpha = (alpha * coverage) >> 8;
                     }
                     PB_WRITE_RGBA_PIXEL( PB, x, y, z, red, green, blue, alpha );
                  }
               }
            }
            PB_CHECK_FLUSH(ctx,PB)
         }
      }
   }
}



/*
 * Null rasterizer for measuring transformation speed.
 */
static void null_points( GLcontext *ctx, GLuint first, GLuint last )
{
   (void) ctx;
   (void) first;
   (void) last;
}



/* Definition of the functions for GL_EXT_point_parameters */

/*
 * Calculates the distance attenuation formula of a point in eye space
 * coordinates
 */
static GLfloat dist_attenuation(GLcontext *ctx, const GLfloat p[3])
{
 GLfloat dist;
 dist=GL_SQRT(p[0]*p[0]+p[1]*p[1]+p[2]*p[2]);
 return (1/(ctx->Point.Params[0]+ ctx->Point.Params[1]*dist +
             ctx->Point.Params[2]*dist*dist));
}

/*
 * Distance Attenuated General CI points.
 */
static void dist_atten_general_ci_points( GLcontext *ctx, GLuint first, 
					GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLint isize;
   GLfloat psize,dsize;
   psize=CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE);

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

         dsize=psize*dist_attenuation(ctx,VB->Eye[i]);
         if(dsize>=ctx->Point.Threshold) {
            isize=(GLint) (MIN2(dsize,ctx->Point.MaxSize)+0.5F);
         } else {
            isize=(GLint) (MAX2(ctx->Point.Threshold,ctx->Point.MinSize)+0.5F);
         }

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         PB_SET_INDEX( ctx, PB, VB->Index[i] );

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_PIXEL( PB, ix, iy, z );
            }
         }
         PB_CHECK_FLUSH(ctx,PB)
      }
   }
}

/*
 * Distance Attenuated General RGBA points.
 */
static void dist_atten_general_rgba_points( GLcontext *ctx, GLuint first, 
				GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLubyte alpha;
   GLint isize;
   GLfloat psize,dsize;
   psize=CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE);

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);
         dsize=psize*dist_attenuation(ctx,VB->Eye[i]);
         if (dsize >= ctx->Point.Threshold) {
            isize = (GLint) (MIN2(dsize,ctx->Point.MaxSize)+0.5F);
            alpha = VB->Color[i][3];
         }
         else {
            isize = (GLint) (MAX2(ctx->Point.Threshold,ctx->Point.MinSize)+0.5F);
            dsize /= ctx->Point.Threshold;
            alpha = (GLint) (VB->Color[i][3]* (dsize*dsize));
         }
         if (isize & 1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }
         PB_SET_COLOR( ctx, PB,
                       VB->Color[i][0],
                       VB->Color[i][1],
                       VB->Color[i][2],
                       alpha );

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_PIXEL( PB, ix, iy, z );
            }
         }
         PB_CHECK_FLUSH(ctx,PB)
      }
   }
}

/*
 *  Distance Attenuated Textured RGBA points.
 */
static void dist_atten_textured_rgba_points( GLcontext *ctx, GLuint first, 
					GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLfloat psize,dsize;
   psize=CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE);

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;
         GLint isize;
         GLint red, green, blue, alpha;
         GLfloat s, t, u;

         x = (GLint)  VB->Win[i][0];
         y = (GLint)  VB->Win[i][1];
         z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

         dsize=psize*dist_attenuation(ctx,VB->Eye[i]);
         if(dsize>=ctx->Point.Threshold) {
            isize=(GLint) (MIN2(dsize,ctx->Point.MaxSize)+0.5F);
            alpha=VB->Color[i][3];
         } else {
            isize=(GLint) (MAX2(ctx->Point.Threshold,ctx->Point.MinSize)+0.5F);
            dsize/=ctx->Point.Threshold;
            alpha = (GLint) (VB->Color[i][3]* (dsize*dsize));
         }

         if (isize<1) {
            isize = 1;
         }

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         red   = VB->Color[i][0];
         green = VB->Color[i][1];
         blue  = VB->Color[i][2];
         s = VB->TexCoord[i][0] / VB->TexCoord[i][3];
         t = VB->TexCoord[i][1] / VB->TexCoord[i][3];
         u = VB->TexCoord[i][2] / VB->TexCoord[i][3];

/*    don't think this is needed
         PB_SET_COLOR( red, green, blue, alpha );
*/
  
         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_TEX_PIXEL( PB, ix, iy, z, red, green, blue, alpha, s, t,
 u );
            }
         }
         PB_CHECK_FLUSH(ctx,PB)
      }
   }
}

/*
 * Distance Attenuated Antialiased points with or without texture mapping.
 */
static void dist_atten_antialiased_rgba_points( GLcontext *ctx,
                                     GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
   GLuint i;
   GLfloat radius, rmin, rmax, rmin2, rmax2, cscale;
   GLfloat psize,dsize,alphaf;
   psize=CLAMP(ctx->Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE);

   if (ctx->Texture.Enabled) {
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            GLint xmin, ymin, xmax, ymax;
            GLint x, y, z;
            GLint red, green, blue, alpha;
            GLfloat s, t, u;


            dsize=psize*dist_attenuation(ctx,VB->Eye[i]);
            if(dsize>=ctx->Point.Threshold) {
               radius=(MIN2(dsize,ctx->Point.MaxSize)*0.5F);
               alphaf=1.0;
            } else {
               radius=(MAX2(ctx->Point.Threshold,ctx->Point.MinSize)*0.5F);
               dsize/=ctx->Point.Threshold;
               alphaf=(dsize*dsize);
            }
            rmin = radius - 0.7071F;  /* 0.7071 = sqrt(2)/2 */
            rmax = radius + 0.7071F;
            rmin2 = rmin*rmin;
            rmax2 = rmax*rmax;
            cscale = 256.0F / (rmax2-rmin2);

            xmin = (GLint) (VB->Win[i][0] - radius);
            xmax = (GLint) (VB->Win[i][0] + radius);
            ymin = (GLint) (VB->Win[i][1] - radius);
            ymax = (GLint) (VB->Win[i][1] + radius);
            z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

            red   = VB->Color[i][0];
            green = VB->Color[i][1];
            blue  = VB->Color[i][2];
            s = VB->TexCoord[i][0] / VB->TexCoord[i][3];
            t = VB->TexCoord[i][1] / VB->TexCoord[i][3];
            u = VB->TexCoord[i][2] / VB->TexCoord[i][3];

            for (y=ymin;y<=ymax;y++) {
               for (x=xmin;x<=xmax;x++) {
                  GLfloat dx = x/*+0.5F*/ - VB->Win[i][0];
                  GLfloat dy = y/*+0.5F*/ - VB->Win[i][1];
                  GLfloat dist2 = dx*dx + dy*dy;
                  if (dist2<rmax2) {
                     alpha = VB->Color[i][3];
                     if (dist2>=rmin2) {
                        GLint coverage = (GLint) (256.0F-(dist2-rmin2)*cscale);
                        /* coverage is in [0,256] */
                        alpha = (alpha * coverage) >> 8;
                     }
                     alpha = (GLint) (alpha * alphaf);
                     PB_WRITE_TEX_PIXEL( PB, x,y,z, red, green, blue, alpha, s,
t, u );
                  }
               }
            }
            PB_CHECK_FLUSH(ctx,PB)
         }
      }
   }
   else {
      /* Not texture mapped */
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            GLint xmin, ymin, xmax, ymax;
            GLint x, y, z;
            GLint red, green, blue, alpha;

            dsize=psize*dist_attenuation(ctx,VB->Eye[i]);
            if(dsize>=ctx->Point.Threshold) {
               radius=(MIN2(dsize,ctx->Point.MaxSize)*0.5F);
               alphaf=1.0;
            } else {
               radius=(MAX2(ctx->Point.Threshold,ctx->Point.MinSize)*0.5F);
               dsize/=ctx->Point.Threshold;
               alphaf=(dsize*dsize);
            }
            rmin = radius - 0.7071F;  /* 0.7071 = sqrt(2)/2 */
            rmax = radius + 0.7071F;
            rmin2 = rmin*rmin;
            rmax2 = rmax*rmax;
            cscale = 256.0F / (rmax2-rmin2);

            xmin = (GLint) (VB->Win[i][0] - radius);
            xmax = (GLint) (VB->Win[i][0] + radius);
            ymin = (GLint) (VB->Win[i][1] - radius);
            ymax = (GLint) (VB->Win[i][1] + radius);
            z = (GLint) (VB->Win[i][2] + ctx->PointZoffset);

            red   = VB->Color[i][0];
            green = VB->Color[i][1];
            blue  = VB->Color[i][2];

            for (y=ymin;y<=ymax;y++) {
               for (x=xmin;x<=xmax;x++) {
                  GLfloat dx = x/*+0.5F*/ - VB->Win[i][0];
                  GLfloat dy = y/*+0.5F*/ - VB->Win[i][1];
                  GLfloat dist2 = dx*dx + dy*dy;
                  if (dist2<rmax2) {
                      alpha = VB->Color[i][3];
                     if (dist2>=rmin2) {
                        GLint coverage = (GLint) (256.0F-(dist2-rmin2)*cscale);
                        /* coverage is in [0,256] */
                        alpha = (alpha * coverage) >> 8;
                     }
                     alpha = (GLint) (alpha * alphaf);
                     PB_WRITE_RGBA_PIXEL( PB, x, y, z, red, green, blue, alpha )
;
                  }
               }
            }
            PB_CHECK_FLUSH(ctx,PB)
         }
      }
   }
}


/*
 * Examine the current context to determine which point drawing function
 * should be used.
 */
void gl_set_point_function( GLcontext *ctx )
{
   GLboolean rgbmode = ctx->Visual->RGBAflag;

   if (ctx->RenderMode==GL_RENDER) {
      if (ctx->NoRaster) {
         ctx->Driver.PointsFunc = null_points;
         return;
      }
      if (ctx->Driver.PointsFunc) {
         /* Device driver will draw points. */
         ctx->Driver.PointsFunc = ctx->Driver.PointsFunc;
      }
      else if (ctx->Point.Params[0]==1.0 && ctx->Point.Params[1]==0.0 && 
               ctx->Point.Params[2]==0.0) { 
         if (ctx->Point.SmoothFlag && rgbmode) {
            ctx->Driver.PointsFunc = antialiased_rgba_points;
         }
         else if (ctx->Texture.Enabled) {
	    ctx->Driver.PointsFunc = textured_rgba_points;
         }
         else if (ctx->Point.Size==1.0) {
            /* size=1, any raster ops */
            if (rgbmode)
               ctx->Driver.PointsFunc = size1_rgba_points;
            else
               ctx->Driver.PointsFunc = size1_ci_points;
         }
         else {
	    /* every other kind of point rendering */
            if (rgbmode)
               ctx->Driver.PointsFunc = general_rgba_points;
            else
               ctx->Driver.PointsFunc = general_ci_points;
         }
      } 
      else if(ctx->Point.SmoothFlag && rgbmode) {
         ctx->Driver.PointsFunc = dist_atten_antialiased_rgba_points;
      }
      else if (ctx->Texture.Enabled) {
         ctx->Driver.PointsFunc = dist_atten_textured_rgba_points;
      } 
      else {
         /* every other kind of point rendering */
         if (rgbmode)
            ctx->Driver.PointsFunc = dist_atten_general_rgba_points;
         else
            ctx->Driver.PointsFunc = dist_atten_general_ci_points;
     }
   }
   else if (ctx->RenderMode==GL_FEEDBACK) {
      ctx->Driver.PointsFunc = feedback_points;
   }
   else {
      /* GL_SELECT mode */
      ctx->Driver.PointsFunc = select_points;
   }

}

