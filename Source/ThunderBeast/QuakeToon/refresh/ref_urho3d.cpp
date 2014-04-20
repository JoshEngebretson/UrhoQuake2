
#include "Camera.h"
#include "CoreEvents.h"
#include "Engine.h"
#include "Font.h"
#include "Graphics.h"
#include "Input.h"
#include "Material.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Geometry.h"
#include "Model.h"
#include "Octree.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "Scene.h"
#include "Zone.h"
#include "StaticModel.h"
#include "Text.h"
#include "UI.h"
#include "DebugRenderer.h"

#include "DebugNew.h"

#include "../system/sys_urho3d.h"

#include "ref_urho3d.h"
extern "C" {
#include "ref_local.h"
#include "ref_model.h"
#include "ref_image.h"
}

using namespace Urho3D;

Q2Renderer::Q2Renderer(Context* context) : Object(context)
{

}

void Q2Renderer::InitializeWorldModel()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create the Octree component to the scene. This is required before adding any drawable components, or else nothing will
    // show up. The default octree volume will be from (-1000, -1000, -1000) to (1000, 1000, 1000) in world coordinates; it
    // is also legal to place objects outside the volume but their visibility can then not be checked in a hierarchically
    // optimizing manner
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    SharedPtr<Model> world(new Model(context_));

    SharedPtr<IndexBuffer> ib;
    SharedPtr<VertexBuffer> vb;

    vb = new VertexBuffer(context_);
    ib = new IndexBuffer(context_);

    // going to need normal
    unsigned elementMask = MASK_POSITION | MASK_NORMAL | MASK_TEXCOORD1;// | MASK_TEXCOORD2;

    // count the vertices and indices
    int numvertices = 0;
    int numpolys = 0;
    msurface_t* surf = r_worldmodel->surfaces;
    for (int i = 0; i < r_worldmodel->numsurfaces; i++, surf++)
    {
        glpoly_t* poly = surf->polys;

        while(poly)
        {
            numvertices += poly->numverts;
            numpolys += poly->numverts - 2;
            poly = poly->next;
        }
    }

    vb->SetSize(numvertices, elementMask);
    ib->SetSize(numpolys * 3, false);

    int vcount = 0;
    float* vertexData = (float *) vb->Lock(0, numvertices);
    unsigned short* indexData = (unsigned short*) ib->Lock(0, numpolys * 3);

    surf = r_worldmodel->surfaces;

    for (int i = 0; i < r_worldmodel->numsurfaces; i++, surf++)
    {
        glpoly_t* poly = surf->polys;

        while(poly)
        {
            // GL POLYGON

            // copy vertex data into vertex buffer
            for (int j = 0; j < poly->numverts; j++)
            {
                //printf("%f, %f, %f\n", poly->verts[j][0], poly->verts[j][1], poly->verts[j][2]);
                *vertexData = poly->verts[j][0]; vertexData++; // x
                *vertexData = poly->verts[j][1]; vertexData++; // y
                *vertexData = poly->verts[j][2]; vertexData++; // z

                // fake normal
                *vertexData = 0; vertexData++;
                *vertexData = 1; vertexData++;
                *vertexData = 0; vertexData++;

                *vertexData = poly->verts[j][3]; vertexData++; // u0
                *vertexData = poly->verts[j][4]; vertexData++; // v0
                //*vertexData = poly->verts[j][5]; vertexData++; // u1
                //*vertexData = poly->verts[j][6]; vertexData++; // v1
            }

            int c = 0;
            for (int j = 0; j < poly->numverts - 1; j += 2)
            {
                *indexData = vcount + j; indexData++;
                *indexData = vcount + ((j + 1) % poly->numverts); indexData++;
                *indexData = vcount + ((j + 2) % poly->numverts); indexData++;
                c+=3;
            }

            vcount += c;


            poly = poly->next;
        }

    }

    vb->Unlock();
    ib->Unlock();


    world->SetNumGeometries(1);

    SharedPtr<Geometry> geom(new Geometry(context_));

    geom->SetIndexBuffer(ib);
    geom->SetVertexBuffer(0, vb, elementMask);
    geom->SetDrawRange(TRIANGLE_LIST, 0, numpolys * 3, true);
    world->SetNumGeometryLodLevels(0, 1);
    world->SetGeometry(0, 0, geom);


    world->SetBoundingBox(BoundingBox(Vector3(-10000, -10000, -10000),  Vector3(10000, 10000, 10000)));

    Node* worldNode = scene_->CreateChild("World");
    worldNode->SetScale(1);
    StaticModel* worldObject = worldNode->CreateComponent<StaticModel>();
    worldObject->SetModel(world);
    worldObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    // Set same volume as the Octree, set a close bluish fog and some ambient light
    zone->SetBoundingBox(BoundingBox(Vector3(-10000, -10000, -10000),  Vector3(10000, 10000, 10000)));
    zone->SetAmbientColor(Color(1, 1, 1));
    zone->SetFogStart(10000);
    zone->SetFogEnd(10000);

    printf("%i\n", numpolys * 3);

    // Create a directional light to the world so that we can see something. The light scene node's orientation controls the
    // light direction; we will use the SetDirection() function which calculates the orientation from a forward direction vector.
    // The light will use default settings (white light, no shadows)
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f)); // The direction vector does not need to be normalized
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);

    // Create a scene node for the camera, which we will move around
    // The camera will use default settings (1000 far clip distance, 45 degrees FOV, set aspect ratio automatically)
    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(65000.0f);

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(128, -320, -1000));

    Renderer* renderer = GetSubsystem<Renderer>();


    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
    // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
    // use, but now we just use full screen and default render path configured in the engine command line options
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

    // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, during which we request
    // debug geometry
    SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(Q2Renderer, HandlePostRenderUpdate));


}

