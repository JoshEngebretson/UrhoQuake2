
#include "ProcessUtils.h"
#include "Context.h"
#include "Engine.h"
#include "FileSystem.h"
#include "TBESystem.h"

using namespace Urho3D;

TBESystem* TBESystem::sInstance_ = NULL;

/// Construct.
TBESystem::TBESystem(Context* context) : Object(context)
{
    if (sInstance_)
    {
        ErrorExit("TBESystem already initialized");
    }

    sInstance_ = this;

    timer_.Reset();

}

/// Destruct.
TBESystem::~TBESystem()
{

}

Context* TBESystem::GetGlobalContext()
{
    if (!sInstance_)
    {
        ErrorExit("TBESystem not initialized");
    }

    return sInstance_->GetContext();

}

TBESystem* TBESystem::GetSystem()
{
    if (!sInstance_)
    {
        ErrorExit("TBESystem not initialized");
    }

    return sInstance_;

}

Engine* TBESystem::GetEngine()
{
    if (!sInstance_)
    {
        ErrorExit("TBESystem not initialized");
    }

    return sInstance_->GetContext()->GetSubsystem<Engine>();

}

unsigned TBESystem::GetMilliseconds()

{
    if (!sInstance_)
    {
        ErrorExit("TBESystem not initialized");
    }

    return sInstance_->timer_.GetMSec(false);
}


extern "C"
{

#include "../../qcommon/qcommon.h"

int	curtime;
unsigned sys_frame_time;

static Vector<String> _findfiles;
char* Sys_FindFirst (char *path, unsigned musthave, unsigned canthave)
{
    Engine* engine = TBESystem::GetEngine();
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
    return (int) TBESystem::GetMilliseconds();
}

void Sys_ConsoleOutput (char *string)
{
    printf("%s", string);
}

void Sys_UnloadGame (void)
{
}

void *GetGameAPI (void *import);
void *Sys_GetGameAPI (void *parms)
{
    return GetGameAPI(parms);
}

}
