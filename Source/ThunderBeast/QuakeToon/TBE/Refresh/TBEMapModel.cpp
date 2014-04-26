
#include "StringUtils.h"
#include "ProcessUtils.h"
#include "Context.h"
#include "Material.h"
#include "Model.h"
#include "Geometry.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Technique.h"
#include "FileSystem.h"
#include "Graphics.h"
#include "ResourceCache.h"
#include "StaticModel.h"
#include "TBESystem.h"

// move me
#include "Scene.h"
#include "Octree.h"
#include "Node.h"
#include "Camera.h"
#include "Zone.h"
#include "Renderer.h"
#include "DebugRenderer.h"

#include "TBEMapModel.h"


extern model_t *r_worldmodel;

SharedPtr<Scene> scene_;
SharedPtr<Node> cameraNode_;

// a surface may be contained in more than one cluster

struct RenderCluster
{
    PODVector<msurface_t*> surfaces;
};

static PODVector<Node*> worldNodes;

static HashMap<msurface_t*, PODVector<Node*> > surfaceNodes;

static HashMap<model_t*, Node* > brushNodes;

static Vector<RenderCluster> renderClusters;
PODVector<Texture2D*> lightmapTextures;

static HashMap<String, SharedPtr<Material> > materialLookup;

static HashMap<Material*, PODVector<msurface_t*> > surfaceMap;

static float _scale = .1f;

static Material* LoadMaterial(int lightmap, const String& name, msurface_t* surface)
{
    Context* context = TBESystem::GetGlobalContext();

    if (!materialLookup.Contains(name))
    {
        ResourceCache* cache = context->GetSubsystem<ResourceCache>();

        if (name.Find("bluwter") != String::NPOS)
        {
            Texture* texture = surface->texinfo->image->texture;
            SharedPtr<Material> material = SharedPtr<Material>(cache->GetResource<Material>("Materials/Water.xml"));
            material->SetTexture(TU_DIFFUSE, texture);
            material->SetName(name);
            material->SetShaderParameter("NoiseTiling", .002f);
            material->SetShaderParameter("NoiseStrength", .04f);
            material->SetShaderParameter("WaterTint", Vector3(0.7f,0.7f,1.0f));
            materialLookup.Insert(MakePair(name, material));
        }
        else
        {
            Texture* texture = surface->texinfo->image->texture;

            SharedPtr<Material> material = SharedPtr<Material>(new Material(context));
            Technique* technique;
            bool trans = surface->texinfo->flags & SURF_TRANS33 || surface->texinfo->flags & SURF_TRANS66;
            if (!trans)
            {
                technique = cache->GetResource<Technique>("Techniques/DiffLightMap.xml");
            }
            else
            {
                technique = cache->GetResource<Technique>("Techniques/DiffAlpha.xml");
                material->SetShaderParameter("MatDiffColor", Vector4(1,1,1, .2f));
            }

            material->SetNumTechniques(1);
            material->SetTechnique(0, technique);
            material->SetName(name);
            material->SetTexture(TU_DIFFUSE, texture);

            if (lightmap < lightmapTextures.Size())
                material->SetTexture(TU_EMISSIVE, lightmapTextures[lightmap]);
            else
            {
                // I believe this is in the case of moving brush models
                // just set here, renders wrong but doesn't flicker
                material->SetTexture(TU_EMISSIVE, texture);
            }

            materialLookup.Insert(MakePair(name, material));

        }
    }

    return materialLookup.Find(name)->second_;

}

