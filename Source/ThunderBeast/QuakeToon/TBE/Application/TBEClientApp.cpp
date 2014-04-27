
#include "XMLFile.h"
#include "ResourceCache.h"
#include "Console.h"
#include "UI.h"
#include "CoreEvents.h"
#include "Engine.h"
#include "DebugHud.h"
#include "Input.h"
#include "Texture2D.h"
#include "UI.h"
#include "Sprite.h"
#include "TBEClientApp.h"

extern "C"
{
 #include "../../client/client.h"
}

DEFINE_APPLICATION_MAIN(TBEClientApp)

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

    CreateLogo();

    // todo, define argc and argv as Urho3D also wants command line args
    //int argc = 5;
    //const char *argv[] = {"quake", "+map", "demo1", "+notarget", "+god"};

    int argc = 3;
    const char *argv[] = {"quake", "+demomap", "q2demo1.dm2"};

    Qcommon_Init (argc, (char**) argv);

    // Finally subscribe to the update event. Note that by subscribing events at this point we have already missed some events
    // like the ScreenMode event sent by the Graphics subsystem when opening the application window. To catch those as well we
    // could subscribe in the constructor instead.
    SubscribeToEvents();
}

void TBEClientApp::CreateLogo()
{
    // Get logo texture
   ResourceCache* cache = GetSubsystem<ResourceCache>();
   Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/LogoLarge.png");
   if (!logoTexture)
       return;

   // Create logo sprite and add to the UI layout
   UI* ui = GetSubsystem<UI>();
   logoSprite_ = ui->GetRoot()->CreateChild<Sprite>();

   // Set logo sprite texture
   logoSprite_->SetTexture(logoTexture);

   int textureWidth = logoTexture->GetWidth();
   int textureHeight = logoTexture->GetHeight();

   // Set logo sprite scale
   logoSprite_->SetScale(256.0f / textureWidth);

   // Set logo sprite size
   logoSprite_->SetSize(textureWidth, textureHeight);

   // Set logo sprite hot spot
   logoSprite_->SetHotSpot(0, textureHeight);

   // Set logo sprite alignment
   logoSprite_->SetAlignment(HA_LEFT, VA_BOTTOM);

   // Make logo not fully opaque to show the scene underneath
   logoSprite_->SetOpacity(0.75f);

   // Set a low priority for the logo so that other UI elements can be drawn on top
   logoSprite_->SetPriority(-100);
}

void TBEClientApp::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, HANDLER(TBEClientApp, HandleUpdate));
}

void TBEClientApp::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat() * 1000.0f;

    Input* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.2f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();

    cl.viewangles[YAW] -= MOUSE_SENSITIVITY * mouseMove.x_;
    cl.viewangles[PITCH] += MOUSE_SENSITIVITY * mouseMove.y_;

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    //cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    // Use the Translate() function (default local space) to move relative to the node's orientation.
    /*
       if (input->GetKeyDown('W'))
           cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
       if (input->GetKeyDown('S'))
           cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
       if (input->GetKeyDown('A'))
           cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
       if (input->GetKeyDown('D'))
           cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
       */

    static bool wdown = false;
    static bool sdown = false;
    static bool adown = false;
    static bool ddown = false;
    static bool cdown = false;
    static bool fdown = false;
    static bool spacedown = false;

    if (input->GetKeyDown('W') && !wdown)
    {
        wdown = true;

        Key_Event(K_UPARROW, qtrue, 0);

    }
    else if (wdown)
    {
        wdown = false;

        Key_Event(K_UPARROW, qfalse, 0);
    }

    if (input->GetKeyDown('S') && !sdown)
    {
        sdown = true;

        Key_Event(K_DOWNARROW, qtrue, 0);

    }
    else if (sdown)
    {
        sdown = false;

        Key_Event(K_DOWNARROW, qfalse, 1);
    }

    if (input->GetKeyDown('A') && !adown)
    {
        adown = true;

        Key_Event(K_LEFTARROW, qtrue, 0);

    }
    else if (adown)
    {
        adown = false;

        Key_Event(K_LEFTARROW, qfalse, 0);
    }

    if (input->GetKeyDown('D') && !ddown)
    {
        ddown = true;

        Key_Event(K_RIGHTARROW, qtrue, 0);

    }
    else if (ddown)
    {
        ddown = false;

        Key_Event(K_RIGHTARROW, qfalse, 0);
    }

    if (input->GetKeyDown(' ') && !spacedown)
    {
        spacedown = true;

        Key_Event(K_SPACE, qtrue, 0);

    }
    else if (spacedown)
    {
        spacedown = false;

        Key_Event(K_SPACE, qfalse, 0);
    }

    if (input->GetKeyDown('C') && !cdown)
    {
        cdown = true;

        Key_Event('c', qtrue, 0);

    }
    else if (cdown)
    {
        cdown = false;

        Key_Event('c', qfalse, 0);
    }

    if (input->GetKeyDown('F') && !fdown)
    {
        fdown = true;

        Key_Event(K_MOUSE1, qtrue, 0);

    }
    else if (fdown)
    {
        fdown = false;

        Key_Event(K_MOUSE1, qfalse, 0);
    }

    Qcommon_Frame((int) timeStep);
}

