
#include "TBEImage.h"
#include "Engine.h"
#include "FileSystem.h"
#include "ResourceCache.h"
#include "TBESystem.h"

// this should not be being used, as we will default to this texture
// keep r_notexture for now as it is referenced externally

image_t *r_notexture = NULL;
static HashMap<String, image_t*> textureLookup;

void GL_InitImages ()
{
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

        ResourceCache* cache = TBESystem::GetEngine()->GetSubsystem<ResourceCache>();

        Texture2D* texture = cache->GetResource<Texture2D>(textureName);

        if (!texture)
        {
            texture = cache->GetResource<Texture2D>("Textures/UrhoIcon.png");
        }

        image_t* image = new image_t;
        image->texture = texture;
        image->width = texture->GetWidth();
        image->height = texture->GetHeight();

        textureLookup.Insert(MakePair(String(name), image));
    }

    return textureLookup.Find(name)->second_;

}

void GL_FreeUnusedImages()
{

}
