// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_bsp.c,v 1.9 2000/08/10 14:16:25 hurdler Exp $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: hw_bsp.c,v $
// Revision 1.9  2000/08/10 14:16:25  hurdler
// no message
//
// Revision 1.8  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.7  2000/08/03 17:32:31  metzgermeister
// *** empty log message ***
//
// Revision 1.6  2000/03/13 21:41:40  linuxcub
// Removed my socalled "fix". The answer lies elsewhere :-( linuxcub@email.dk
//
// Revision 1.5  2000/03/12 23:01:29  linuxcub
// I really hope this doesn't break anything important. I'd like
// to hear from anyone, especially Linux/opengl users. Try
// running E4M7 from "ultimate doom", at least my patch improves
// that level, although other levels still render incorrectly.
// Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.4  2000/03/06 18:44:00  hurdler
// hack for the polypoolsize problem
//
// Revision 1.3  2000/03/06 15:24:24  hurdler
// remove polypoolsize limit
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      convert Doom map
//
//-----------------------------------------------------------------------------


#include "hw_glob.h"
#include "../r_local.h"

// --------------------------------------------------------------------------
// This is global data for planes rendering
// --------------------------------------------------------------------------

extrasubsector_t*   extrasubsectors = NULL;

// newsubsectors are subsectors without segs, added for the plane polygons
#define NEWSUBSECTORS       50
int                 totsubsectors;
int                 addsubsector;

typedef struct { 
    float x;
    float y;
    float dx;
    float dy;
} fdivline_t;

// ==========================================================================
//                                    FLOOR & CEILING CONVEX POLYS GENERATION
// ==========================================================================

//debug counters
static int nobackpoly=0;
static int skipcut=0;
static int totalsubsecpolys=0;

// --------------------------------------------------------------------------
// Polygon fast alloc / free
// --------------------------------------------------------------------------
//hurdler: quick fix for those who wants to play with larger wad
#include "../m_argv.h"
//#define POLYPOOLSIZE   1024000    // may be much over what is needed
//                                  //TODO: check out how much is used
static int POLYPOOLSIZE=1024000;

static byte*    gr_polypool=NULL;
static byte*    gr_ppcurrent;
static int      gr_ppfree;

// only between levels, clear poly pool
static void HWR_ClearPolys (void)
{
    gr_ppcurrent = gr_polypool;
    gr_ppfree = POLYPOOLSIZE;
}


// allocate  pool for fast alloc of polys
void HWR_InitPolyPool (void)
{
    int pnum;

    //hurdler: quick fix for those who wants to play with larger wad
    if ( (pnum=M_CheckParm("-polypoolsize")) )
        POLYPOOLSIZE = atoi(myargv[pnum+1])*1024; // (in kb)

    CONS_Printf ("HWR_InitPolyPool() : allocating %d bytes\n", POLYPOOLSIZE);
    gr_polypool = (byte*) malloc (POLYPOOLSIZE);
    if (!gr_polypool)
        I_Error ("HWR_InitPolyPool() : couldn't malloc polypool\n");
    HWR_ClearPolys ();
}

void HWR_FreePolyPool (void)
{
    if (gr_polypool)
        free (gr_polypool);
    gr_polypool = NULL;
}

static poly_t* HWR_AllocPoly (int numpts)
{
    poly_t*     p;
    int         size;
#ifdef PARANOIA
    if(!gr_polypool)
        I_Error("Used gr_polypool without init !\n");
    if(!gr_ppcurrent)
        I_Error("gr_ppcurrent == NULL !!!\n");
#endif
    size = sizeof(poly_t) + sizeof(polyvertex_t) * numpts;
    if (gr_ppfree < size)
        I_Error ("allocpoly() : no more memory %d bytes left, %d bytes needed\n\n%s\n", 
                  gr_ppfree, size, "You can try the param -polypoolsize 2048 (or higher if needed)");

    p = (poly_t*)gr_ppcurrent;
    p->numpts = numpts;
    gr_ppcurrent += size;
    gr_ppfree -= size;
    return p;
}

