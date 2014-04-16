/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// vid_null.c -- null video driver to aid porting efforts
// this assumes that one of the refs is statically linked to the executable

#include "../client/client.h"

viddef_t	viddef;				// global video state

refexport_t	re;

/*
==========================================================================

DIRECT LINK GLUE

==========================================================================
*/

#define	MAXPRINTMSG	4096
void VID_Printf (int print_level, char *fmt, ...)
{
        va_list		argptr;
        char		msg[MAXPRINTMSG];

        va_start (argptr,fmt);
        vsprintf (msg,fmt,argptr);
        va_end (argptr);

        if (print_level == PRINT_ALL)
                Com_Printf ("%s", msg);
        else
                Com_DPrintf ("%s", msg);
}

void VID_Error (int err_level, char *fmt, ...)
{
        va_list		argptr;
        char		msg[MAXPRINTMSG];

        va_start (argptr,fmt);
        vsprintf (msg,fmt,argptr);
        va_end (argptr);

		Com_Error (err_level, "%s", msg);
}

void VID_NewWindow (int width, int height)
{
        viddef.width = width;
        viddef.height = height;
}

/*
** VID_GetModeInfo
*/
typedef struct vidmode_s
{
    const char *description;
    int         width, height;
    int         mode;
} vidmode_t;

vidmode_t vid_modes[] =
{
    { "Mode 0: 320x240",   320, 240,   0 },
    { "Mode 1: 400x300",   400, 300,   1 },
    { "Mode 2: 512x384",   512, 384,   2 },
    { "Mode 3: 640x480",   640, 480,   3 },
    { "Mode 4: 800x600",   800, 600,   4 },
    { "Mode 5: 960x720",   960, 720,   5 },
    { "Mode 6: 1024x768",  1024, 768,  6 },
    { "Mode 7: 1152x864",  1152, 864,  7 },
    { "Mode 8: 1280x960",  1280, 960, 8 },
    { "Mode 9: 1600x1200", 1600, 1200, 9 },
	{ "Mode 10: 2048x1536", 2048, 1536, 10 }
};
#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

qboolean VID_GetModeInfo( int *width, int *height, int mode )
{
    if ( mode < 0 || mode >= VID_NUM_MODES )
        return false;

    *width  = vid_modes[mode].width;
    *height = vid_modes[mode].height;

    return true;
}

void	R_BeginRegistration (char *map)
{

}

struct model_s	*R_RegisterModel (char *name)
{
    return NULL;
}

struct image_s	*R_RegisterSkin (char *name)
{
    return NULL;
}

void R_SetSky (char *name, float rotate, vec3_t axis)
{

}

void	R_EndRegistration (void)
{

}

void	R_RenderFrame (refdef_t *fd)
{

}

struct image_s	*Draw_FindPic (char *name)
{

}

void	Draw_Pic (int x, int y, char *name)
{

}

void	Draw_Char (int x, int y, int c)
{

}

void	Draw_TileClear (int x, int y, int w, int h, char *name)
{

}

void	Draw_Fill (int x, int y, int w, int h, int c)
{

}

void	Draw_FadeScreen (void)
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

qboolean R_Init( void *hinstance, void *hWnd )
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

void GLimp_EndFrame (void);
void GLimp_AppActivate( qboolean active );

refexport_t GetRefAPI (refimport_t rimp )
{
    refexport_t	re;

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

void	VID_Init (void)
{
    refimport_t	ri;

    viddef.width = 320;
    viddef.height = 240;

    ri.Cmd_AddCommand = Cmd_AddCommand;
    ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
    ri.Cmd_Argc = Cmd_Argc;
    ri.Cmd_Argv = Cmd_Argv;
    ri.Cmd_ExecuteText = Cbuf_ExecuteText;
    ri.Con_Printf = VID_Printf;
    ri.Sys_Error = VID_Error;
    ri.FS_LoadFile = FS_LoadFile;
    ri.FS_FreeFile = FS_FreeFile;
    ri.FS_Gamedir = FS_Gamedir;
	ri.Vid_NewWindow = VID_NewWindow;
    ri.Cvar_Get = Cvar_Get;
    ri.Cvar_Set = Cvar_Set;
    ri.Cvar_SetValue = Cvar_SetValue;
    ri.Vid_GetModeInfo = VID_GetModeInfo;

    re = GetRefAPI(ri);

    if (re.api_version != API_VERSION)
        Com_Error (ERR_FATAL, "Re has incompatible api_version");
    
        // call the init function
    if (re.Init (NULL, NULL) == -1)
		Com_Error (ERR_FATAL, "Couldn't start refresh");
}

void	VID_Shutdown (void)
{
    if (re.Shutdown)
	    re.Shutdown ();
}

void	VID_CheckChanges (void)
{
}

void	VID_MenuInit (void)
{
}

void	VID_MenuDraw (void)
{
}

const char *VID_MenuKey( int k)
{
	return NULL;
}
