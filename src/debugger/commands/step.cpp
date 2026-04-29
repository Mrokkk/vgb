#include "debugger/interpreter/command.hpp"
#include "debugger/printer.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(step)
{
    EXECUTOR()
    {
        printInstruction(gb.cpu);
        gb.cpu.step();
        //printCpuRegs(gb.cpu);
        return 0;
    }

    HELP() = "Step one instruction";
}

}  // namespace debugger::commands