static polyvertex_t* HWR_AllocVertex (void)
{
    polyvertex_t* p;
    int          size;

    size =  sizeof(polyvertex_t);
    if (gr_ppfree < size)
        I_Error ("allocpoly() : no more memory %d bytes left, %d bytes needed\n\n%s\n", 
                  gr_ppfree, size, "You can try the param -polypoolsize 2048 (or higher if needed)");

    p = (polyvertex_t*)gr_ppcurrent;
    gr_ppcurrent += size;
    gr_ppfree -= size;
    return p;
}


//TODO: polygons should be freed in reverse order for efficiency,
// for now don't free because it doenst' free in reverse order
static void HWR_FreePoly (poly_t* poly)
{
    int         size;
    
    size = sizeof(poly_t) + sizeof(polyvertex_t) * poly->numpts;
    memset(poly,0,size);
    //mempoly -= polysize;
}


// Return interception along bsp line,
// with the polygon segment
//
static float  bspfrac;
static polyvertex_t* fracdivline (fdivline_t* bsp, polyvertex_t* v1, polyvertex_t* v2)
{
static polyvertex_t  pt;
    double      frac;
    double      num;
    double      den;
    double      v1x,v1y,v1dx,v1dy;
    double      v2x,v2y,v2dx,v2dy;

    // a segment of a polygon
    v1x  = v1->x;
    v1y  = v1->y;
    v1dx = v2->x - v1->x;
    v1dy = v2->y - v1->y;

    // the bsp partition line
    v2x  = bsp->x;
    v2y  = bsp->y;
    v2dx = bsp->dx;
    v2dy = bsp->dy;

    den = v2dy*v1dx - v2dx*v1dy;
    if (den == 0)
        return NULL;       // parallel

    // first check the frac along the polygon segment,
    // (do not accept hit with the extensions)
    num = (v2x - v1x)*v2dy + (v1y - v2y)*v2dx;
    frac = num / den;
    if (frac<0 || frac>1)
        return NULL;

    // now get the frac along the BSP line
    // which is useful to determine what is left, what is right
    num = (v2x - v1x)*v1dy + (v1y - v2y)*v1dx;
    frac = num / den;
  bspfrac = frac;


    // find the interception point along the partition line
    pt.x = v2x + v2dx*frac;
    pt.y = v2y + v2dy*frac;

    return &pt;
}

//Hurdler: it's not used anymore
static boolean NearVertice (polyvertex_t* p1, polyvertex_t* p2)
{
#if 1
    float diff;
    diff = p2->x - p1->x;
    if (diff < -1.5f || diff > 1.5f)
       return false;
    diff = p2->y - p1->y;
    if (diff < -1.5f || diff > 1.5f)
       return false;
#else       
    if (p1->x != p2->x)
        return false;
    if (p1->y != p2->y)
        return false;
#endif
    // p1 and p2 are considered the same vertex
    return true;
}

// if two vertice coords have a x and/or y difference
// of less or equal than 1 FRACUNIT, they are considered the same
// point. Note: hardcoded value, 1.0f could be anything else.
static boolean SameVertice (polyvertex_t* p1, polyvertex_t* p2)
{
#if 0
    float diff;
    diff = p2->x - p1->x;
    if (diff < -1.5f || diff > 1.5f)
       return false;
    diff = p2->y - p1->y;
    if (diff < -1.5f || diff > 1.5f)
       return false;
#else       
    if (p1->x != p2->x)
        return false;
    if (p1->y != p2->y)
        return false;
#endif
    // p1 and p2 are considered the same vertex
    return true;
}


