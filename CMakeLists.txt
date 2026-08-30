cmake_minimum_required(VERSION 3.30)

project(vgb CXX)

option(BUILD_TESTS "Build tests"                   ON)
option(LTO         "Enable Link Time Optimization" OFF)
option(OPTIMIZE    "Enable compiler optimizations" ON)
option(SANITIZE    "Enable ASan and UBsan"         OFF)
option(PROFILE     "Enable profiling"              OFF)

message(STATUS "Build tests:                   ${BUILD_TESTS}")
message(STATUS "Enable Link Time Optimization: ${LTO}")
message(STATUS "Enable compiler optimizations: ${OPTIMIZE}")
message(STATUS "Enable ASan and UBsan:         ${SANITIZE}")
message(STATUS "Enable profiling:              ${PROFILE}")

set(CMAKE_BUILD_TYPE Debug)

include(cmake/compile_commands.cmake)
include(cmake/ccache.cmake)
include(cmake/fetch_dependencies.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=c++2c -ggdb3 -Wall -Wextra -Werror -Wno-missing-field-initializers")

if(PROFILE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
endif()

if(SANITIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -Wl,-lasan -fno-omit-frame-pointer")
endif()

if(OPTIMIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()

if(LTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
endif()

include_directories(SYSTEM src/ext)

file(GLOB INSTRUCTIONS "src/cpu/isa/instructions/*.cpp")

add_subdirectory(src/assembler)

# https://gitlab.kitware.com/cmake/cmake/-/work_items/18090
set(LIBRARIES
    fmt
    raylib
    imgui
    rlimgui
    zlib
    backtrace
    Fontconfig::Fontconfig
    assembler
)

add_library(emulator OBJECT

    src/apu.cpp
    src/core/ini_serializer.cpp
    src/core/logger.cpp
    src/core/thread.cpp
    src/cpu/isa/decoder.cpp
    src/cpu/isa/instruction_set.cpp
    src/cpu/isa/opcode.cpp
    src/cpu/isa/printers.cpp
    src/cpu/printers.cpp
    src/cpu/sm83.cpp
    src/debugger/commands/break.cpp
    src/debugger/commands/continue.cpp
    src/debugger/commands/crash.cpp
    src/debugger/commands/delete.cpp
    src/debugger/commands/events.cpp
    src/debugger/commands/help.cpp
    src/debugger/commands/registers.cpp
    src/debugger/commands/reset.cpp
    src/debugger/commands/step.cpp
    src/debugger/commands/symbol.cpp
    src/debugger/commands/watch.cpp
    src/debugger/console.cpp
    src/debugger/games/pokemon_red.cpp
    src/debugger/games/registry.cpp
    src/debugger/gui/callstack_window.cpp
    src/debugger/gui/cartridge_window.cpp
    src/debugger/gui/config_window.cpp
    src/debugger/gui/console_window.cpp
    src/debugger/gui/cpu_window.cpp
    src/debugger/gui/disassembly_window.cpp
    src/debugger/gui/emulation_window.cpp
    src/debugger/gui/game_window.cpp
    src/debugger/gui/io_window.cpp
    src/debugger/gui/lcd_window.cpp
    src/debugger/gui/log_window.cpp
    src/debugger/gui/main.cpp
    src/debugger/gui/map_window.cpp
    src/debugger/gui/memory_window.cpp
    src/debugger/gui/messages.cpp
    src/debugger/gui/system_stats_window.cpp
    src/debugger/main.cpp
    src/debugger/printer.cpp
    src/debugger/symbols_map.cpp
    src/error.cpp
    src/event_system.cpp
    src/ext/imgui_ext.cpp
    src/game_boy.cpp
    src/interpreter/command.cpp
    src/interpreter/interpreter.cpp
    src/interpreter/lexer.cpp
    src/interpreter/lexer_printers.cpp
    src/joypad.cpp
    src/memory/cartridge.cpp
    src/memory/memory.cpp
    src/memory/printers.cpp
    src/ppu.cpp
    src/save_manager.cpp
    src/save_serializer.cpp
    src/sys/mapped_file.cpp
    src/sys/path.cpp
    src/sys/platform.cpp
    src/sys/posix.cpp
    src/sys/raylib/input.cpp
    src/sys/raylib/renderer.cpp
    src/sys/supervision.cpp
    src/sys/system.cpp
    src/timer.cpp
    src/utils/string.cpp
    src/utils/time.cpp
    src/utils/units.cpp

    ${INSTRUCTIONS}
)
target_include_directories(emulator PUBLIC src)
target_link_libraries(emulator PUBLIC ${LIBRARIES})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE emulator ${LIBRARIES} argh)

if(SANITIZE)
    set_source_files_properties(src/cpu/isa/instruction_set.cpp PROPERTIES COMPILE_OPTIONS -fno-var-tracking-assignments)
endif()

add_custom_target(run
    COMMAND "${CMAKE_BINARY_DIR}/${PROJECT_NAME}"
    COMMENT "Running ${PROJECT_NAME}"
    DEPENDS ${PROJECT_NAME}
)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()
