
#include "TBESurface.h"

extern refimport_t ri;
extern model_t	*currentmodel;
extern model_t	*loadmodel;

int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

static vec3_t	modelorg;		// relative to viewpoint

msurface_t	*r_alpha_surfaces;

#define LIGHTMAP_BYTES 4

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

#define	MAX_LIGHTMAPS	128

int		c_visible_lightmaps;
int		c_visible_textures;

typedef struct
{
    int internal_format;
    int	current_lightmap_texture;

    msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];

    int			allocated[BLOCK_WIDTH];

    // the lightmap texture data needs to be kept in
    // main memory so texsubimage can update properly
    byte		lightmap_buffer[4*BLOCK_WIDTH*BLOCK_HEIGHT];

} gllightmapstate_t;

static float s_blocklights[34*34*3];
static gllightmapstate_t gl_lms;

static void		LM_InitBlock( void );
static void		LM_UploadBlock( qboolean dynamic );
static qboolean	LM_AllocBlock (int w, int h, int *x, int *y);

/*
** R_SetCacheState
*/
void R_SetCacheState( msurface_t *surf )
{
    int maps;

    for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
    {
        surf->cached_light[maps] = 1.0f;//r_newrefdef.lightstyles[surf->styles[maps]].white;
    }
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
{
    int			smax, tmax;
    int			r, g, b, a, max;
    int			i, j, size;
    byte		*lightmap;
    float		scale[4];
    int			nummaps;
    float		*bl;

    //lightstyle_t	*style;

    int monolightmap;

    if ( surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP) )
        ri.Sys_Error (ERR_DROP, "R_BuildLightMap called for non-lit surface");

    smax = (surf->extents[0]>>4)+1;
    tmax = (surf->extents[1]>>4)+1;
    size = smax*tmax;
    if (size > (sizeof(s_blocklights)>>4) )
        ri.Sys_Error (ERR_DROP, "Bad s_blocklights size");

// set to full bright if no light data
    if (!surf->samples)
    {
        int maps;

        for (i=0 ; i<size*3 ; i++)
            s_blocklights[i] = 255;
        for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
             maps++)
        {
            //style = &r_newrefdef.lightstyles[surf->styles[maps]];
        }

        goto store;
    }

    // count the # of maps
    for ( nummaps = 0 ; nummaps < MAXLIGHTMAPS && surf->styles[nummaps] != 255 ;nummaps++)
    {

    }

    lightmap = surf->samples;

    // add all the lightmaps
    if ( nummaps == 1 )
    {
        int maps;

        for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
             maps++)
        {
            bl = s_blocklights;

            for (i=0 ; i<3 ; i++)
                scale[i] = 2.0f;//gl_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

            if ( scale[0] == 1.0F &&
                 scale[1] == 1.0F &&
                 scale[2] == 1.0F )
            {
                for (i=0 ; i<size ; i++, bl+=3)
                {
                    bl[0] = lightmap[i*3+0];
                    bl[1] = lightmap[i*3+1];
                    bl[2] = lightmap[i*3+2];
                }
            }
            else
            {
                for (i=0 ; i<size ; i++, bl+=3)
                {
                    bl[0] = lightmap[i*3+0] * scale[0];
                    bl[1] = lightmap[i*3+1] * scale[1];
                    bl[2] = lightmap[i*3+2] * scale[2];
                }
            }
            lightmap += size*3;		// skip to next lightmap
        }
    }
    else
    {
        int maps;

        memset( s_blocklights, 0, sizeof( s_blocklights[0] ) * size * 3 );

        for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
             maps++)
        {
            bl = s_blocklights;

            for (i=0 ; i<3 ; i++)
                scale[i] = 2.0f;//gl_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

            if ( scale[0] == 1.0F &&
                 scale[1] == 1.0F &&
                 scale[2] == 1.0F )
            {
                for (i=0 ; i<size ; i++, bl+=3 )
                {
                    bl[0] += lightmap[i*3+0];
                    bl[1] += lightmap[i*3+1];
                    bl[2] += lightmap[i*3+2];
                }
            }
            else
            {
                for (i=0 ; i<size ; i++, bl+=3)
                {
                    bl[0] += lightmap[i*3+0] * scale[0];
                    bl[1] += lightmap[i*3+1] * scale[1];
                    bl[2] += lightmap[i*3+2] * scale[2];
                }
            }
            lightmap += size*3;		// skip to next lightmap
        }
    }

