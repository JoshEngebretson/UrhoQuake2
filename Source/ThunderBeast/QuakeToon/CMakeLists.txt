
# Set project name
project (QuakeToon)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS} TBE/Application TBE/System)

add_definitions(-DC_ONLY -DGAME_HARD_LINKED)

file (GLOB CLIENT_FILES client/*.c client/*.h )

file (GLOB SERVER_FILES server/*.c server/*.h )

file (GLOB COMMON_FILES qcommon/*.c qcommon/*.h )

file (GLOB GAME_FILES game/*.c game/*.h )

if (MSVC)

set (PLATFORM_FILES platform/net_win_udp.c platform/sys_win_hunk.c )

else()

set (PLATFORM_FILES platform/net_udp.c platform/sys_hunk.c )

endif()


file (GLOB APPLICATION_FILES TBE/Application/*.cpp TBE/Application/*.h )

file (GLOB SYSTEM_FILES TBE/System/*.cpp TBE/System/*.h )

file (GLOB REFRESH_FILES TBE/Refresh/*.cpp TBE/Refresh/*.h )

set (SOURCE_FILES ${COMMON_FILES} ${CLIENT_FILES} ${SERVER_FILES} ${GAME_FILES} ${PLATFORM_FILES} ${APPLICATION_FILES} ${SYSTEM_FILES} ${REFRESH_FILES})

# Define target name
set (TARGET_NAME QuakeToon)

# Define source files
#define_source_files (EXTRA_H_FILES ${COMMON_SAMPLE_H_FILES})

# Setup target with resource copying
setup_main_executable ()
