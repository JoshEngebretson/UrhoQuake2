
#include "TBEModelLoad.h"

refimport_t	ri;

extern "C"
{

#include "../../client/ref.h"

qboolean R_Init( void *hinstance, void *hWnd )
{
    GL_InitImages ();
    Mod_Init ();

    return qtrue;
}

void R_SetSky (char *name, float rotate, vec3_t axis)
{

}

struct image_s	*R_RegisterSkin (char *name)
{
    return NULL;
}


void	R_RenderFrame (refdef_t *fd);

struct image_s *Draw_FindPic (char *name)
{
    return NULL;
}

void Draw_Pic (int x, int y, char *name)
{

}

void Draw_Char (int x, int y, int c)
{

}

void Draw_TileClear (int x, int y, int w, int h, char *name)
{

}

void Draw_Fill (int x, int y, int w, int h, int c)
{

}

void Draw_FadeScreen (void)
{

}

void    Draw_GetPicSize (int *w, int *h, char *pic)
{

}

void    Draw_StretchPic (int x, int y, int w, int h, char *pic)
{

}

void    Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{

}

void R_Shutdown (void)
{

}

void R_SetPalette ( const unsigned char *palette)
{

}

void R_BeginFrame( float camera_separation )
{

}

void GLimp_EndFrame (void)
{

}

void GLimp_AppActivate( qboolean active )
{

}

refexport_t GetRefAPI (refimport_t rimp )
{
    refexport_t	re;

    ri = rimp;

    re.api_version = API_VERSION;

    re.BeginRegistration = R_BeginRegistration;
    re.RegisterModel = R_RegisterModel;
    re.RegisterSkin = R_RegisterSkin;
    re.RegisterPic = Draw_FindPic;
    re.SetSky = R_SetSky;
    re.EndRegistration = R_EndRegistration;

    re.RenderFrame = R_RenderFrame;

    re.DrawGetPicSize = Draw_GetPicSize;
    re.DrawPic = Draw_Pic;
    re.DrawStretchPic = Draw_StretchPic;
    re.DrawChar = Draw_Char;
    re.DrawTileClear = Draw_TileClear;
    re.DrawFill = Draw_Fill;
    re.DrawFadeScreen= Draw_FadeScreen;

    re.DrawStretchRaw = Draw_StretchRaw;

    re.Init = R_Init;
    re.Shutdown = R_Shutdown;

    re.CinematicSetPalette = R_SetPalette;
    re.BeginFrame = R_BeginFrame;
    re.EndFrame = GLimp_EndFrame;

    re.AppActivate = GLimp_AppActivate;

    Swap_Init ();

    return re;
}

}
