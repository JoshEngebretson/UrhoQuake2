
#include "XMLFile.h"
#include "ResourceCache.h"
#include "Console.h"
#include "UI.h"
#include "CoreEvents.h"
#include "Engine.h"
#include "DebugHud.h"
#include "TBEClientApp.h"

DEFINE_APPLICATION_MAIN(TBEClientApp)

extern "C"
{
    void Qcommon_Init (int argc, char **argv);
    void Qcommon_Frame (int msec);
}

TBEClientApp::TBEClientApp(Context* context) : TBEApp(context)
{

}

/// Setup before engine initialization. Modifies the engine parameters.
void TBEClientApp::Setup()
{
    // Modify engine startup parameters
    engineParameters_["WindowTitle"] = "QuakeToon";
    engineParameters_["LogName"]     = "QuakeToon.log";
    engineParameters_["FullScreen"]  = false;
    engineParameters_["Headless"]    = false;
    engineParameters_["WindowWidth"]    = 1280;
    engineParameters_["WindowHeight"]    = 720;
}

/// Setup after engine initialization. Creates the logo, console & debug HUD.
void TBEClientApp::Start()
{

    // Disable OS cursor
    // GetSubsystem<Input>()->SetMouseVisible(false);

    // Get default style
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(xmlFile);
    console->GetBackground()->SetOpacity(0.8f);

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);
    debugHud->Toggle(DEBUGHUD_SHOW_ALL);

    // todo, define argc and argv as Urho3D also wants command line args
    int argc = 4;
    const char *argv[] = {"quake", "+map", "demo1", "+notarget"};
    Qcommon_Init (argc, (char**) argv);

    // Finally subscribe to the update event. Note that by subscribing events at this point we have already missed some events
    // like the ScreenMode event sent by the Graphics subsystem when opening the application window. To catch those as well we
    // could subscribe in the constructor instead.
    SubscribeToEvents();
}

void TBEClientApp::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, HANDLER(TBEClientApp, HandleUpdate));
}

void TBEClientApp::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat() * 1000.0f;
    Qcommon_Frame((int) timeStep);
}

