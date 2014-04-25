

#pragma once

#include "Application.h"

using namespace Urho3D;

// base abstract class for graphics client and dedicated server apps
class TBEApp : public Application
{
    // Enable type information.
    OBJECT(TBEApp);

public:
    /// Construct.
    TBEApp(Context* context);

    /// Setup before engine initialization. Modifies the engine parameters.
    virtual void Setup() = 0;
    virtual void Start() = 0;

};
