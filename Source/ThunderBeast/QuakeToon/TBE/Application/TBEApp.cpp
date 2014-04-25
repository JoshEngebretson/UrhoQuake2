
#include "Engine.h"
#include "TBEApp.h"
#include "TBESystem.h"

TBEApp::TBEApp(Context* context) : Application(context)
{
    context_->RegisterSubsystem(new TBESystem(context_));
}

