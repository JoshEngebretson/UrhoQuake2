
#pragma once

#include "Object.h"
#include "Str.h"
#include "Timer.h"

namespace Urho3D
{
class Engine;
class Context;
}

using namespace Urho3D;

class TBESystem : Object
{
    /// Construct.
    TBESystem(Context* context);

    /// Destruct.
    virtual ~TBESystem();

    static TBESystem* sInstance_;

    Timer timer_;

public:

    static Engine* GetEngine();
    static Context* GetContext();
    static TBESystem* GetSystem();
    static unsigned GetMilliseconds()
    {
        return sInstance_->timer_.GetMSec(false);
    }


};
