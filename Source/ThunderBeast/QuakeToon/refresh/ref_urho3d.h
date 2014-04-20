
#pragma once

#include "Context.h"
#include "Object.h"

using namespace Urho3D;

class Q2Renderer: public Object
{
    OBJECT(Q2Renderer);

    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

    void CreateScene();

public:

    Q2Renderer(Context* context);

    void InitializeWorldModel();
};

