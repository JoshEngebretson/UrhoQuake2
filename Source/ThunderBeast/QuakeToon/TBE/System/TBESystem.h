
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

class TBESystem : public Object
{
    OBJECT(TBESystem);

    static TBESystem* sInstance_;
    Timer timer_;

public:

    /// Construct.
    TBESystem(Context* context);

    /// Destruct.
    virtual ~TBESystem();


    static Engine* GetEngine();
    static Context* GetGlobalContext();
    static TBESystem* GetSystem();
    static unsigned GetMilliseconds();
};
