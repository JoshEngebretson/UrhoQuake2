
# Set project name
project (QuakeToon)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

add_definitions(-DC_ONLY -DGAME_HARD_LINKED)

file (GLOB CLIENT_FILES client/*.c client/*.h )

file (GLOB SERVER_FILES server/*.c server/*.h )

file (GLOB COMMON_FILES qcommon/*.c qcommon/*.h )

file (GLOB GAME_FILES game/*.c game/*.h )


# will be replaces
#set (PLATFORM_SOURCE_FILES platform/net_udp.c )

#set (NULL_SOURCE_FILES null/cd_null.c
#                       null/glimp_null.c
#                       null/snddma_null.c
#                       null/sys_null.c
#                       null/in_null.c
#                       null/vid_null.c )

#set (REFRESH_SOURCE_FILES refresh/ref_urho3d.cpp refresh/ref_urho3d.h refresh/ref_model.c refresh/ref_model.h
#                          refresh/ref_gl.c refresh/ref_gl.h refresh/ref_image.c refresh/ref_image.h refresh/ref_local.h refresh/vid.c)

#set (SYSTEM_SOURCE_FILES system/sys_urho3d.cpp system/sys_urho3d.h system/sys_hunk.c)

set (SOURCE_FILES ${COMMON_FILES} ${CLIENT_FILES} ${SERVER_FILES} ${GAME_FILES})

# Define target name
set (TARGET_NAME QuakeToon)

# Define source files
#define_source_files (EXTRA_H_FILES ${COMMON_SAMPLE_H_FILES})

# Setup target with resource copying
setup_main_executable ()
