
#include "Str.h"
#include "Material.h"
#include "Context.h"
#include "ResourceCache.h"
#include "ProcessUtils.h"
#include "Technique.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Geometry.h"
#include "Model.h"
#include "BoundingBox.h"
#include "TBEAliasModel.h"
#include "TBESystem.h"

static float r_avertexnormals[NUMVERTEXNORMALS][3] = {
   #include "TBEAliasNormals.h"
};


// should match the one in TBEMapModel.cpp
static float _scale = .1f;
static HashMap<model_t*, Model* > modelMap;

static HashMap<String, SharedPtr<Material> > materialLookup;

static Material* GetAliasMaterial(model_t* model, int skinnum)
{
    byte* pheader = (byte*) model->extradata;
    dmdl_t* palias = (dmdl_t*) model->extradata;

    String textureFile = (const char *) (pheader + palias->ofs_skins + skinnum*MAX_SKINNAME);

    HashMap<String, SharedPtr<Material> >::Iterator itr = materialLookup.Find(textureFile);
    if (itr != materialLookup.End())
        return itr->second_;

    Context* context = TBESystem::GetGlobalContext();

    SharedPtr<Material> material = SharedPtr<Material>(new Material(context));
    ResourceCache* cache = context->GetSubsystem<ResourceCache>();
    Technique* technique = cache->GetResource<Technique>("Techniques/Diff.xml");

    material->SetNumTechniques(1);
    material->SetTechnique(0, technique);
    material->SetName(textureFile);
    material->SetTexture(TU_DIFFUSE, model->skins[skinnum]->texture);

    materialLookup.Insert(MakePair(textureFile, material));

    return material;

}

struct AliasVertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

static PODVector<AliasVertex> aliasVertices;

static int EmitAliasVertex(AliasVertex& v)
{
    for (unsigned i = 0; i < aliasVertices.Size(); i++)
    {
        const AliasVertex& v2 = aliasVertices[i];

        if (fabs(v.x - v2.y ) > .01f)
            continue;
        if (fabs(v.y - v2.y ) > .01f)
            continue;
        if (fabs(v.z - v2.z ) > .01f)
            continue;

        if (fabs(v.u - v2.u ) > .01f)
            continue;

        if (fabs(v.v - v2.v ) > .01f)
            continue;

        return i;
    }

    aliasVertices.Push(v);
    return (int) aliasVertices.Size() - 1;

}

Model* GetAliasModel(model_t* model)
{
    Context* context = TBESystem::GetGlobalContext();

    if (model->type != mod_alias)
    {
        ErrorExit("GetAliasModel for non-alias model");
    }

    HashMap<model_t*, Model* >::Iterator itr = modelMap.Find(model);
    if (itr != modelMap.End())
        return itr->second_;

    // we have to instantiate a new model

    byte* pheader = (byte*) model->extradata;
    dmdl_t* palias = (dmdl_t*) model->extradata;

    dstvert_t* pST = (dstvert_t *)(pheader + palias->ofs_st);
    dtriangle_t* pTRI = (dtriangle_t *)(pheader + palias->ofs_tris);

    int frame = 0;
    daliasframe_t *pFRAME = (daliasframe_t *)(pheader + palias->ofs_frames + frame * palias->framesize);

    SharedPtr<IndexBuffer> ib;
    SharedPtr<VertexBuffer> vb;

    // TODO: share vertex buffers
    vb = new VertexBuffer(context);
    ib = new IndexBuffer(context);

    // going to need normal
    unsigned elementMask = MASK_POSITION  | MASK_NORMAL| MASK_TEXCOORD1;

    int numVertices = palias->num_tris * 3;
    vb->SetSize(numVertices, elementMask);
    ib->SetSize(palias->num_tris * 3, false);

    int vcount = 0;

    float* vertexData = (float *) vb->Lock(0, numVertices);
    unsigned short* indexData = (unsigned short*) ib->Lock(0, palias->num_tris * 3);

    Vector3 center = Vector3::ZERO;
    BoundingBox bbox;

    for (int i = 0; i < palias->num_tris; i++)
    {
        dtriangle_t* tri = pTRI + i;

        *indexData++ = i * 3;
        *indexData++ = i * 3 + 1;
        *indexData++ = i * 3 + 2;

        for (int j = 0; j < 3; j++)
        {
            int vidx = tri->index_xyz[j];

            float x = float(pFRAME->verts[vidx].v[0]) * pFRAME->scale[0];
            float y = float(pFRAME->verts[vidx].v[2]) * pFRAME->scale[2];
            float z = float(pFRAME->verts[vidx].v[1]) * pFRAME->scale[1];

            x += pFRAME->translate[0];
            y += pFRAME->translate[2];
            z += pFRAME->translate[1];

            x *= _scale;
            y *= _scale;
            z *= _scale;

            float nx = r_avertexnormals[pFRAME->verts[vidx].lightnormalindex][0];
            float ny = r_avertexnormals[pFRAME->verts[vidx].lightnormalindex][2];
            float nz = r_avertexnormals[pFRAME->verts[vidx].lightnormalindex][1];

            float s = float(pST[tri->index_st[j]].s) / float(palias->skinwidth);
            float t = float(pST[tri->index_st[j]].t) / float(palias->skinheight);

            *vertexData++ = x;
            *vertexData++ = y;
            *vertexData++ = z;

            *vertexData++ = nx;
            *vertexData++ = ny;
            *vertexData++ = nz;

            *vertexData++ = s;
            *vertexData++ = t;

            center += Vector3(x, y, z);
            bbox.Merge(Vector3(x, y, z));

        }

    }


    vb->Unlock();
    ib->Unlock();

    center /= numVertices;

    Geometry* geom = new Geometry(context);

    geom->SetIndexBuffer(ib);
    geom->SetVertexBuffer(0, vb, elementMask);
    geom->SetDrawRange(TRIANGLE_LIST, 0, palias->num_tris * 3, false);


    Model* nmodel = new Model(context);

    nmodel->SetNumGeometries(1);

    nmodel->SetNumGeometryLodLevels(0, 1);
    nmodel->SetGeometry(0, 0, geom);
    nmodel->SetGeometryCenter(0, center);
    nmodel->SetBoundingBox(bbox);


    for (int i=0 ; i<palias->num_skins ; i++)
    {
        model->materials[i] = GetAliasMaterial(model, i);
    }

    modelMap.Insert(MakePair(model, nmodel));

    return nmodel;

}
