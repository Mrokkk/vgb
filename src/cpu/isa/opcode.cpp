#include "opcode.hpp"

namespace cpu::isa
{

const char* opcodeName(Opcode::Type t)
{
    switch (t)
    {
        case  Opcode::ILL:      return  "ill";
        case  Opcode::ADC:      return  "adc";
        case  Opcode::ADD:      return  "add";
        case  Opcode::AND:      return  "and";
        case  Opcode::BIT:      return  "bit";
        case  Opcode::CALL:     return  "call";
        case  Opcode::CCF:      return  "ccf";
        case  Opcode::CP:       return  "cp";
        case  Opcode::CPL:      return  "cpl";
        case  Opcode::DAA:      return  "daa";
        case  Opcode::DEC:      return  "dec";
        case  Opcode::DI:       return  "di";
        case  Opcode::EI:       return  "ei";
        case  Opcode::HALT:     return  "halt";
        case  Opcode::INC:      return  "inc";
        case  Opcode::JP:       return  "jp";
        case  Opcode::JR:       return  "jr";
        case  Opcode::LD:       return  "ld";
        case  Opcode::LDH:      return  "ldh";
        case  Opcode::NOP:      return  "nop";
        case  Opcode::OR:       return  "or";
        case  Opcode::POP:      return  "pop";
        case  Opcode::PREFIX:   return  "prefix";
        case  Opcode::PUSH:     return  "push";
        case  Opcode::RES:      return  "res";
        case  Opcode::RET:      return  "ret";
        case  Opcode::RETI:     return  "reti";
        case  Opcode::RL:       return  "rl";
        case  Opcode::RLA:      return  "rla";
        case  Opcode::RLC:      return  "rlc";
        case  Opcode::RLCA:     return  "rlca";
        case  Opcode::RR:       return  "rr";
        case  Opcode::RRA:      return  "rra";
        case  Opcode::RRC:      return  "rrc";
        case  Opcode::RRCA:     return  "rrca";
        case  Opcode::RST:      return  "rst";
        case  Opcode::SBC:      return  "sbc";
        case  Opcode::SCF:      return  "scf";
        case  Opcode::SET:      return  "set";
        case  Opcode::SLA:      return  "sla";
        case  Opcode::SRA:      return  "sra";
        case  Opcode::SRL:      return  "srl";
        case  Opcode::STOP:     return  "stop";
        case  Opcode::SUB:      return  "sub";
        case  Opcode::SWAP:     return  "swap";
        case  Opcode::XOR:      return  "xor";
    }
    return "INVALID";
}

}  // namespace cpu::isa