static Node* EmitBrushModel(const HashMap<Material*, PODVector<msurface_t*> >& materialMap, bool isWorld = true)
{
    Context* context = TBESystem::GetGlobalContext();
    Vector<Geometry*> submeshes;
    Vector<Material*> materials;
    Vector<Vector3> centers;

    BoundingBox bbox;

    for (HashMap<Material*, PODVector<msurface_t*> >::ConstIterator i = materialMap.Begin(); i != materialMap.End(); ++i)
    {
        materials.Push(i->first_);

        const PODVector<msurface_t*>& surfaces = i->second_;

        // count the vertices and indices
        int numvertices = 0;
        int numpolys = 0;

        for (unsigned i = 0; i < surfaces.Size(); i++)
        {
            const msurface_t* surf = surfaces[i];
            glpoly_t* poly = surf->polys;

            while(poly)
            {
                numvertices += poly->numverts;
                numpolys += poly->numverts - 2;
                poly = poly->next;
            }
        }

        SharedPtr<IndexBuffer> ib;
        SharedPtr<VertexBuffer> vb;

        // TODO: share vertex buffers
        vb = new VertexBuffer(context);
        ib = new IndexBuffer(context);

        // going to need normal
        unsigned elementMask = MASK_POSITION  | MASK_NORMAL| MASK_TEXCOORD1  | MASK_TEXCOORD2;// | MASK_TANGENT;//;

        vb->SetSize(numvertices, elementMask);
        ib->SetSize(numpolys * 3, false);

        int vcount = 0;
        float* vertexData = (float *) vb->Lock(0, numvertices);
        unsigned short* indexData = (unsigned short*) ib->Lock(0, numpolys * 3);

        Vector3 center = Vector3::ZERO;



        for (unsigned i = 0; i < surfaces.Size(); i++)
        {
            const msurface_t* surf = surfaces[i];
            glpoly_t* poly = surf->polys;

            // use the plane normal, this will result in faceted lighting
            // we need to calculate vertex normals properly, however this
            // is complicated by Quake2's file format not having adjacency info
            Vector3 normal(surf->plane->normal[0], surf->plane->normal[2], surf->plane->normal[1]);
            if (surf->flags & SURF_PLANEBACK)
                normal = -normal;

            while(poly)
            {
                // copy vertex data into vertex buffer
                for (int j = 0; j < poly->numverts; j++)
                {
                    bbox.Merge(Vector3(poly->verts[j][0] * _scale, poly->verts[j][2] * _scale, poly->verts[j][1] * _scale));
                    *vertexData++ = poly->verts[j][0] * _scale; // x
                    *vertexData++ = poly->verts[j][2] * _scale; // y
                    *vertexData++ = poly->verts[j][1] * _scale ; // z

                    center.x_ += poly->verts[j][0];
                    center.y_ += poly->verts[j][2];
                    center.z_ += poly->verts[j][1];

                    *vertexData++ = normal.x_;
                    *vertexData++ = normal.y_;
                    *vertexData++ = normal.z_;

                    *vertexData++ = poly->verts[j][3];    // u0
                    *vertexData++ = poly->verts[j][4];    // v0
                    *vertexData++ = poly->verts[j][5]; // u1
                    *vertexData++ = poly->verts[j][6]; // v1

                    // Tangent
                    //#undef DotProduct // fix me, coming in from Quake2
                    //Vector3 xyz = (Vector3::RIGHT - normal * normal.DotProduct(Vector3::RIGHT)).Normalized();
                    //*vertexData++ = xyz.x_;
                    //*vertexData++ = xyz.y_;
                    //*vertexData++ = xyz.z_;
                    //*vertexData++ = 1.0f;

                }

                for (int j = 0; j < poly->numverts - 2; j++)
                {
                    *indexData = vcount; indexData++;
                    *indexData = vcount + j + 1; indexData++;
                    *indexData = vcount + j + 2; indexData++;
                }

                vcount += poly->numverts;

                poly = poly->next;            }
            }

        vb->Unlock();
        ib->Unlock();

        center /= numpolys * 3;

        centers.Push(center);

        Geometry* geom = new Geometry(context);

        geom->SetIndexBuffer(ib);
        geom->SetVertexBuffer(0, vb, elementMask);
        geom->SetDrawRange(TRIANGLE_LIST, 0, numpolys * 3, false);
        submeshes.Push(geom);
    }

    SharedPtr<Model> world(new Model(context));

    world->SetNumGeometries(submeshes.Size());

    for (unsigned i = 0; i < submeshes.Size(); i++)
    {
        world->SetNumGeometryLodLevels(i, 1);
        world->SetGeometry(i, 0, submeshes[i]);
        //world->SetGeometryCenter(i, centers[i]);
    }

    world->SetBoundingBox(bbox);

    Node* worldNode = scene_->CreateChild("World");
    StaticModel* worldObject = worldNode->CreateComponent<StaticModel>();
    worldObject->SetCastShadows(false);
    worldObject->SetModel(world);
    for (unsigned i = 0; i < materials.Size(); i++)
    {
        worldObject->SetMaterial(i, materials[i]);
    }

    if (isWorld)
        worldNodes.Push(worldNode);

    // clear for next model
    surfaceMap.Clear();

    return worldNode;
}

static void MapSurface(msurface_t* surface)
{
    String name(surface->texinfo->image->texture->GetName());

    if (name.Find("trigger") != String::NPOS)
        return;

    if (name.Find("sky") != String::NPOS)
        return;

    name += "_LM" + String(surface->lightmaptexturenum);

    Material* material = LoadMaterial(surface->lightmaptexturenum, name, surface);

    if (!surfaceMap.Contains(material))
    {
        surfaceMap.Insert(MakePair(material, PODVector<msurface_t*>()));
    }

    surfaceMap.Find(material)->second_.Push(surface);

}


