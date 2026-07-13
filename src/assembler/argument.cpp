#include "argument.hpp"

#include "assembler/glue.hpp"
#include "cpu/isa/printers.hpp"

namespace assembler
{

Argument createSpPlusExpression(Context& ctx, cpu::isa::Operand::Type operand, Expression expression)
{
    if (operand != cpu::isa::Operand::SP) [[unlikely]]
    {
        reportError(ctx, "incorrect unary operation on non-const operand \"{}\"", operand);
        return {};
    }

    return assembler::Argument(operand, expression);
}

}  // namespace assembler
