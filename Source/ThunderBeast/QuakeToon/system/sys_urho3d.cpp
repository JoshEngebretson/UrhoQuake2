
#include "Application.h"
#include "Console.h"
#include "DebugHud.h"
#include "Engine.h"
#include "FileSystem.h"
#include "Graphics.h"
#include "Input.h"
#include "InputEvents.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "Sprite.h"
#include "Texture2D.h"
#include "Timer.h"
#include "UI.h"
#include "XMLFile.h"
#include "CoreEvents.h"

#include "sys_urho3d.h"



extern "C" {
#include "../qcommon/qcommon.h"
void *GetGameAPI (void *import);
}

using namespace Urho3D;

Context* Q2System::sContext_ = NULL;

class QuakeToonApp : public Application
{
    // Enable type information.
    OBJECT(QuakeToonApp);

    static QuakeToonApp* sInstance_;

    Timer timer_;

public:
    /// Construct.
    QuakeToonApp(Context* context) : Application(context)
    {
        Q2System::SetContext(context);
        sInstance_ = this;
        timer_.Reset();
    }

    static unsigned GetMilliseconds()
    {
        return sInstance_->timer_.GetMSec(false);
    }


    static Engine* GetEngine()
    {
        return sInstance_->engine_;
    }

    /// Setup before engine initialization. Modifies the engine parameters.
    void Setup()
    {
        // Modify engine startup parameters
        engineParameters_["WindowTitle"] = "QuakeToon";
        engineParameters_["LogName"]     = "QuakeToon.log";
        engineParameters_["FullScreen"]  = false;
        engineParameters_["Headless"]    = false;
    }

    /// Setup after engine initialization. Creates the logo, console & debug HUD.
    void Start()
    {

        // Enable OS cursor
        GetSubsystem<Input>()->SetMouseVisible(true);

        // todo, define argc and argv as Urho3D also wants command line args
        int argc = 3;
        char *argv[] = {"quake", "+map", "demo1"};
        Qcommon_Init (argc, argv);

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


        // Finally subscribe to the update event. Note that by subscribing events at this point we have already missed some events
        // like the ScreenMode event sent by the Graphics subsystem when opening the application window. To catch those as well we
        // could subscribe in the constructor instead.
        SubscribeToEvents();
    }

    void SubscribeToEvents()
    {
        // Subscribe HandleUpdate() function for processing update events
        SubscribeToEvent(E_UPDATE, HANDLER(QuakeToonApp, HandleUpdate));
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        // Do nothing for now, could be extended to eg. animate the display
        Qcommon_Frame(10);
    }

};

QuakeToonApp* QuakeToonApp::sInstance_ = NULL;

/*
int main (int argc, char **argv)
{
    Qcommon_Init (argc, argv);

    while (1)
    {
        Qcommon_Frame (1);
    }

    return 0;
}
*/

// Expands to this example's entry-point
DEFINE_APPLICATION_MAIN(QuakeToonApp)

static Vector<String> _findfiles;
char* Sys_FindFirst (char *path, unsigned musthave, unsigned canthave)
{
    Engine* engine = QuakeToonApp::GetEngine();
    FileSystem* fileSystem = engine->GetSubsystem<FileSystem>();

    _findfiles.Clear();

    fileSystem->ScanDir(_findfiles, path, "*.*", SCAN_FILES, false);

    return Sys_FindNext(musthave, canthave);

}

char* Sys_FindNext (unsigned musthave, unsigned canthave)
{
    if (!_findfiles.Size())
        return NULL;

    static String file = _findfiles.Front();

    _findfiles.Erase(0);

    return (char*) file.CString();
}

void Sys_FindClose (void)
{
    _findfiles.Clear();
}

void	Sys_Init (void)
{
}

void Sys_Error (char *error, ...)
{
    va_list		argptr;

    printf ("Sys_Error: ");
    va_start (argptr,error);
    vprintf (error,argptr);
    va_end (argptr);
    printf ("\n");

    exit (1);
}

void Sys_Quit (void)
{
    exit (0);
}

int Sys_Milliseconds (void)
{
    return (int) QuakeToonApp::GetMilliseconds();
}

void Sys_ConsoleOutput (char *string)
{
    printf("%s", string);
}

void Sys_UnloadGame (void)
{
}

void *Sys_GetGameAPI (void *parms)
{
    return GetGameAPI(parms);
}





