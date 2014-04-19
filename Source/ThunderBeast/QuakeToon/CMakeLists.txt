
# Set project name
project (QuakeToon)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

add_definitions(-DC_ONLY -DGAME_HARD_LINKED)

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

set ( GAME_SOURCE_FILES game/g_ai.c
                        game/g_save.c
                        game/m_boss2.c
                        game/m_gladiator.c
                        game/m_supertank.c
                          game/g_chase.c
                          game/g_spawn.c
                          game/m_boss3.c
                          game/m_gunner.c
                          game/m_tank.c
                          game/g_cmds.c
                          game/g_svcmds.c
                          game/m_boss31.c
                          game/m_hover.c
                          game/p_client.c
                          game/g_combat.c
                          game/g_target.c
                          game/m_boss32.c
                          game/m_infantry.c
                          game/p_hud.c
                          game/g_func.c
                          game/g_trigger.c
                          game/m_brain.c
                          game/m_insane.c
                          game/p_trail.c
                          game/g_items.c
                          game/g_turret.c
                          game/m_chick.c
                          game/m_medic.c
                          game/p_view.c
                          game/g_main.c
                          game/g_utils.c
                          game/m_flash.c
                          game/m_move.c
                          game/p_weapon.c
                          game/g_misc.c
                          game/g_weapon.c
                          game/m_flipper.c
                          game/m_mutant.c
                          game/q_shared.c
                          game/g_monster.c
                          game/m_actor.c
                          game/m_float.c
                          game/m_parasite.c
                          game/g_phys.c
                          game/m_berserk.c
                          game/m_flyer.c
                          game/m_soldier.c
                          game/g_local.h
                          game/m_boss32.h
                          game/m_gladiator.h
                          game/m_mutant.h
                          game/m_tank.h
                          game/game.h
                          game/m_brain.h
                          game/m_gunner.h
                          game/m_parasite.h
                          game/q_shared.h
                          game/m_actor.h
                          game/m_chick.h
                          game/m_hover.h
                          game/m_player.h
                          game/m_berserk.h
                          game/m_flipper.h
                          game/m_infantry.h
                          game/m_rider.h
                          game/m_boss2.h
                          game/m_float.h
                          game/m_insane.h
                          game/m_soldier.h
                          game/m_boss31.h
                          game/m_flyer.h
                          game/m_medic.h
                          game/m_supertank.h )

# will be replaces
set (PLATFORM_SOURCE_FILES platform/net_udp.c )

set (NULL_SOURCE_FILES null/cd_null.c
                       null/glimp_null.c
                       null/snddma_null.c
                       null/sys_null.c
                       null/in_null.c
                       null/vid_null.c )

set (SYSTEM_SOURCE_FILES system/sys_urho3d.cpp system/sys_urho3d.h system/ref_urho3d.cpp system/ref_urho3d.h)

set (SOURCE_FILES ${COMMON_SOURCE_FILES} ${GAME_SOURCE_FILES} ${PLATFORM_SOURCE_FILES} ${SYSTEM_SOURCE_FILES} ${CLIENT_SOURCE_FILES} ${SERVER_SOURCE_FILES} ${NULL_SOURCE_FILES})

# Define target name
set (TARGET_NAME QuakeToon)

# Define source files
#define_source_files (EXTRA_H_FILES ${COMMON_SAMPLE_H_FILES})

# Setup target with resource copying
setup_main_executable ()
