#include "printers.hpp"

#include "memory/cartridge.hpp"

using namespace memory;

fmt::format_context::iterator fmt::formatter<CartridgeType>::format(const memory::CartridgeType type, format_context& ctx) const
{
    switch (type)
    {
        case CartridgeType::ROM_ONLY:
            return format_to(ctx.out(), "ROM only");
        case CartridgeType::MBC1:
            return format_to(ctx.out(), "MBC1");
        case CartridgeType::MBC1_RAM:
            return format_to(ctx.out(), "MBC1+RAM");
        case CartridgeType::MBC1_RAM_BATTERY:
            return format_to(ctx.out(), "MBC1+RAM+BATTERY");
        case CartridgeType::MBC2:
            return format_to(ctx.out(), "MBC2");
        case CartridgeType::MBC2_BATTERY:
            return format_to(ctx.out(), "MBC2+BATTERY");
        case CartridgeType::ROM_RAM:
            return format_to(ctx.out(), "ROM+RAM");
        case CartridgeType::ROM_RAM_BATTERY:
            return format_to(ctx.out(), "ROM+RAM+BATTERY");
        case CartridgeType::MMM01:
            return format_to(ctx.out(), "MMM01");
        case CartridgeType::MMM01_RAM:
            return format_to(ctx.out(), "MMM01+RAM");
        case CartridgeType::MMM01_RAM_BATTERY:
            return format_to(ctx.out(), "MMM01+RAM+BATTERY");
        case CartridgeType::MBC3_TIMER_BATTERY:
            return format_to(ctx.out(), "MBC3+TIMER+BATTERY");
        case CartridgeType::MBC3_TIMER_RAM_BATTERY:
            return format_to(ctx.out(), "MBC3+TIMER+RAM+BATTERY");
        case CartridgeType::MBC3:
            return format_to(ctx.out(), "MBC3");
        case CartridgeType::MBC3_RAM:
            return format_to(ctx.out(), "MBC3+RAM");
        case CartridgeType::MBC3_RAM_BATTERY:
            return format_to(ctx.out(), "MBC3+RAM+BATTERY");
        case CartridgeType::MBC5:
            return format_to(ctx.out(), "MBC5");
        case CartridgeType::MBC5_RAM:
            return format_to(ctx.out(), "MBC5+RAM");
        case CartridgeType::MBC5_RAM_BATTERY:
            return format_to(ctx.out(), "MBC5+RAM+BATTERY");
        case CartridgeType::MBC5_RUMBLE:
            return format_to(ctx.out(), "MBC5+RUMBLE");
        case CartridgeType::MBC5_RUMBLE_RAM:
            return format_to(ctx.out(), "MBC5+RUMBLE+RAM");
        case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
            return format_to(ctx.out(), "MBC5+RUMBLE+RAM+BATTERY");
        case CartridgeType::MBC6:
            return format_to(ctx.out(), "MBC6");
        case CartridgeType::MBC7_SENSOR_RUMBLE_RAM_BATTERY:
            return format_to(ctx.out(), "MBC7+SENSOR+RUMBLE+RAM+BATTERY");
        case CartridgeType::POCKET_CAMERA:
            return format_to(ctx.out(), "POCKET CAMERA");
        case CartridgeType::BANDAI_TAMA5:
            return format_to(ctx.out(), "BANDAI TAMA5");
        case CartridgeType::HuC3:
            return format_to(ctx.out(), "HuC3");
        case CartridgeType::HuC1_RAM_BATTERY:
            return format_to(ctx.out(), "HuC1+RAM+BATTERY");
    }
    return format_to(ctx.out(), "Unknown({})", int(type));
}
