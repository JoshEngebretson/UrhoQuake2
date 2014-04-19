
#pragma once

namespace Urho3D
{
    class Context;
}

using namespace Urho3D;

class Q2System
{

    static Context* sContext_;

public:

    static Context* GetContext() { return sContext_; }
    static void SetContext(Context* ctx) { sContext_ = ctx; }

};

