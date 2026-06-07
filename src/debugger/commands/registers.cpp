#include "debugger/interpreter/command.hpp"
#include "debugger/printer.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(registers)
{
    EXECUTOR()
    {
        printCpuRegs(state, gb.cpu);
        return 0;
    }

    HELP() = "Print CPU registers";
}

}  // namespace debugger::commands
