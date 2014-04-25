
#pragma once

#include "Context.h"
#include "Object.h"

using namespace Urho3D;

class Q2Renderer: public Object
{
    OBJECT(Q2Renderer);

    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;

    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;


    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

    void CreateScene();

    void MoveCamera(float timeStep);

public:

    Q2Renderer(Context* context);

    void InitializeWorldModel();
};