// split a _CONVEX_ polygon in two convex polygons
// outputs:
//   frontpoly : polygon on right side of bsp line
//   backpoly  : polygon on left side
//
static void SplitPoly (fdivline_t* bsp,         //splitting parametric line
                poly_t* poly,                   //the convex poly we split
                poly_t** frontpoly,             //return one poly here
                poly_t** backpoly)              //return the other here
{
    int     i,j;
    polyvertex_t *pv;

    int          ps,pe;
    int          nptfront,nptback;
    polyvertex_t vs;
    polyvertex_t ve;
    polyvertex_t lastpv;
    float        fracs,frace;   //used to tell which poly is on
                                // the front side of the bsp partition line
    poly_t*     swappoly;
    int         psonline, peonline;

    ps = pe = -1;
    psonline = peonline = 0;

    for (i=0; i<poly->numpts; i++)
    {
        j=i+1;
        if (j==poly->numpts) j=0;

        // start & end points
        pv = fracdivline (bsp, &poly->pts[i], &poly->pts[j]);

        if (pv)
        {
            if (ps<0) {
                // first point
                ps = i;
                vs = *pv;
                fracs = bspfrac;
            }
            else {
                //the partition line traverse a junction between two segments
                // or the two points are so close, they can be considered as one
                // thus, don't accept, since split 2 must be another vertex
                if (SameVertice(pv, &lastpv))
                {
                    if (pe<0) {
                        ps = i;
                        psonline = 1;
                    }
                    else {
                        pe = i;
                        peonline = 1;
                    }
                }else{
                    if (pe<0) {
                        pe = i;
                        ve = *pv;
                        frace = bspfrac;
                    }
                    else {
                    // a frac, not same vertice as last one
                    // we already got pt2 so pt 2 is not on the line,
                    // so we probably got back to the start point
                    // which is on the line
                        if (SameVertice(pv, &vs))
                            psonline = 1;
                        break;
                    }
                }
            }

            // remember last point intercept to detect identical points
            lastpv = *pv;
        }
    }

    // no split : the partition line is either parallel and
    // aligned with one of the poly segments, or the line is totally
    // out of the polygon and doesn't traverse it (happens if the bsp
    // is fooled by some trick where the sidedefs don't point to
    // the right sectors)
    if (ps<0)
    {
        //I_Error ("SplitPoly: did not split polygon (%d %d)\n"
        //         "debugpos %d",ps,pe,debugpos);

        // this eventually happens with 'broken' BSP's that accept
        // linedefs where each side point the same sector, that is:
        // the deep water effect with the original Doom

        //TODO: make sure front poly is to front of partition line?

        *frontpoly = poly;
        *backpoly  = NULL;
        return;
    }

    if (ps>=0 && pe<0)
    {
        //I_Error ("SplitPoly: only one point for split line (%d %d)",ps,pe);
        *frontpoly = poly;
        *backpoly  = NULL;
        return;
    }
    if (pe<=ps)
        I_Error ("SplitPoly: invalid splitting line (%d %d)",ps,pe);

    // number of points on each side, _not_ counting those
    // that may lie just one the line
    nptback  = pe - ps - peonline;
    nptfront = poly->numpts - peonline - psonline - nptback;

    if (nptback>0)
       *backpoly = HWR_AllocPoly (2 + nptback);
    else
       *backpoly = NULL;
    if (nptfront)
       *frontpoly = HWR_AllocPoly (2 + nptfront);
    else
       *frontpoly = NULL;

    // generate FRONT poly
    if (*frontpoly)
    {
        pv = (*frontpoly)->pts;
        *pv++ = vs;
        *pv++ = ve;
        i = pe;
        do {
            if (++i == poly->numpts)
               i=0;
            *pv++ = poly->pts[i];
        } while (i!=ps && --nptfront);
    }

    // generate BACK poly
    if (*backpoly)
    {
        pv = (*backpoly)->pts;
        *pv++ = ve;
        *pv++ = vs;
        i = ps;
        do {
            if (++i == poly->numpts)
               i=0;
            *pv++ = poly->pts[i];
        } while (i!=pe && --nptback);
    }

    // make sure frontpoly is the one on the 'right' side
    // of the partition line
    if (fracs>frace)
    {
        swappoly = *backpoly;
        *backpoly= *frontpoly;
        *frontpoly = swappoly;
    }
}