void Q2Renderer::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->DrawDebugGeometry(false);
}

extern "C"
{

static Q2Renderer* gRender = NULL;

void R_SetupUrhoScene()
{
    gRender->InitializeWorldModel();
}

byte	dottexture[8][8] =
{
    {0,0,0,0,0,0,0,0},
    {0,0,1,1,0,0,0,0},
    {0,1,1,1,1,0,0,0},
    {0,1,1,1,1,0,0,0},
    {0,0,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
};

void R_InitParticleTexture (void)
{
    int		x,y;
    byte	data[8][8][4];

    //
    // particle texture
    //
    for (x=0 ; x<8 ; x++)
    {
        for (y=0 ; y<8 ; y++)
        {
            data[y][x][0] = 255;
            data[y][x][1] = 255;
            data[y][x][2] = 255;
            data[y][x][3] = dottexture[x][y]*255;
        }
    }

    //r_particletexture = GL_LoadPic ("***particle***", (byte *)data, 8, 8, it_sprite, 32);

    //
    // also use this for bad textures, but without alpha
    //
    for (x=0 ; x<8 ; x++)
    {
        for (y=0 ; y<8 ; y++)
        {
            data[y][x][0] = dottexture[x&3][y&3]*255;
            data[y][x][1] = 0; // dottexture[x&3][y&3]*255;
            data[y][x][2] = 0; //dottexture[x&3][y&3]*255;
            data[y][x][3] = 255;
        }
    }

    r_notexture = GL_LoadPic ("***r_notexture***", (byte *)data, 8, 8, it_wall, 32);
}


qboolean R_Init( void *hinstance, void *hWnd )
{

    GL_InitImages ();
    Mod_Init ();
    R_InitParticleTexture ();

    gRender = new Q2Renderer(Q2System::GetContext());
    return qtrue;
}

refimport_t	ri;

void R_BeginRegistration (char *model);
struct model_s	*R_RegisterModel (char *name);
struct image_s	*R_RegisterSkin (char *name);
void R_EndRegistration (void);

void R_SetSky (char *name, float rotate, vec3_t axis)
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