// add all the dynamic lights
    //if (surf->dlightframe == r_framecount)
//        R_AddDynamicLights (surf);

// put into texture format
store:
    stride -= (smax<<2);
    bl = s_blocklights;

    //monolightmap = gl_monolightmap->string[0];

    if ( true )
    {
        for (i=0 ; i<tmax ; i++, dest += stride)
        {
            for (j=0 ; j<smax ; j++)
            {

                r = Q_ftol( bl[0] );
                g = Q_ftol( bl[1] );
                b = Q_ftol( bl[2] );

                // catch negative lights
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;

                /*
                ** determine the brightest of the three color components
                */
                if (r > g)
                    max = r;
                else
                    max = g;
                if (b > max)
                    max = b;

                /*
                ** alpha is ONLY used for the mono lightmap case.  For this reason
                ** we set it to the brightest of the color components so that
                ** things don't get too dim.
                */
                a = max;

                /*
                ** rescale all the color components if the intensity of the greatest
                ** channel exceeds 1.0
                */
                if (max > 255)
                {
                    float t = 255.0F / max;

                    r = r*t;
                    g = g*t;
                    b = b*t;
                    a = a*t;
                }

                dest[0] = r;
                dest[1] = g;
                dest[2] = b;
                dest[3] = a;

                bl += 3;
                dest += 4;
            }
        }
    }
    else
    {
        for (i=0 ; i<tmax ; i++, dest += stride)
        {
            for (j=0 ; j<smax ; j++)
            {

                r = Q_ftol( bl[0] );
                g = Q_ftol( bl[1] );
                b = Q_ftol( bl[2] );

                // catch negative lights
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;

                /*
                ** determine the brightest of the three color components
                */
                if (r > g)
                    max = r;
                else
                    max = g;
                if (b > max)
                    max = b;

                /*
                ** alpha is ONLY used for the mono lightmap case.  For this reason
                ** we set it to the brightest of the color components so that
                ** things don't get too dim.
                */
                a = max;

                /*
                ** rescale all the color components if the intensity of the greatest
                ** channel exceeds 1.0
                */
                if (max > 255)
                {
                    float t = 255.0F / max;

                    r = r*t;
                    g = g*t;
                    b = b*t;
                    a = a*t;
                }

                /*
                ** So if we are doing alpha lightmaps we need to set the R, G, and B
                ** components to 0 and we need to set alpha to 1-alpha.
                */
                switch ( monolightmap )
                {
                case 'L':
                case 'I':
                    r = a;
                    g = b = 0;
                    break;
                case 'C':
                    // try faking colored lighting
                    a = 255 - ((r+g+b)/3);
                    r *= a/255.0;
                    g *= a/255.0;
                    b *= a/255.0;
                    break;
                case 'A':
                default:
                    r = g = b = 0;
                    a = 255 - a;
                    break;
                }

                dest[0] = r;
                dest[1] = g;
                dest[2] = b;
                dest[3] = a;

                bl += 3;
                dest += 4;
            }
        }
    }
}


/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

static void LM_InitBlock( void )
{
    memset( gl_lms.allocated, 0, sizeof( gl_lms.allocated ) );
}

