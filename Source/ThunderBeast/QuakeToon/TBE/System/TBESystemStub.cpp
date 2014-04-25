
// stubbed system functions, will eventually be removed entirely

extern "C"
{

#include "../../client/client.h"
#include "../../client/snd_loc.h"

cvar_t	*in_joystick = NULL;

qboolean SNDDMA_Init(void)
{
    return qfalse;
}

int	SNDDMA_GetDMAPos(void)
{
    return 0;
}

void SNDDMA_Shutdown(void)
{
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}


void CDAudio_Play(int track, qboolean looping)
{
}


void CDAudio_Stop(void)
{
}


void CDAudio_Resume(void)
{
}


void CDAudio_Update(void)
{
}


int CDAudio_Init(void)
{
    return 0;
}


void CDAudio_Shutdown(void)
{
}

void IN_Init (void)
{
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
}

void IN_Frame (void)
{
}

void IN_Move (usercmd_t *cmd)
{
}

void IN_Activate (qboolean active)
{
}

void IN_ActivateMouse (void)
{
}

void IN_DeactivateMouse (void)
{
}

void Sys_mkdir (char *path)
{
}


char *Sys_ConsoleInput (void)
{
    return NULL;
}

void Sys_SendKeyEvents (void)
{
}

void Sys_AppActivate (void)
{
}

void Sys_CopyProtect (void)
{
}

char *Sys_GetClipboardData( void )
{
    return NULL;
}

void	Sys_Mkdir (char *path)
{
}


}