// use each seg of the poly as a partition line, keep only the
// part of the convex poly to the front of the seg (that is,
// the part inside the sector), the part behind the seg, is
// the void space and is cut out
//
static poly_t* CutOutSubsecPoly (seg_t* lseg, int count, poly_t* poly)
{
    int         i,j;
    
    polyvertex_t *pv;
    
    int          nump,ps,pe;
    polyvertex_t vs,ve,p1,p2;
    float        fracs;
    
    fdivline_t   cutseg;     //x,y,dx,dy as start of node_t struct
    
    poly_t*      temppoly;
    
    // for each seg of the subsector
    for(;count--;lseg++)
    {
        // no need to cut with a two sided line
        //if(lseg->backsector==NULL)
        //    continue;
        //x,y,dx,dy (like a divline)
        p1.x = lseg->v1->x*crapmul;
        p1.y = lseg->v1->y*crapmul;
        p2.x = lseg->v2->x*crapmul;
        p2.y = lseg->v2->y*crapmul;

        cutseg.x = p1.x;
        cutseg.y = p1.y;
        cutseg.dx = p2.x - p1.x;
        cutseg.dy = p2.y - p1.y;
        
        // see if it cuts the convex poly
        ps = -1;
        pe = -1;
        for (i=0; i<poly->numpts; i++)
        {
            j=i+1;
            if (j==poly->numpts)
                j=0;
            
            pv = fracdivline (&cutseg, &poly->pts[i], &poly->pts[j]);
            
            if (pv)
            {
                if (ps<0) {
                    ps = i;
                    vs = *pv;
                    fracs = bspfrac;
                }
                else {
                    //frac 1 on previous segment,
                    //     0 on the next,
                    //the split line goes through one of the convex poly
                    // vertices, happens quite often since the convex
                    // poly is already adjacent to the subsector segs
                    // on most borders
                    if (SameVertice(pv, &vs))
                        continue;
                    
                    if (fracs<=bspfrac) {
                        nump = 2 + poly->numpts - (i-ps);
                        pe = ps;
                        ps = i;
                        ve = *pv;
                    }
                    else {
                        nump = 2 + (i-ps);
                        pe = i;
                        ve = vs;
                        vs = *pv;
                    }
                    //found 2nd point
                    break;
                }
            }
        }
        
        // there was a split
        if (ps>=0)
        {
            //need 2 points
            if (pe>=0)
            {
                // generate FRONT poly
                temppoly = HWR_AllocPoly (nump);
                pv = temppoly->pts;
                *pv++ = vs;
                *pv++ = ve;
                do {
                    if (++ps == poly->numpts)
                        ps=0;
                    *pv++ = poly->pts[ps];
                } while (ps!=pe);
                HWR_FreePoly(poly);
                poly = temppoly;
            }
            //hmmm... maybe we should NOT accept this, but this happens
            // only when the cut is not needed it seems (when the cut
            // line is aligned to one of the borders of the poly, and
            // only some times..)
            else
                skipcut++;
            //    I_Error ("CutOutPoly: only one point for split line (%d %d) %d",ps,pe,debugpos);
        }
    }
    return poly;
}


// At this point, the poly should be convex and the exact
// layout of the subsector, it is not always the case,
// so continue to cut off the poly into smaller parts with
// each seg of the subsector.
//
static void HWR_SubsecPoly (int num, poly_t* poly)
{
    int          count;
    subsector_t* sub;
    seg_t*       lseg;

    sscount++;

    sub = &subsectors[num];
    count = sub->numlines;
    lseg = &segs[sub->firstline];

    if (poly) {
        poly = CutOutSubsecPoly (lseg,count,poly);
        totalsubsecpolys++;
        //extra data for this subsector
        extrasubsectors[num].planepoly = poly;
    }
}

