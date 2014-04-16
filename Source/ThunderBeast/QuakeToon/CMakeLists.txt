
add_definitions(-DC_ONLY)

set ( CLIENT_SOURCE_FILES client/cl_cin.c
                          client/cl_inv.c
                          client/cl_pred.c
                          client/console.c
                          client/snd_dma.c
                          client/cl_ents.c
                          client/cl_main.c
                          client/cl_scrn.c
                          client/keys.c
                          client/snd_mem.c
                          client/cl_fx.c
                          client/cl_newfx.c
                          client/cl_tent.c
                          client/menu.c
                          client/snd_mix.c
                          client/cl_input.c
                          client/cl_parse.c
                          client/cl_view.c
                          client/qmenu.c
                          client/x86.c
                          client/adivtab.h
                          client/block8.h
                          client/input.h
                          client/screen.h
                          client/anorms.h
                          client/cdaudio.h
                          client/keys.h
                          client/snd_loc.h
                          client/asm_i386.h
                          client/client.h
                          client/qmenu.h
                          client/sound.h
                          client/block16.h
                          client/console.h
                          client/ref.h
                          client/vid.h )

set ( SERVER_SOURCE_FILES server/server.h
                          server/sv_ccmds.c
                          server/sv_ents.c
                          server/sv_game.c
                          server/sv_init.c
                          server/sv_main.c
                          server/sv_send.c
                          server/sv_user.c
                          server/sv_world.c )

set ( COMMON_SOURCE_FILES qcommon/cmd.c
                          qcommon/cmodel.c
                          qcommon/common.c
                          qcommon/crc.c
                          qcommon/crc.h
                          qcommon/cvar.c
                          qcommon/files.c
                          qcommon/md4.c
                          qcommon/net_chan.c
                          qcommon/pmove.c
                          qcommon/qcommon.h
                          qcommon/qfiles.h )

set ( SHARED_SOURCE_FILES game/q_shared.c
                          game/q_shared.h )

# will be replaces
set (PLATFORM_SOURCE_FILES platform/net_udp.c )

set (NULL_SOURCE_FILES null/cd_null.c
                       null/glimp_null.c
                       null/snddma_null.c
                       null/sys_null.c
                       null/in_null.c
                       null/vid_null.c )

set (SOURCE_FILES ${COMMON_SOURCE_FILES} ${SHARED_SOURCE_FILES} ${PLATFORM_SOURCE_FILES} ${CLIENT_SOURCE_FILES} ${SERVER_SOURCE_FILES} ${NULL_SOURCE_FILES})

# Define target name
set (TARGET_NAME QuakeToon)

# Define source files
#define_source_files (EXTRA_H_FILES ${COMMON_SAMPLE_H_FILES})

# Setup target with resource copying
setup_main_executable ()