static void LM_UploadBlock( qboolean dynamic )
{
    int texture;
    int height = 0;

    if ( dynamic )
    {
        texture = 0;
    }
    else
    {
        texture = gl_lms.current_lightmap_texture;
    }

    //GL_Bind( gl_state.lightmap_textures + texture );
    //qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if ( dynamic )
    {
        int i;

        for ( i = 0; i < BLOCK_WIDTH; i++ )
        {
            if ( gl_lms.allocated[i] > height )
                height = gl_lms.allocated[i];
        }

        /*
        qglTexSubImage2D( GL_TEXTURE_2D,
                          0,
                          0, 0,
                          BLOCK_WIDTH, height,
                          GL_LIGHTMAP_FORMAT,
                          GL_UNSIGNED_BYTE,
                          gl_lms.lightmap_buffer );
        */
    }
    else
    {
        /*
        qglTexImage2D( GL_TEXTURE_2D,
                       0,
                       gl_lms.internal_format,
                       BLOCK_WIDTH, BLOCK_HEIGHT,
                       0,
                       GL_LIGHTMAP_FORMAT,
                       GL_UNSIGNED_BYTE,
                       gl_lms.lightmap_buffer );
        */
        void R_CreateLightmap(int id, int width, int height, unsigned char* data);
        R_CreateLightmap(gl_lms.current_lightmap_texture, BLOCK_WIDTH, BLOCK_HEIGHT, gl_lms.lightmap_buffer);
        if ( ++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS )
            ri.Sys_Error( ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n" );
    }
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock (int w, int h, int *x, int *y)
{
    int		i, j;
    int		best, best2;

    best = BLOCK_HEIGHT;

    for (i=0 ; i<BLOCK_WIDTH-w ; i++)
    {
        best2 = 0;

        for (j=0 ; j<w ; j++)
        {
            if (gl_lms.allocated[i+j] >= best)
                break;
            if (gl_lms.allocated[i+j] > best2)
                best2 = gl_lms.allocated[i+j];
        }
        if (j == w)
        {	// this is a valid spot
            *x = i;
            *y = best = best2;
        }
    }

    if (best + h > BLOCK_HEIGHT)
        return qfalse;

    for (i=0 ; i<w ; i++)
        gl_lms.allocated[*x + i] = best + h;

    return qtrue;
}

void GL_BuildPolygonFromSurface(msurface_t *fa)
{
    int			i, lindex, lnumverts;
    medge_t		*pedges, *r_pedge;
    int			vertpage;
    float		*vec;
    float		s, t;
    glpoly_t	*poly;
    vec3_t		total;

    // reconstruct the polygon
    pedges = currentmodel->edges;
    lnumverts = fa->numedges;
    vertpage = 0;

    VectorClear (total);
    //
    // draw texture
    //
    poly = (glpoly_t*) Hunk_Alloc (sizeof(glpoly_t) + (lnumverts-4) * VERTEXSIZE*sizeof(float));
    poly->next = fa->polys;
    poly->flags = fa->flags;
    fa->polys = poly;
    poly->numverts = lnumverts;

    for (i=0 ; i<lnumverts ; i++)
    {
        lindex = currentmodel->surfedges[fa->firstedge + i];

        if (lindex > 0)
        {
            r_pedge = &pedges[lindex];
            vec = currentmodel->vertexes[r_pedge->v[0]].position;
        }
        else
        {
            r_pedge = &pedges[-lindex];
            vec = currentmodel->vertexes[r_pedge->v[1]].position;
        }
        s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
        s /= fa->texinfo->image->width;

        t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
        t /= fa->texinfo->image->height;

        VectorAdd (total, vec, total);
        VectorCopy (vec, poly->verts[i]);
        poly->verts[i][3] = s;
        poly->verts[i][4] = t;

        //
        // lightmap texture coordinates
        //
        s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
        s -= fa->texturemins[0];
        s += fa->light_s*16;
        s += 8;
        s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

        t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
        t -= fa->texturemins[1];
        t += fa->light_t*16;
        t += 8;
        t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

        poly->verts[i][5] = s;
        poly->verts[i][6] = t;
    }

    poly->numverts = lnumverts;
}

void GL_CreateSurfaceLightmap (msurface_t *surf)
{

    int		smax, tmax;
    byte	*base;

    if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
        return;

    smax = (surf->extents[0]>>4)+1;
    tmax = (surf->extents[1]>>4)+1;

    if ( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ) )
    {
        LM_UploadBlock( qfalse );
        LM_InitBlock();
        if ( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ) )
        {
            ri.Sys_Error( ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax );
        }
    }

    surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

    base = gl_lms.lightmap_buffer;
    base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

    R_SetCacheState( surf );
    R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
}

void GL_BeginBuildingLightmaps (model_t *m)
{
    memset( gl_lms.allocated, 0, sizeof(gl_lms.allocated) );
    gl_lms.current_lightmap_texture = 0;
}

void GL_EndBuildingLightmaps (void)
{

}

