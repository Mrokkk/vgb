cmake_minimum_required(VERSION 3.12)

project(vgb CXX)

set(CMAKE_BUILD_TYPE Debug)

option(SANITIZE "Enable ASan and UBsan" OFF)
message(STATUS "Enable ASan and UBsan: ${SANITIZE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE ccache)

if(CCACHE)
    message(STATUS "Using ccache: ${CCACHE}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE}")
    set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always")
endif()

include(FindPkgConfig)
pkg_check_modules(BACKTRACE REQUIRED libbacktrace)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f # tag: 12.1.0
)

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG dbc56a87da87d973a9c5baa4e7438a9d20121d28 # tag: 6.0
)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG b61e56346a92cfcaf1f43a545ca37b0b32239654 # tag: v1.92.8-docking
)

FetchContent_Declare(imgui_club
    GIT_REPOSITORY https://github.com/ocornut/imgui_club.git
    GIT_TAG a7eab6ccb9fec09f37705406a06bb3bfc09597fe # master
)

FetchContent_Declare(rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui
    GIT_TAG 3bc5731c4216bb8caa67fbea24aa85ce80d57ccb # tag: Raylib_6_0
)

FetchContent_Declare(argh
    GIT_REPOSITORY https://github.com/adishavit/argh
    GIT_TAG c3f0d8c8a6dacb00df626b409248a34e3bcd15f5 # master
)

set(DOCTEST_VERSION v2.4.11)

FetchContent_Declare(doctest
    URL  https://raw.githubusercontent.com/doctest/doctest/${DOCTEST_VERSION}/doctest/doctest.h
    DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(imgui_club)
FetchContent_MakeAvailable(rlimgui)
FetchContent_MakeAvailable(doctest)
FetchContent_MakeAvailable(argh)

set(DOCTEST_INCLUDE_DIR ${doctest_SOURCE_DIR})

add_library(imgui OBJECT
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

add_library(rlimgui OBJECT
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PRIVATE raylib imgui)
target_compile_options(rlimgui PRIVATE -Wno-error)

include(FindPkgConfig)
pkg_check_modules(BACKTRACE REQUIRED libbacktrace)
pkg_check_modules(READLINE REQUIRED readline)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=c++2c -ggdb3 -Wall -Wextra -Werror -Wno-missing-field-initializers")

if(SANITIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -Wl,-lasan -fno-omit-frame-pointer")
endif()

if(OPTIMIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(LTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
endif()

include_directories(src ${BACKTRACE_INCLUDE_DIRS})

file(GLOB INSTRUCTIONS "src/cpu/isa/instructions/*.cpp")

set(COMMON_LIBRARIES

    argh
    fmt::fmt
    raylib
    ${READLINE_LIBRARIES}

)

add_library(emulator OBJECT

    src/apu.cpp
    src/cpu/isa/decoder.cpp
    src/cpu/isa/instruction_set.cpp
    src/cpu/isa/opcode.cpp
    src/cpu/isa/printers.cpp
    src/cpu/printers.cpp
    src/cpu/sm83.cpp
    src/event_system.cpp
    src/game_boy.cpp
    src/input.cpp
    src/joypad.cpp
    src/logger.cpp
    src/memory/cartridge.cpp
    src/memory/memory.cpp
    src/memory/printers.cpp
    src/ppu.cpp
    src/raylib/input.cpp
    src/raylib/renderer.cpp
    src/renderer.cpp
    src/sys/posix.cpp
    src/timer.cpp
    src/utils/buffer.cpp
    src/utils/string.cpp
    src/utils/time.cpp
    src/utils/units.cpp

    ${INSTRUCTIONS}
)

add_library(debugger OBJECT

    src/debugger/commands/break.cpp
    src/debugger/commands/continue.cpp
    src/debugger/commands/crash.cpp
    src/debugger/commands/events.cpp
    src/debugger/commands/help.cpp
    src/debugger/commands/registers.cpp
    src/debugger/commands/reset.cpp
    src/debugger/commands/set.cpp
    src/debugger/commands/step.cpp
    src/debugger/commands/watch.cpp
    src/debugger/games/pokemon_red.cpp
    src/debugger/games/registry.cpp
    src/debugger/imgui.cpp
    src/debugger/interpreter/command.cpp
    src/debugger/interpreter/commands.cpp
    src/debugger/interpreter/lexer.cpp
    src/debugger/interpreter/lexer_printers.cpp
    src/debugger/main.cpp
    src/debugger/printer.cpp

)

target_include_directories(debugger PUBLIC ${imgui_club_SOURCE_DIR})
target_link_libraries(emulator PUBLIC ${COMMON_LIBRARIES} imgui rlimgui)
target_link_libraries(debugger PUBLIC fmt::fmt imgui)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE emulator debugger imgui rlimgui)

add_executable(test
    test/instruction_tests.cpp
    test/main.cpp
)

target_link_libraries(test PUBLIC emulator debugger imgui rlimgui)

if(SANITIZE)
    set_source_files_properties(src/cpu/isa/instruction_set.cpp PROPERTIES COMPILE_OPTIONS -fno-var-tracking-assignments)
endif()

target_include_directories(test PRIVATE ${DOCTEST_INCLUDE_DIR})

add_custom_target(run
    COMMAND "${CMAKE_BINARY_DIR}/${PROJECT_NAME}"
    COMMENT "Running ${PROJECT_NAME}"
    DEPENDS ${PROJECT_NAME}
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()
