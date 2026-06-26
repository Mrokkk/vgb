#include "main.hpp"

#include <map>
#include <unistd.h>

#include "cpu/exception.hpp"
#include "cpu/printers.hpp"
#include "cpu/sm83.hpp"
#include "debugger/context.hpp"
#include "debugger/games/registry.hpp"
#include "debugger/imgui.hpp"
#include "debugger/printer.hpp"
#include "game_boy.hpp"
#include "interpreter/command.hpp"
#include "interpreter/interpreter.hpp"
#include "interpreter/operations.hpp"

namespace debugger
{

DEFINE_COMMAND(alias)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        if (args.empty())
        {
            interpreter::forEachAlias(
                [&ctx](const std::string& alias, const std::string& command)
                {
                    ctx.console.writeLine("{} => {}", alias, command);
                });
            return 0;
        }

        auto alias = GET_ARGUMENT(0, String);
        auto command = GET_ARGUMENT(1, String);

        interpreter::setAlias(std::string(alias), std::string(command));

        return 0;
    }

    HELP() = "Create/update/list alias(es)";
}

static void runCpu(Context& ctx)
{
    while (1)
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            break;
        }
        if (static_cast<int>(ctx.cpu.pc) != ctx.prevBreakpoint)
        {
            const auto breakpoint = ctx.breakpoints.find(ctx.cpu.pc);

            if (breakpoint != ctx.breakpoints.end())
            {
                ctx.console.writeLine("Hit breakpoint {} at {:#04x}", breakpoint->second.id, breakpoint->second.address);
                ctx.prevBreakpoint = ctx.cpu.pc;
                gb.stop();
                break;
            }
            else
            {
                ctx.prevBreakpoint = -1;
            }
        }

        if (ctx.cpu.step())
        {
            gb.stop();
            break;
        }
    }
    if (ctx.cpu.exc)
    {
        ctx.console.writeLine("Exception raised: {}", ctx.cpu.exc);
    }
    printInstruction(ctx);
}

void main(GameBoy& gb)
{
    Context ctx{
        .gb = gb,
        .cpu = gb.cpu,
        .prevBreakpoint = -1,
        .console = {
            .prompt = "(vgb)",
            .lines = {},
        }
    };

    initImGui(ctx);
    ctx.console.addLine("Debugger mode");
    ctx.console.addLine("For help, type \"help\"");

    interpreter::Operations ops;
    ops.print =
        [&ctx](std::string line)
        {
            ctx.console.addLine(std::move(line));
        };

    interpreter::initialize(std::move(ops), &ctx);
    interpreter::setAlias("b",    "break");
    interpreter::setAlias("c",    "continue");
    interpreter::setAlias("cont", "continue");
    interpreter::setAlias("regs", "registers");
    interpreter::setAlias("r",    "registers");
    interpreter::setAlias("s",    "step");

    gb.debuggerData = static_cast<void*>(&ctx);

    ctx.game = games::Registry::createGame(gb.cartridge.getTitle());

    while (1)
    {
        if (ctx.cpu.exc.type == cpu::Exception::UserInterruption)
        {
            break;
        }

        while (gb.state == GameBoy::State::Stopped)
        {
            gb.frame();

            if (ctx.cpu.exc.type == cpu::Exception::UserInterruption)
            {
                goto finish;
            }
        }

        runCpu(ctx);
    }

finish:
    deinitImGui(ctx);
    gb.debuggerData = nullptr;
}

}  // namespace debugger
