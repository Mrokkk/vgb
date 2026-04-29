cmake_minimum_required(VERSION 3.12)

project(vgb CXX)

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
    GIT_TAG master
)

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG master
)

FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(raylib)

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

add_executable(${PROJECT_NAME}

    src/event.cpp
    src/event_system.cpp
    src/main.cpp

    src/cpu/isa/decoder.cpp
    src/cpu/isa/instruction_set.cpp
    src/cpu/isa/opcode.cpp
    src/cpu/isa/printers.cpp
    src/cpu/printers.cpp
    src/cpu/sm83.cpp
    src/cpu/timer.cpp
    src/debugger/commands/break.cpp
    src/debugger/commands/continue.cpp
    src/debugger/commands/crash.cpp
    src/debugger/commands/help.cpp
    src/debugger/commands/registers.cpp
    src/debugger/commands/set.cpp
    src/debugger/commands/step.cpp
    src/debugger/commands/watch.cpp
    src/debugger/interpreter/command.cpp
    src/debugger/interpreter/commands.cpp
    src/debugger/interpreter/lexer.cpp
    src/debugger/interpreter/lexer_printers.cpp
    src/debugger/main.cpp
    src/debugger/printer.cpp
    src/memory/cartridge.cpp
    src/memory/memory.cpp
    src/ppu/video.cpp
    src/sys/posix.cpp
    src/utils/buffer.cpp
    src/utils/string.cpp
    src/utils/time.cpp
    src/utils/units.cpp

    ${INSTRUCTIONS}
)

target_link_libraries(${PROJECT_NAME} fmt::fmt raylib ${READLINE_LIBRARIES})

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
