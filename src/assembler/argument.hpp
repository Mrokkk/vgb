#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>

#include "assembler/assembler_fwd.hpp"
#include "assembler/expression.hpp"
#include "cpu/isa/operand.hpp"
#include "utils/memory.hpp"

namespace assembler
{

struct Argument
{
    using Action = cpu::isa::Operand::Action;
    using OperandType = cpu::isa::Operand::Type;

    enum Type : uint8_t
    {
        None,
        Label,
        Expression,
        Operand,
        SpPlusExpression,
    };

    constexpr Argument()
        : mType(None)
        , mIndirect(false)
        , mAction(Action::None)
    {
    }

    constexpr Argument(const std::string_view& label, bool indirect = false)
        : mType(Label)
        , mIndirect(indirect)
        , mAction(Action::None)
        , mLabel(label)
    {
    }

    constexpr Argument(struct Expression expression, bool indirect = false)
        : mType(Expression)
        , mIndirect(indirect)
        , mAction(Action::None)
        , mExpression(std::move(expression))
    {
    }

    constexpr Argument(OperandType operand, bool indirect = false, Action action = Action::None)
        : mType(Operand)
        , mIndirect(indirect)
        , mAction(action)
        , mOperand(operand)
    {
    }

    constexpr Argument(cpu::isa::Operand::Type, struct Expression expression)
        : mType(SpPlusExpression)
        , mIndirect(false)
        , mAction(Action::None)
        , mExpression(std::move(expression))
    {
    }

    constexpr Argument(const Argument& other)
        : mType(other.mType)
        , mIndirect(other.mIndirect)
        , mAction(other.mAction)
    {
        switch (mType)
        {
            case None:
                break;
            case Label:
                utils::constructAt(&mLabel, other.mLabel);
                break;
            case Expression:
            case SpPlusExpression:
                utils::constructAt(&mExpression, other.mExpression);
                break;
            case Operand:
                utils::constructAt(&mOperand, other.mOperand);
                break;
        }
    }

    constexpr Argument& operator=(Argument&& other)
    {
        mType = other.mType;
        mIndirect = other.mIndirect;
        mAction = other.mAction;
        switch (mType)
        {
            case None:
                break;
            case Label:
                utils::constructAt(&mLabel, std::move(other.mLabel));
                break;
            case Expression:
            case SpPlusExpression:
                utils::constructAt(&mExpression, std::move(other.mExpression));
                break;
            case Operand:
                utils::constructAt(&mOperand, other.mOperand);
                break;

        }
        return *this;
    }

    constexpr Type getType() const
    {
        return mType;
    }

    constexpr bool isIndirect() const
    {
        return mIndirect;
    }

    constexpr Action getAction() const
    {
        return mAction;
    }

    constexpr const struct Expression& expression() const
    {
        assert(mType == Expression or mType == SpPlusExpression);
        return mExpression;
    }

    constexpr cpu::isa::Operand::Type operand() const
    {
        assert(mType == Operand);
        return mOperand;
    }

    constexpr const std::string_view& label() const
    {
        assert(mType == Label);
        return mLabel;
    }

private:
    Type   mType:3;
    bool   mIndirect:1;
    Action mAction:2;
    union
    {
        std::string_view  mLabel;
        struct Expression mExpression;
        OperandType       mOperand;
    };
};

using Arguments = std::vector<Argument>;

Argument createSpPlusExpression(Context& ctx, cpu::isa::Operand::Type, Expression expression);

}  // namespace assembler