void MapModel::Initialize()
{
    int maxcluster = -1;

    for (int i = 0; i < r_worldmodel->numleafs; i++)
    {
        if (r_worldmodel->leafs[i].cluster > maxcluster)
            maxcluster = r_worldmodel->leafs[i].cluster;
    }

    renderClusters.Resize(maxcluster + 1);

    for (int i = 0; i < r_worldmodel->numleafs; i++)
    {
        mleaf_t* leaf = &r_worldmodel->leafs[i];

        // cluster 0 holds inline brush surfaces?
        if (leaf->cluster < 1 || !leaf->nummarksurfaces)
            continue;

        RenderCluster& cluster = renderClusters[leaf->cluster];

        for (int j = 0; j < leaf->nummarksurfaces; j++)
        {
            msurface_t* surface = leaf->firstmarksurface[j];

            if (!cluster.surfaces.Contains(surface))
            {
                cluster.surfaces.Push(surface);
            }
        }
    }

    // emit cluster models/nodes
    for (unsigned i = 0; i < renderClusters.Size(); i++)
    {
        byte* vis = Mod_ClusterPVS (i, r_worldmodel);
        PODVector<msurface_t*> emitted;
        int numvis = 0;

        for (unsigned j = 0; j < renderClusters.Size(); j++)
        {
            if (i == j || vis[j>>3] & (1<<(j&7)))
            {
                for (unsigned k = 0; k < renderClusters[j].surfaces.Size(); k++)
                {
                    msurface_t* surf = renderClusters[j].surfaces[k];
                    // as surfaces can be contained in multiple clusters
                    // the numvis will be slightly higher than the # emitted
                    numvis++;
                    if (!surf->emitted)
                    {
                        surf->emitted = 1;
                        emitted.Push(surf);
                    }
                }
            }

        }

        if (emitted.Size())
        {
            // if we emitted some surfaces for this cluster

            //printf("Cluster %i emitted %i new surfaces for %i visible\n", i, (int) emitted.Size(), numvis);

            for (unsigned j = 0; j < emitted.Size(); j++)
            {
                MapSurface(emitted.At(j));
            }

            Node* node = EmitBrushModel(surfaceMap);

            for (unsigned j = 0; j < emitted.Size(); j++)
            {
                if (!surfaceNodes.Contains(emitted[j]))
                {
                    surfaceNodes.Insert(MakePair(emitted[j], PODVector<Node*>()));
                }

                PODVector<Node*>& nodes = surfaceNodes.Find(emitted[j])->second_;
                nodes.Push(node);
            }

        }
    }

    for (int i = 1; i < r_worldmodel->numsubmodels;i++)
    {
        String modelname = "*";
        modelname += i;
        model_t* model = Mod_ForName((char*) modelname.CString(), qtrue);

        for (int j = 0; j < model->nummodelsurfaces; j++)
        {
            msurface_t* surf = r_worldmodel->surfaces + model->firstmodelsurface + j;
            if (surf->emitted)
            {
                ErrorExit("inline brush model already emitted");
            }

            MapSurface(surf);
        }

        Node* node = EmitBrushModel(surfaceMap, false);
        brushNodes.Insert(MakePair(model, node));

    }
}

