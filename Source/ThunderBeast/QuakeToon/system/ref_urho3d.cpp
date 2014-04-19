
#include "Camera.h"
#include "CoreEvents.h"
#include "Engine.h"
#include "Font.h"
#include "Graphics.h"
#include "Input.h"
#include "Material.h"
#include "Model.h"
#include "Octree.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "Scene.h"
#include "StaticModel.h"
#include "Text.h"
#include "UI.h"

#include "DebugNew.h"

#include "sys_urho3d.h"
#include "ref_urho3d.h"

using namespace Urho3D;

Q2Renderer::Q2Renderer(Context* context) : Object(context)
{

}

void Q2Renderer::Initialize()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create the Octree component to the scene. This is required before adding any drawable components, or else nothing will
    // show up. The default octree volume will be from (-1000, -1000, -1000) to (1000, 1000, 1000) in world coordinates; it
    // is also legal to place objects outside the volume but their visibility can then not be checked in a hierarchically
    // optimizing manner
    scene_->CreateComponent<Octree>();

    // Create a child scene node (at world origin) and a StaticModel component into it. Set the StaticModel to show a simple
    // plane mesh with a "stone" material. Note that naming the scene nodes is optional. Scale the scene node larger
    // (100 x 100 world units)
    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(100.0f, 1.0f, 100.0f));
    StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

    // Create a directional light to the world so that we can see something. The light scene node's orientation controls the
    // light direction; we will use the SetDirection() function which calculates the orientation from a forward direction vector.
    // The light will use default settings (white light, no shadows)
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f)); // The direction vector does not need to be normalized
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);

    // Create more StaticModel objects to the scene, randomly positioned, rotated and scaled. For rotation, we construct a
    // quaternion from Euler angles where the Y angle (rotation about the Y axis) is randomized. The mushroom model contains
    // LOD levels, so the StaticModel component will automatically select the LOD level according to the view distance (you'll
    // see the model get simpler as it moves further away). Finally, rendering a large number of the same object with the
    // same material allows instancing to be used, if the GPU supports it. This reduces the amount of CPU work in rendering the
    // scene.
    const unsigned NUM_OBJECTS = 200;
    for (unsigned i = 0; i < NUM_OBJECTS; ++i)
    {
        Node* mushroomNode = scene_->CreateChild("Mushroom");
        mushroomNode->SetPosition(Vector3(Random(90.0f) - 45.0f, 0.0f, Random(90.0f) - 45.0f));
        mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        mushroomNode->SetScale(0.5f + Random(2.0f));
        StaticModel* mushroomObject = mushroomNode->CreateComponent<StaticModel>();
        mushroomObject->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
        mushroomObject->SetMaterial(cache->GetResource<Material>("Materials/Mushroom.xml"));
    }

    // Create a scene node for the camera, which we will move around
    // The camera will use default settings (1000 far clip distance, 45 degrees FOV, set aspect ratio automatically)
    cameraNode_ = scene_->CreateChild("Camera");
    cameraNode_->CreateComponent<Camera>();

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
    // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
    // use, but now we just use full screen and default render path configured in the engine command line options
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

}

extern "C"
{

#include "../client/client.h"

viddef_t	viddef;				// global video state
refexport_t	re;

static Q2Renderer* gRender = NULL;

qboolean R_Init( void *hinstance, void *hWnd )
{
    gRender = new Q2Renderer(Q2System::GetContext());
    gRender->Initialize();
    return qtrue;
}

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
        return qfalse;

    *width  = vid_modes[mode].width;
    *height = vid_modes[mode].height;

    return qtrue;
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
    if (re.Init (NULL, NULL) == false)
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

}


