
#include "TBEImage.h"
#include "Engine.h"
#include "FileSystem.h"
#include "ResourceCache.h"
#include "TBESystem.h"

// this should not be being used, as we will default to this texture
// keep r_notexture for now as it is referenced externally
image_t *r_notexture = NULL;
static HashMap<String, image_t*> textureLookup;

extern refimport_t ri;

unsigned	d_8to24table[256];

void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height)
{
    byte	*raw;
    pcx_t	*pcx;
    int		x, y;
    int		len;
    int		dataByte, runLength;
    byte	*out, *pix;

    *pic = NULL;
    *palette = NULL;

    //
    // load the file
    //
    len = ri.FS_LoadFile (filename, (void **)&raw);
    if (!raw)
    {
        ri.Con_Printf (PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
        return;
    }

    //
    // parse the PCX file
    //
    pcx = (pcx_t *)raw;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

    raw = &pcx->data;

    if (pcx->manufacturer != 0x0a
        || pcx->version != 5
        || pcx->encoding != 1
        || pcx->bits_per_pixel != 8
        || pcx->xmax >= 640
        || pcx->ymax >= 480)
    {
        ri.Con_Printf (PRINT_ALL, "Bad pcx file %s\n", filename);
        return;
    }

    out = (byte*) malloc ( (pcx->ymax+1) * (pcx->xmax+1) );

    *pic = out;

    pix = out;

    if (palette)
    {
        *palette = (byte *) malloc(768);
        memcpy (*palette, (byte *)pcx + len - 768, 768);
    }

    if (width)
        *width = pcx->xmax+1;
    if (height)
        *height = pcx->ymax+1;

    for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1)
    {
        for (x=0 ; x<=pcx->xmax ; )
        {
            dataByte = *raw++;

            if((dataByte & 0xC0) == 0xC0)
            {
                runLength = dataByte & 0x3F;
                dataByte = *raw++;
            }
            else
                runLength = 1;

            while(runLength-- > 0)
                pix[x++] = dataByte;
        }

    }

    if ( raw - (byte *)pcx > len)
    {
        ri.Con_Printf (PRINT_DEVELOPER, "PCX file %s was malformed", filename);
        free (*pic);
        *pic = NULL;
    }

    ri.FS_FreeFile (pcx);
}

/*
===============
Draw_GetPalette
===============
*/
int R_GetPalette (void)
{
    int		i;
    int		r, g, b;
    unsigned	v;
    byte	*pic, *pal;
    int		width, height;

    // get the palette

    LoadPCX ("pics/colormap.pcx", &pic, &pal, &width, &height);
    if (!pal)
        ri.Sys_Error (ERR_FATAL, "Couldn't load pics/colormap.pcx");

    for (i=0 ; i<256 ; i++)
    {
        r = pal[i*3+0];
        g = pal[i*3+1];
        b = pal[i*3+2];

        v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
        d_8to24table[i] = LittleLong(v);
    }

    d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

    free (pic);
    free (pal);

    return 0;
}


void GL_InitImages ()
{
    R_GetPalette();
}

static void GL_GetWalSize (char *name, int& width, int &height)
{
    miptex_t	*mt;

    ri.FS_LoadFile (name, (void **)&mt);
    if (!mt)
    {
        ri.Con_Printf (PRINT_ALL, "GL_GetWalSize: can't load %s\n", name);
        return;
    }

    width = LittleLong (mt->width);
    height = LittleLong (mt->height);
    ri.FS_FreeFile ((void *)mt);
}


image_t	*GL_FindImage (char *name, imagetype_t type)
{
    if (!textureLookup.Contains(name))
    {
        String pathName;
        String fileName;
        String extension;
        SplitPath(name, pathName, fileName, extension);

        String textureName = "Textures/"+ fileName + ".tga";

        if (extension == ".pcx")
        {
            textureName = String("baseq2/") + pathName + fileName + ".jpg";

        }

        ResourceCache* cache = TBESystem::GetEngine()->GetSubsystem<ResourceCache>();

        Texture2D* texture = cache->GetResource<Texture2D>(textureName);

        if (!texture)
        {
            texture = cache->GetResource<Texture2D>("Textures/UrhoIcon.png");
        }

        image_t* image = new image_t;
        image->texture = texture;

        int width = texture->GetWidth();
        int height = texture->GetHeight();

        if (extension == ".wal")
        {
            GL_GetWalSize(name, width, height);
        }

        image->width = width;
        image->height = height;

        textureLookup.Insert(MakePair(String(name), image));
    }

    return textureLookup.Find(name)->second_;

}

void GL_FreeUnusedImages()
{

}