msurface_t	*warpface;
#define	SUBDIVIDE_SIZE	64

void BoundPoly (int numverts, float *verts, vec3_t mins, vec3_t maxs)
{
    int		i, j;
    float	*v;

    mins[0] = mins[1] = mins[2] = 9999;
    maxs[0] = maxs[1] = maxs[2] = -9999;
    v = verts;
    for (i=0 ; i<numverts ; i++)
        for (j=0 ; j<3 ; j++, v++)
        {
            if (*v < mins[j])
                mins[j] = *v;
            if (*v > maxs[j])
                maxs[j] = *v;
        }
}

void SubdividePolygon (int numverts, float *verts)
{
    int		i, j, k;
    vec3_t	mins, maxs;
    float	m;
    float	*v;
    vec3_t	front[64], back[64];
    int		f, b;
    float	dist[64];
    float	frac;
    glpoly_t	*poly;
    float	s, t;
    vec3_t	total;
    float	total_s, total_t;

    if (numverts > 60)
        ri.Sys_Error (ERR_DROP, "numverts = %i", numverts);

    BoundPoly (numverts, verts, mins, maxs);

    for (i=0 ; i<3 ; i++)
    {
        m = (mins[i] + maxs[i]) * 0.5;
        m = SUBDIVIDE_SIZE * floor (m/SUBDIVIDE_SIZE + 0.5);
        if (maxs[i] - m < 8)
            continue;
        if (m - mins[i] < 8)
            continue;

        // cut it
        v = verts + i;
        for (j=0 ; j<numverts ; j++, v+= 3)
            dist[j] = *v - m;

        // wrap cases
        dist[j] = dist[0];
        v-=i;
        VectorCopy (verts, v);

        f = b = 0;
        v = verts;
        for (j=0 ; j<numverts ; j++, v+= 3)
        {
            if (dist[j] >= 0)
            {
                VectorCopy (v, front[f]);
                f++;
            }
            if (dist[j] <= 0)
            {
                VectorCopy (v, back[b]);
                b++;
            }
            if (dist[j] == 0 || dist[j+1] == 0)
                continue;
            if ( (dist[j] > 0) != (dist[j+1] > 0) )
            {
                // clip point
                frac = dist[j] / (dist[j] - dist[j+1]);
                for (k=0 ; k<3 ; k++)
                    front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
                f++;
                b++;
            }
        }

        SubdividePolygon (f, front[0]);
        SubdividePolygon (b, back[0]);
        return;
    }

    // add a point in the center to help keep warp valid
    poly = (glpoly_t*) Hunk_Alloc (sizeof(glpoly_t) + ((numverts-4)+2) * VERTEXSIZE*sizeof(float));
    poly->next = warpface->polys;
    warpface->polys = poly;
    poly->numverts = numverts+2;
    VectorClear (total);
    total_s = 0;
    total_t = 0;
    for (i=0 ; i<numverts ; i++, verts+= 3)
    {
        VectorCopy (verts, poly->verts[i+1]);
        s = DotProduct (verts, warpface->texinfo->vecs[0]);
        t = DotProduct (verts, warpface->texinfo->vecs[1]);

        total_s += s;
        total_t += t;
        VectorAdd (total, verts, total);

        poly->verts[i+1][3] = s;
        poly->verts[i+1][4] = t;
    }

    VectorScale (total, (1.0/numverts), poly->verts[0]);
    poly->verts[0][3] = total_s/numverts;
    poly->verts[0][4] = total_t/numverts;

    // copy first vertex to last
    memcpy (poly->verts[i+1], poly->verts[1], sizeof(poly->verts[0]));
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface (msurface_t *fa)
{
    vec3_t		verts[64];
    int			numverts;
    int			i;
    int			lindex;
    float		*vec;

    warpface = fa;

    //
    // convert edges back to a normal polygon
    //
    numverts = 0;
    for (i=0 ; i<fa->numedges ; i++)
    {
        lindex = loadmodel->surfedges[fa->firstedge + i];

        if (lindex > 0)
            vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
        else
            vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
        VectorCopy (vec, verts[numverts]);
        numverts++;
    }

    SubdividePolygon (numverts, verts[0]);
}
