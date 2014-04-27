
#pragma once

#include "TBEApp.h"
#include "Variant.h"

namespace Urho3D
{
    class Sprite;
}


using namespace Urho3D;

// base abstract class for graphics client and dedicated server apps
class TBEClientApp : public TBEApp
{
    // Enable type information.
    OBJECT(TBEClientApp);

     SharedPtr<Sprite> logoSprite_;

    void SubscribeToEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void CreateLogo();

public:
    /// Construct.
    TBEClientApp(Context* context);

    /// Setup before engine initialization. Modifies the engine parameters.
    void Setup();
    void Start();

};
