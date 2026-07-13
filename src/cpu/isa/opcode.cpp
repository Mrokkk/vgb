#include "opcode.hpp"

namespace cpu::isa
{

const char* opcodeName(Opcode::Type t)
{
    switch (t)
    {
#define MNEMO(UPPER, LOWER) case Opcode::UPPER: return #LOWER;
#define MNEMO_ILL MNEMO
#include "cpu/isa/mnemos.hpp"
    }
    return "INVALID";
}

}  // namespace cpu::isa
