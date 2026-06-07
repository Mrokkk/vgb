#include "debugger/interpreter/command.hpp"
#include "debugger/printer.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(step)
{
    EXECUTOR()
    {
        gb.cpu.step();
        printInstruction(state, gb.cpu);
        return 0;
    }

    HELP() = "Step one instruction";
}

}  // namespace debugger::commands
