include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(fmt
    SYSTEM
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f # tag: 12.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(raylib
    SYSTEM
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG dbc56a87da87d973a9c5baa4e7438a9d20121d28 # tag: 6.0
)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(imgui
    SYSTEM
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG b61e56346a92cfcaf1f43a545ca37b0b32239654 # tag: v1.92.8-docking
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(imgui_club
    SYSTEM
    GIT_REPOSITORY https://github.com/ocornut/imgui_club.git
    GIT_TAG a7eab6ccb9fec09f37705406a06bb3bfc09597fe # master
)
FetchContent_MakeAvailable(imgui_club)

FetchContent_Declare(rlimgui
    SYSTEM
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui
    GIT_TAG 3bc5731c4216bb8caa67fbea24aa85ce80d57ccb # tag: Raylib_6_0
)
FetchContent_MakeAvailable(rlimgui)

FetchContent_Declare(argh
    SYSTEM
    GIT_REPOSITORY https://github.com/adishavit/argh
    GIT_TAG c3f0d8c8a6dacb00df626b409248a34e3bcd15f5 # master
)
FetchContent_MakeAvailable(argh)

target_compile_definitions(raylib PUBLIC -DSUPPORT_SCREEN_CAPTURE=0)

add_library(imgui OBJECT
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui SYSTEM PUBLIC ${imgui_SOURCE_DIR})
target_include_directories(imgui SYSTEM INTERFACE ${imgui_club_SOURCE_DIR})

add_library(rlimgui OBJECT
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PRIVATE raylib imgui)
target_compile_options(rlimgui PRIVATE -Wno-error)
target_compile_definitions(rlimgui PRIVATE -DNO_FONT_AWESOME)

include(FindPkgConfig)
pkg_check_modules(BACKTRACE libbacktrace)
pkg_check_modules(ZLIB REQUIRED zlib)

add_library(zlib INTERFACE)
target_link_libraries(zlib INTERFACE ${ZLIB_LIBRARIES})
target_link_directories(zlib INTERFACE ${ZLIB_LIBRARY_DIRS})
target_include_directories(zlib SYSTEM INTERFACE ${ZLIB_INCLUDE_DIRS})

add_library(backtrace INTERFACE)
if(BACKTRACE_FOUND)
    target_compile_options(backtrace INTERFACE "-DUSE_BACKTRACE")
    target_include_directories(backtrace SYSTEM INTERFACE ${BACKTRACE_INCLUDE_DIRS})
endif()

find_package(Fontconfig)
