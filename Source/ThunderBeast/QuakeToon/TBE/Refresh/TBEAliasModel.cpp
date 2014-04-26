
#include "Str.h"
#include "Material.h"
#include "ProcessUtils.h"
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

    printf("%s\n", pFRAME->name);

    SharedPtr<IndexBuffer> ib;
    SharedPtr<VertexBuffer> vb;

    // TODO: share vertex buffers
    vb = new VertexBuffer(context);
    ib = new IndexBuffer(context);

    // going to need normal
    unsigned elementMask = MASK_POSITION  | MASK_NORMAL| MASK_TEXCOORD1;

    vb->SetSize(palias->num_xyz, elementMask);
    ib->SetSize(palias->num_tris * 3, false);

    int vcount = 0;
    float* vertexData = (float *) vb->Lock(0, palias->num_xyz);
    unsigned short* indexData = (unsigned short*) ib->Lock(0, palias->num_tris * 3);

    Vector3 center = Vector3::ZERO;
    BoundingBox bbox;

    for (int i = 0; i < palias->num_xyz; i++)
    {
        float x = float(pFRAME->verts[i].v[0]) * pFRAME->scale[0];
        float y = float(pFRAME->verts[i].v[2]) * pFRAME->scale[2];
        float z = float(pFRAME->verts[i].v[1]) * pFRAME->scale[1];

        x += pFRAME->translate[0];
        y += pFRAME->translate[2];
        z += pFRAME->translate[1];

        x *= _scale;
        y *= _scale;
        z *= _scale;

        float nx = r_avertexnormals[pFRAME->verts[i].lightnormalindex][0];
        float ny = r_avertexnormals[pFRAME->verts[i].lightnormalindex][2];
        float nz = r_avertexnormals[pFRAME->verts[i].lightnormalindex][1];

        float s = float(pST[i].s) / float(palias->skinwidth);
        float t = float(pST[i].t) / float(palias->skinheight);

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


    for (int i = 0; i < palias->num_tris; i++)
    {
        dtriangle_t* tri = pTRI + i;

        *indexData++ = tri->index_xyz[0];
        *indexData++ = tri->index_xyz[1];
        *indexData++ = tri->index_xyz[2];
    }

    vb->Unlock();
    ib->Unlock();

    center /= palias->num_xyz;

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
        //printf("%s\n", (char *) (pheader + palias->ofs_skins + i*MAX_SKINNAME));
    }

    modelMap.Insert(MakePair(model, nmodel));

    return nmodel;

}
