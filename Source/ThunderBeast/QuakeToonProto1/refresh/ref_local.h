

#include "../client/ref.h"

/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/

#pragma once

#define	TEXNUM_LIGHTMAPS	1024
#define	TEXNUM_SCRAPS		1152
#define	TEXNUM_IMAGES		1153

#define		MAX_GLTEXTURES	1024

typedef enum
{
    it_skin,
    it_sprite,
    it_wall,
    it_pic,
    it_sky
} imagetype_t;

typedef struct image_s
{
    char	name[MAX_QPATH];			// game path, including extension
    imagetype_t	type;
    int		width, height;				// source image
    int		upload_width, upload_height;	// after power of two and picmip
    int		registration_sequence;		// 0 = free
    struct msurface_s	*texturechain;	// for sort-by-texture world drawing
    int		texnum;						// gl texture binding
    float	sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
    qboolean	scrap;
    qboolean	has_alpha;

    qboolean paletted;
} image_t;

extern refimport_t	ri;
extern	int		registration_sequence;
extern	image_t	*r_notexture;
extern	int	r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;