// the bsp divline have not enouth presition 
// search for the segs source of this divline
void SearchDivline(node_t* bsp,fdivline_t *divline)
{
    int i;
    line_t* line=NULL;
    float   nearline;
    boolean side;

    for(i=0;i<numsegs;i++)
    {
        if( segs[i].v1->x == bsp->x && segs[i].v1->y == bsp->y )
        {
            float dx = (segs[i].v1->x - segs[i].v2->x);
            float dy = (segs[i].v1->y - segs[i].v2->y);
            float tmp=bsp->dy*dx-bsp->dx*dy;
            
            if (abs(tmp)<abs(nearline) || !line)
            {
                line=segs[i].linedef;
                nearline=tmp;
                side=true;
            }
        } 
        else
        if( segs[i].v2->x == bsp->x && segs[i].v2->y == bsp->y )
        {
            float dx = segs[i].v2->x - segs[i].v1->x;
            float dy = segs[i].v2->y - segs[i].v1->y;
            float tmp=bsp->dy*dx - bsp->dx*dy;

            if (abs(tmp)<abs(nearline) || !line)
            {
                line=segs[i].linedef;
                nearline=tmp;
                side=false;
            }
        } 
    }

    if( line && abs(nearline*crapmul*crapmul)<=20 
        && false // FIXME: There are severe problems in some levels without this - we should fix it properly!
        )
    {
        if( devparm )
            CONS_Printf("Found divline %-6.2f\n",nearline*crapmul*crapmul);
        divline->x=line->v1->x*crapmul;
        divline->y=line->v1->y*crapmul;
        divline->dx=(line->v2->x-line->v1->x)*crapmul;
        divline->dy=(line->v2->y-line->v1->y)*crapmul;
        if(bsp->dx*divline->dx<0)
            divline->dx = -divline->dx;
        if(bsp->dy*divline->dy<0)
            divline->dy = -divline->dy;
    }
    else
    {
        if( devparm )
            CONS_Printf("Divline not found %-6.2f\n",nearline*crapmul*crapmul);
        divline->x=bsp->x*crapmul;
        divline->y=bsp->y*crapmul;
        divline->dx=bsp->dx*crapmul;
        divline->dy=bsp->dy*crapmul;
    }
}

//#define BP_FIX_BBOX

// poly : the convex polygon that encloses all child subsectors
static void WalkBSPNode (int bspnum, poly_t* poly, unsigned short* leafnode, fixed_t *bbox)
{
    node_t*     bsp;

    poly_t*     backpoly;
    poly_t*     frontpoly;
    fdivline_t  fdivline;   
    polyvertex_t*   pt;
    int     i;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
            if (bspnum == -1)
            {
            // BP: i think this code is useless and wrong because
            // - bspnum==-1 happens only when numsubsectors == 0
            // - it can't happens in bsp recursive call since bspnum is a int and children is unsigned short
            // - the BSP is complet !! (there just can have subsector without segs) (i am not sure of this point)

                // do we have a valid polygon ?
                if (poly && poly->numpts > 2) {
                    CONS_Printf ("Adding a new subsector !!!\n");
                    if (addsubsector == numsubsectors + NEWSUBSECTORS)
                        I_Error ("WalkBSPNode : not enough addsubsectors\n");
                    else if (addsubsector > 0x7fff)
                        I_Error ("WalkBSPNode : addsubsector > 0x7fff\n");
                    *leafnode = (unsigned short)addsubsector | NF_SUBSECTOR;
                    extrasubsectors[addsubsector].planepoly = poly;
                    addsubsector++;
                }
                
                //add subsectors without segs here?
                //HWR_SubsecPoly (0, NULL);
            }
            else
                HWR_SubsecPoly (bspnum&(~NF_SUBSECTOR), poly);
#ifdef BP_FIX_BBOX
        M_ClearBox(bbox);
        poly=extrasubsectors[bspnum&~NF_SUBSECTOR].planepoly;
 
        for (i=0, pt=poly->pts; i<poly->numpts; i++,pt++)
             M_AddToBox (bbox, (fixed_t)(pt->x * FRACUNIT), (fixed_t)(pt->y * FRACUNIT));
#endif
            return;
    }

    bsp = &nodes[bspnum];
    SearchDivline(bsp,&fdivline);
    SplitPoly (&fdivline, poly, &frontpoly, &backpoly);

    //debug
    if (!backpoly)
        nobackpoly++;

    // Recursively divide front space.
#ifndef BP_FIX_BBOX
    // REMOVED TEMPORARY : IT SHOULD COMPLETE BBOX ONLY WITH SUBSECTOR 'CUTOUT' POLY
    if (frontpoly) {
        // Correct front bbox so it includes the space occupied by the convex polygon
        // (in software rendere it only needed to cover the space occupied by segs)
        for (i=0, pt=frontpoly->pts; i<frontpoly->numpts; i++,pt++)
            M_AddToBox (bsp->bbox[0], (fixed_t)(pt->x * FRACUNIT), (fixed_t)(pt->y * FRACUNIT));
    }
    else
        I_Error ("WalkBSPNode: no front poly ?");
