
#pragma once

#include "TBEModelLoad.h"

namespace Urho3D
{
    class Model;
}

using namespace Urho3D;

class MapModel
{

    void Initialize();

public:

    static MapModel* Generate();

};
