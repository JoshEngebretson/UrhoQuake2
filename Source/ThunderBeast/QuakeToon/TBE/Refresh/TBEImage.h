
#pragma once

#include "Texture2D.h"

using namespace Urho3D;

extern "C"
{
    #include "../../client/ref.h"
}

#define	TEXNUM_LIGHTMAPS	1024
#define	TEXNUM_SCRAPS		1152
#define	TEXNUM_IMAGES		1153
#define MAX_GLTEXTURES	    1024

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
    Texture2D* texture;
    int		width, height;				// source image
    int		registration_sequence;		// 0 = free
} image_t;

void GL_InitImages ();
image_t	*GL_FindImage (char *name, imagetype_t type);
void GL_FreeUnusedImages();

extern image_t *r_notexture;