#endif
    WalkBSPNode (bsp->children[0], frontpoly, &bsp->children[0],bsp->bbox[0]);
#ifdef BP_FIX_BBOX
    // copy child bbox
    memcpy(bbox, bsp->bbox[0], 4*sizeof(fixed_t));
#endif
    // Recursively divide back space.
    if (backpoly) {
        // Correct back bbox to include floor/ceiling convex polygon
#ifndef BP_FIX_BBOX
        for (i=0, pt=backpoly->pts; i<backpoly->numpts; i++,pt++)
            M_AddToBox (bsp->bbox[1], (fixed_t)(pt->x * FRACUNIT), (fixed_t)(pt->y * FRACUNIT));
#endif

        WalkBSPNode (bsp->children[1], backpoly, &bsp->children[1],bsp->bbox[1]);

#ifdef BP_FIX_BBOX        
        // enlarge bbox with seconde child
        M_AddToBox (bbox, bsp->bbox[1][BOXLEFT  ],
                          bsp->bbox[1][BOXTOP   ]);
        M_AddToBox (bbox, bsp->bbox[1][BOXRIGHT ],
                          bsp->bbox[1][BOXBOTTOM]);
#endif
    }

    // this frees the poly
    HWR_FreePoly (poly);
}


//FIXME: use Z_MAlloc() STATIC ?
void HWR_FreeExtraSubsectors (void)
{
    if (extrasubsectors)
        free(extrasubsectors);
}

#define MAXDIST   (1.5f)
// BP: can't move vertex : DON'T change polygone geometry ! (convex)
//#define MOVEVERTEX
boolean PointInSeg(polyvertex_t* a,polyvertex_t* v1,polyvertex_t* v2)
{
    register float ax,ay,bx,by,cx,cy,d,norm;
    register polyvertex_t* p;
    
    // check bbox of the seg first
    if( v1->x>v2->x )
    {
        p=v1;
        v1=v2;
        v2=p;
    }
    if(a->x<v1->x-MAXDIST || a->x>v2->x+MAXDIST)
        return false;

    if( v1->y>v2->y )
    {
        p=v1;
        v1=v2;
        v2=p;
    }
    if(a->y<v1->y-MAXDIST || a->y>v2->y+MAXDIST)
        return false;

    // v1 = origine
    ax= v2->x-v1->x;
    ay= v2->y-v1->y;
    norm = sqrt(ax*ax+ay*ay);
    ax/=norm;
    ay/=norm;
    bx=a->x-v1->x;
    by=a->y-v1->y;
    //d = a.b
    d =ax*bx+ay*by;
    // bound of the seg
    if(d<0 || d>norm)
        return false;
    //c=d.1a-b
    cx=ax*d-bx;
    cy=ay*d-by;
#ifdef MOVEVERTEX
    if(cx*cx+cy*cy<=MAXDIST*MAXDIST)
    {
        // ajust a little the point position
        a->x=ax*d+v1->x;
        a->y=ay*d+v1->y;
        // anyway the correction is not enouth
        return true;
    }
    return false;
#else
    return cx*cx+cy*cy<=MAXDIST*MAXDIST;
#endif
}

int numsplitpoly;