static void CreateScene()
{
    Context* context = TBESystem::GetGlobalContext();

    ResourceCache* cache = context->GetSubsystem<ResourceCache>();

    scene_ = new Scene(context);

    // Create the Octree component to the scene. This is required before adding any drawable components, or else nothing will
    // show up. The default octree volume will be from (-1000, -1000, -1000) to (1000, 1000, 1000) in world coordinates; it
    // is also legal to place objects outside the volume but their visibility can then not be checked in a hierarchically
    // optimizing manner
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    // Set same volume as the Octree, set a close bluish fog and some ambient light
    zone->SetBoundingBox(BoundingBox(Vector3(-10000, -10000, -10000),  Vector3(10000, 10000, 10000)));
    zone->SetAmbientColor(Color(.8, .8, .8));
    zone->SetFogStart(10000);
    zone->SetFogEnd(10000);


    // Create a directional light to the world so that we can see something. The light scene node's orientation controls the
    // light direction; we will use the SetDirection() function which calculates the orientation from a forward direction vector.
    // The light will use default settings (white light, no shadows)
    //Node* lightNode = scene_->CreateChild("DirectionalLight");
    //lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f)); // The direction vector does not need to be normalized
    //Light* light = lightNode->CreateComponent<Light>();
    //light->SetLightType(LIGHT_DIRECTIONAL);

    // Create a scene node for the camera, which we will move around
    // The camera will use default settings (1000 far clip distance, 45 degrees FOV, set aspect ratio automatically)
    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(65000.0f);
    camera->SetFov(75);

    // Create a point light to the world so that we can see something.
    Node* pNode = cameraNode_->CreateChild("Light");
    pNode->SetPosition(Vector3(-5, 0, 0));
    Light* plight = pNode->CreateComponent<Light>();
    plight->SetLightType(LIGHT_POINT);
    plight->SetRange(0.0f);
    plight->SetColor(Color(1, 1, 1));
    plight->SetBrightness(1);
    plight->SetCastShadows(false);
    //plight->SetShadowIntensity(0.5f);



    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(128 * _scale,41* _scale,-320* _scale));

    //cameraNode_->SetPosition(Vector3(0, 0, -2000));

    Renderer* renderer = context->GetSubsystem<Renderer>();


    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
    // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
    // use, but now we just use full screen and default render path configured in the engine command line options
    SharedPtr<Viewport> viewport(new Viewport(context, scene_, cameraNode_->GetComponent<Camera>()));

    //SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();

    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    // Make the bloom mixing parameter more pronounced
    //effectRenderPath->SetShaderParameter("BloomMix", Vector2(1.0f, 1.5f));
    //effectRenderPath->SetShaderParameter("BloomThreshold",.2f);
    //viewport->SetRenderPath(effectRenderPath);

    renderer->SetViewport(0, viewport);

}

MapModel* MapModel::Generate()
{
    MapModel* model = new MapModel();
    model->Initialize();
    return model;
}

void R_CreateLightmap(int id, int width, int height, unsigned char* data)
{
    printf("LIGHTMAP: %i %ix%i\n", id, width, height);

    Context* context = TBESystem::GetGlobalContext();

    Texture2D* texture = new Texture2D(context);
    texture->SetNumLevels(1);
    texture->SetSize(width, height, Graphics::GetRGBAFormat());
    texture->SetData(0, 0, 0, width, height, data);
    lightmapTextures.Push(texture);

}

extern "C"
{
void	R_RenderFrame (refdef_t *fd)
{
    if (!cameraNode_)
        return;

    // optimize this, probably doing too much work
    for (unsigned i = 0; i < worldNodes.Size(); i++)
        worldNodes[i]->SetEnabled(false);

    mleaf_t* leaf = Mod_PointInLeaf (fd->vieworg, r_worldmodel);

    if (leaf->cluster < 0)
        return;

    // TODO: factor in areas?
    //if (fd->areabits)
    //{
    //
    //}

    for (HashMap<model_t*, Node *>::Iterator i = brushNodes.Begin(); i != brushNodes.End(); ++i)
    {
        i->second_->SetEnabled(false);

    }

    for (int i = 0; i < fd->num_entities; i++)
    {
        entity_t* ent = &fd->entities[i];
        model_t* model = (model_t*) ent->model;

        if (model == r_worldmodel)
            continue;

        if (model->type == mod_brush)
        {
            Node* node = brushNodes.Find(model)->second_;
            node->SetEnabled(true);

            //#define	PITCH				0		// up / down
            //#define	YAW					1		// left / right
            //#define	ROLL				2		// fall over
            Quaternion q(ent->angles[0], ent->angles[1], ent->angles[2]);
            node->SetRotation(q);
            node->SetPosition(Vector3(ent->origin[0] * _scale, ent->origin[2] * _scale, ent->origin[1] * _scale));
        }
    }

    byte* vis = Mod_ClusterPVS (leaf->cluster, r_worldmodel);

    for (unsigned i = 0; i < renderClusters.Size(); i++)
    {
        if (leaf->cluster == i || vis[i>>3] & (1<<(i&7)))
        {
            RenderCluster& cluster = renderClusters.At(i);

            for (unsigned j = 0; j < cluster.surfaces.Size(); j++)
            {
                PODVector<Node*>& nodes = surfaceNodes.Find(cluster.surfaces[j])->second_;
                for (unsigned k = 0; k < nodes.Size(); k++)
                {
                    if (!nodes[k]->IsEnabled())
                        nodes[k]->SetEnabled(true);
                }

            }
        }
    }

    Quaternion q(fd->viewangles[0], -fd->viewangles[1] + 90, fd->viewangles[2]);

    cameraNode_->SetPosition(Vector3(fd->vieworg[0] *_scale, fd->vieworg[2] *_scale, fd->vieworg[1] *_scale));
    cameraNode_->SetRotation(q);
}
}



void R_InitMapModel()
{
    CreateScene();
    MapModel::Generate();
}