void SearchSegInBSP(int bspnum,polyvertex_t *p,poly_t *poly)
{
    poly_t  *q;
    int     j,k;

    if (bspnum & NF_SUBSECTOR)
    {
        if( bspnum!=-1 )
        {
            bspnum&=~NF_SUBSECTOR;
            q = extrasubsectors[bspnum].planepoly;
            if( poly==q || !q)
                return;
            for(j=0;j<q->numpts;j++)
            {
                k=j+1;
                if( k==q->numpts ) k=0;
                if( !SameVertice(p,&q->pts[j]) && 
                    !SameVertice(p,&q->pts[k]) &&
                    PointInSeg(p,&q->pts[j],&q->pts[k]) )
                {
                    poly_t *newpoly=HWR_AllocPoly(q->numpts+1);
                    int n;

                    for(n=0;n<=j;n++)
                        newpoly->pts[n]=q->pts[n];
                    newpoly->pts[k]=*p;
                    for(n=k+1;n<newpoly->numpts;n++)
                        newpoly->pts[n]=q->pts[n-1];
                    numsplitpoly++;
                    extrasubsectors[bspnum].planepoly = newpoly;
                    HWR_FreePoly(q);
                    return;
                }
            }
        }
        return;
    }

    if((nodes[bspnum].bbox[0][BOXBOTTOM]*crapmul-MAXDIST<=p->y) &&
       (nodes[bspnum].bbox[0][BOXTOP   ]*crapmul+MAXDIST>=p->y) &&
       (nodes[bspnum].bbox[0][BOXLEFT  ]*crapmul-MAXDIST<=p->x) &&
       (nodes[bspnum].bbox[0][BOXRIGHT ]*crapmul+MAXDIST>=p->x) )
        SearchSegInBSP(nodes[bspnum].children[0],p,poly);

    if((nodes[bspnum].bbox[1][BOXBOTTOM]*crapmul-MAXDIST<=p->y) &&
       (nodes[bspnum].bbox[1][BOXTOP   ]*crapmul+MAXDIST>=p->y) &&
       (nodes[bspnum].bbox[1][BOXLEFT  ]*crapmul-MAXDIST<=p->x) &&
       (nodes[bspnum].bbox[1][BOXRIGHT ]*crapmul+MAXDIST>=p->x) )
        SearchSegInBSP(nodes[bspnum].children[1],p,poly);
}

// search for T-intersection problem
// BP : It can be mush more faster doing this at the same time of the splitpoly
// but we must use a different structure : polygone pointing on segs 
// segs pointing on polygone and on vertex (too mush complicated, well not 
// realy but i am soo lasy), the methode discibed is also better for segs presition
int SolveTProblem (void)
{
    poly_t  *p;
    int     i,l;

    numsplitpoly=0;

    for(l=0;l<addsubsector;l++ )
    {
        p = extrasubsectors[l].planepoly;
        if( p )
        for(i=0;i<p->numpts;i++)
            SearchSegInBSP(numnodes-1,&p->pts[i],p);
    }
    return numsplitpoly;
}

#define NEARDIST (0.75f) 
#define MYMAX    (10000000000000.0f)
void AjustSegs(void)
{
    int i,j,count;
    seg_t* lseg;
    poly_t *p;
    int v1found,v2found;
    float nearv1,nearv2;

    for(i=0;i<numsubsectors;i++)
    {
        count = subsectors[i].numlines;
        lseg = &segs[subsectors[i].firstline];
        p = extrasubsectors[i].planepoly;
        if(!p)
            continue;
        for(;count--;lseg++)        
        {
            float distv1,distv2,tmp;
            nearv1=nearv2=MYMAX;
            for(j=0;j<p->numpts;j++)
            {
                distv1 = p->pts[j].x - ((float)lseg->v1->x)*crapmul; 
                tmp    = p->pts[j].y - ((float)lseg->v1->y)*crapmul;
                distv1 = distv1*distv1+tmp*tmp;
                if( distv1 <= nearv1 )
                {
                    v1found=j;
                    nearv1 = distv1;
                }
                // the same with v2
                distv2 = p->pts[j].x - ((float)lseg->v2->x)*crapmul; 
                tmp    = p->pts[j].y - ((float)lseg->v2->y)*crapmul;
                distv2 = distv2*distv2+tmp*tmp;
                if( distv2 <= nearv2 )
                {
                    v2found=j;
                    nearv2 = distv2;
                }
            }
            if( nearv1<=NEARDIST*NEARDIST )
                lseg->v1 = (vertex_t *)&(p->pts[v1found]);
            else
            {
                // BP: here we can do better, using PointInSeg and compute
                // the right point position also split a polygone side to
                // solve a T-intersection, but too mush work

                // convert fixed vertex to float vertex
                polyvertex_t *p=HWR_AllocVertex();
                p->x=lseg->v1->x*crapmul;
                p->y=lseg->v1->y*crapmul;
                lseg->v1 = (vertex_t *)p;
            }
            if( nearv2<=NEARDIST*NEARDIST )
                lseg->v2 = (vertex_t *)&(p->pts[v2found]);
            else
            {
                polyvertex_t *p=HWR_AllocVertex();
                p->x=lseg->v2->x*crapmul;
                p->y=lseg->v2->y*crapmul;
                lseg->v2 = (vertex_t *)p;
            }

            // recompute length 
            {
                float x,y;
                x=((polyvertex_t *)lseg->v2)->x-((polyvertex_t *)lseg->v1)->x+0.5*crapmul;
                y=((polyvertex_t *)lseg->v2)->y-((polyvertex_t *)lseg->v1)->y+0.5*crapmul;
                lseg->length = sqrt(x*x+y*y)*FRACUNIT;
                // BP: debug see this kind of segs
                //if (nearv2>NEARDIST*NEARDIST || nearv1>NEARDIST*NEARDIST)
                //    lseg->length=1;
            }
        }
    }
}

// call this routine after the BSP of a Doom wad file is loaded,
// and it will generate all the convex polys for the hardware renderer
void HWR_CreatePlanePolygons (int bspnum)
{
    poly_t*       rootp;
    polyvertex_t* rootpv;

    int     i;

    fixed_t     rootbbox[4];

    //CONS_Printf ("HWR_CreatePlanePolygons()\n");

    HWR_ClearPolys ();
    
    // find min/max boundaries of map
    //CONS_Printf ("Looking for boundaries of map...\n");
    M_ClearBox(rootbbox);
    for (i=0;i<numvertexes;i++)
        M_AddToBox(rootbbox,vertexes[i].x,vertexes[i].y);

    //CONS_Printf ("Generating subsector polygons... %d subsectors\n", numsubsectors);

    HWR_FreeExtraSubsectors ();
    // allocate extra data for each subsector present in map
    totsubsectors = numsubsectors + NEWSUBSECTORS;
    extrasubsectors = (extrasubsector_t*)malloc (sizeof(extrasubsector_t) * totsubsectors);
    if (!extrasubsectors)
        I_Error ("couldn't malloc extrasubsectors totsubsectors %d\n", totsubsectors);
    // set all data in to 0 or NULL !!!
    memset (extrasubsectors,0,sizeof(extrasubsector_t) * totsubsectors);

    // allocate table for back to front drawing of subsectors
    /*gr_drawsubsectors = (short*)malloc (sizeof(*gr_drawsubsectors) * totsubsectors);
    if (!gr_drawsubsectors)
        I_Error ("couldn't malloc gr_drawsubsectors\n");*/

    // number of the first new subsector that might be added
    addsubsector = numsubsectors;

    // construct the initial convex poly that encloses the full map
    rootp  = HWR_AllocPoly (4);
    rootpv = rootp->pts;

    rootpv->x = (float)rootbbox[BOXLEFT  ] * crapmul;
    rootpv->y = (float)rootbbox[BOXBOTTOM] * crapmul;  //lr
    rootpv++;
    rootpv->x = (float)rootbbox[BOXLEFT  ] * crapmul;
    rootpv->y = (float)rootbbox[BOXTOP   ] * crapmul;  //ur
    rootpv++;
    rootpv->x = (float)rootbbox[BOXRIGHT ] * crapmul;
    rootpv->y = (float)rootbbox[BOXTOP   ] * crapmul;  //ul
    rootpv++;
    rootpv->x = (float)rootbbox[BOXRIGHT ] * crapmul;
    rootpv->y = (float)rootbbox[BOXBOTTOM] * crapmul;  //ll
    rootpv++;

    WalkBSPNode (bspnum, rootp, NULL,rootbbox);

    i=SolveTProblem ();
    //CONS_Printf("%d point div a polygone line\n",i);
    AjustSegs();

    //debug debug..
    //if (nobackpoly)
    //    CONS_Printf ("no back polygon %d times\n",nobackpoly);
                             //"(should happen only with the deep water trick)"

    //if (skipcut)
    //    CONS_Printf ("%d cuts were skipped because of only one point\n",skipcut);


    //CONS_Printf ("done : %d total subsector convex polygons\n", totalsubsecpolys);
}
