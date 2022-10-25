/* SPDX-License-Identifier: BSD-2-Clause */
/* 
 * Copyright (c) 2022, Kirill GPRB
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <functional>
#include <dvcpu.hpp>

dvcpu::Memory::Memory(std::size_t size)
    : memory(nullptr), size(size)
{
    memory = new word_t[size];
}

dvcpu::Memory::~Memory()
{
    delete[] memory;
}

void dvcpu::Memory::init(CPU *cpu)
{

}

void dvcpu::Memory::ldri(CPU *cpu, word_t addr, word_t &value)
{

}

void dvcpu::Memory::stri(CPU *cpu, word_t addr, word_t value)
{

}

dvcpu::word_t dvcpu::Memory::getVendorID() const
{
    return DVCPU_VENDOR_ID;
}

dvcpu::word_t dvcpu::Memory::getDeviceID() const
{
    return MEMORY_DEVICE_ID;
}

dvcpu::word_t dvcpu::Memory::read(word_t address) const
{
    if(address >= ADDRESS_LIMIT)
        return 0x0000;
    return memory[address];
}

void dvcpu::Memory::write(word_t address, word_t value)
{
    if(address >= ADDRESS_LIMIT)
        return;
    memory[address] = value;
}

dvcpu::CPU::CPU(std::size_t speed)
    : flags(0), device_error(Error::OK), memory(nullptr), devices(), io_map(), registers(), parse_flags(0), instruction()
{
    // Default values
    registers[REG_ZR] = 0x0000;
    registers[REG_R0] = 0x0000;
    registers[REG_R1] = 0x0000;
    registers[REG_R2] = 0x0000;
    registers[REG_R3] = 0x0000;
    registers[REG_R4] = 0x0000;
    registers[REG_R5] = 0x0000;
    registers[REG_R6] = 0x0000;
    registers[REG_R7] = 0x0000;
    registers[REG_RI] = 0xFFFF;
    registers[REG_RJ] = 0xFFFF;
    registers[REG_IA] = 0x0000;
    registers[REG_EX] = 0xFFFF;
    registers[REG_SP] = 0xFFFF;
    registers[REG_IP] = 0x0000;
    registers[REG_TS] = 0x0000;
}

dvcpu::dword_t dvcpu::CPU::attach(std::shared_ptr<Memory> memory)
{
    return attach(std::static_pointer_cast<IDevice>(this->memory = memory));
}

dvcpu::dword_t dvcpu::CPU::attach(std::shared_ptr<IDevice> device)
{
    dword_t index = static_cast<dword_t>(devices.size());
    devices.push_back(device);
    return index;
}

dvcpu::Error dvcpu::CPU::interrupt(word_t data)
{
    if(!memory)
        return Error::NO_MEMORY;

    if(!(flags & FLAG_IF))
        return Error::OK;

    pushToStack(registers[REG_EX]);
    pushToStack(registers[REG_IP]);
    registers[REG_EX] = data;
    registers[REG_IP] = registers[REG_IA];
    return Error::OK;
}

dvcpu::Error dvcpu::CPU::pushToStack(word_t value)
{
    if(!memory)
        return Error::NO_MEMORY;
    memory->write(registers[REG_SP]--, value);
    return Error::OK;
}

dvcpu::Error dvcpu::CPU::popFromStack(word_t &value)
{
    if(!memory)
        return Error::NO_MEMORY;
    value = memory->read(--registers[REG_SP]);
    return Error::OK;
}

void dvcpu::CPU::ldri(word_t io, word_t &value)
{
    const auto it = io_map.find(io);
    if(it == io_map.cend())
        return;
    it->second->ldri(this, io, value);
}

void dvcpu::CPU::stri(word_t io, word_t value)
{
    const auto it = io_map.find(io);
    if(it == io_map.cend())
        return;
    it->second->stri(this, io, value);
}

static inline void setCPUFlag(dvcpu::CPU *cpu, dvcpu::flag_t flag, bool set)
{
    if(set)
        cpu->flags |= flag;
    else
        cpu->flags &= ~flag;
}

static inline void setValue(dvcpu::CPU *cpu, dvcpu::dword_t value, dvcpu::word_t *ref)
{
    if(ref)
        ref[0] = value & 0xFFFF;
    setCPUFlag(cpu, dvcpu::FLAG_ZF, value == 0);
    cpu->registers[dvcpu::REG_EX] = (value >> 16) & 0xFFFF;
}

dvcpu::Error dvcpu::CPU::tick()
{
    if(!memory)
        return Error::NO_MEMORY;

    if(device_error != Error::OK)
        return device_error;

    if(!(parse_flags & PARSE_CNTRL)) {
        parse_flags |= PARSE_CNTRL;

        const word_t word = memory->read(registers[REG_IP]++);

        instruction.opcode = static_cast<Opcode>((word >> 10) & 0x3F);
        instruction.a.reg = nullptr;
        instruction.a.value = 0x0000;
        instruction.b.reg = nullptr;
        instruction.b.value = 0x0000;

        if(!((word >> 9) & 0x01)) {
            instruction.a.reg = &registers[(word >> 5) & 0x0F];
            instruction.a.value = instruction.a.reg[0];
        }

        if(!((word >> 4) & 0x01)) {
            instruction.b.reg = &registers[(word >> 0) & 0x0F];
            instruction.b.value = instruction.b.reg[0];
        }

        return Error::OK;
    }

    if(!(parse_flags & PARSE_IMM_1) && !instruction.a.reg) {
        parse_flags |= PARSE_IMM_1;
        instruction.a.value = memory->read(registers[REG_IP]++);
        return Error::OK;
    }

    if(!(parse_flags & PARSE_IMM_2) && !instruction.b.reg) {
        parse_flags |= PARSE_IMM_2;
        instruction.b.value = memory->read(registers[REG_IP]++);
        return Error::OK;
    }

    // Ready to parse once again
    parse_flags = 0;

    static const std::unordered_map<Opcode, std::function<Error()>> instructions = {
        {
            Opcode::SKIP,
            [&]() -> Error
            {
                return Error::OK;
            }
        },
        {
            Opcode::HALT,
            [&]() -> Error
            {
                return Error::DEADLOCK;
            }
        },
        {
            Opcode::PUSH,
            [&]() -> Error
            {
                pushToStack(instruction.a.value);
                return Error::OK;
            }
        },
        {
            Opcode::POP,
            [&]() -> Error
            {
                popFromStack(instruction.a.reg[0]);
                return Error::OK;
            }
        },
        {
            Opcode::CALL,
            [&]() -> Error
            {
                word_t ip = registers[REG_IP];
                registers[REG_IP] = instruction.a.value;
                pushToStack(ip);
                return Error::OK;
            }
        },
        {
            Opcode::RETURN,
            [&]() -> Error
            {
                popFromStack(registers[REG_IP]);
                return Error::OK;
            }
        },
        {
            Opcode::LDRI,
            [&]() -> Error
            {
                if(instruction.b.reg)
                    ldri(instruction.a.value, instruction.b.reg[0]);
                return Error::OK;
            }
        },
        {
            Opcode::LDRM,
            [&]() -> Error
            {
                if(instruction.b.reg)
                    instruction.b.reg[0] = memory->read(instruction.a.value);
                return Error::OK;
            }
        },
        {
            Opcode::STRI,
            [&]() -> Error
            {
                stri(instruction.b.value, instruction.a.value);
                return Error::OK;
            }
        },
        {
            Opcode::STRM,
            [&]() -> Error
            {
                memory->write(instruction.b.value, instruction.a.value);
                return Error::OK;
            }
        },
        {
            Opcode::CCIF,
            [&]() -> Error
            {
                setCPUFlag(this, FLAG_IF, false);
                return Error::OK;
            }
        },
        {
            Opcode::SCIF,
            [&]() -> Error
            {
                setCPUFlag(this, FLAG_IF, true);
                return Error::OK;
            }
        },
        {
            Opcode::INT,
            [&]() -> Error
            {
                interrupt(instruction.a.value);
                return Error::OK;
            }
        },
        {
            Opcode::IRETURN,
            [&]() -> Error
            {
                popFromStack(registers[REG_IP]);
                popFromStack(registers[REG_EX]);
                return Error::OK;
            }
        },
        {
            Opcode::CPUDEF,
            [&]() -> Error
            {
                if(instruction.b.reg) {
                    instruction.b.reg[0] = DVCPU_VENDOR_ID;
                }
                
                return Error::OK;
            }
        },
        {
            Opcode::CMP,
            [&]() -> Error
            {
                setCPUFlag(this, FLAG_EF | FLAG_ZF, instruction.b.value == instruction.a.value);
                setCPUFlag(this, FLAG_GF, instruction.b.value > instruction.a.value);
                setCPUFlag(this, FLAG_LF, instruction.b.value < instruction.a.value);
                return Error::OK;
            }
        },
        {
            Opcode::MOV,
            [&]() -> Error
            {
                setValue(this, instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::ADD,
            [&]() -> Error
            {
                setValue(this, instruction.b.value + instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::SUB,
            [&]() -> Error
            {
                setValue(this, instruction.b.value - instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::MUL,
            [&]() -> Error
            {
                setValue(this, instruction.b.value * instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::DIV,
            [&]() -> Error
            {
                if(instruction.a.value)
                    setValue(this, instruction.b.value / instruction.a.value, instruction.b.reg);
                else
                    setValue(this, 0xFFFFFFFF, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::MOD,
            [&]() -> Error
            {
                if(instruction.a.value)
                    setValue(this, instruction.b.value % instruction.a.value, instruction.b.reg);
                else
                    setValue(this, instruction.b.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::SHL,
            [&]() -> Error
            {
                setValue(this, instruction.b.value << instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::SHR,
            [&]() -> Error
            {
                setValue(this, instruction.b.value >> instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::AND,
            [&]() -> Error
            {
                setValue(this, instruction.b.value & instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::BOR,
            [&]() -> Error
            {
                setValue(this, instruction.b.value | instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::XOR,
            [&]() -> Error
            {
                setValue(this, instruction.b.value ^ instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::NOT,
            [&]() -> Error
            {
                setValue(this, ~instruction.a.value, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::INCR,
            [&]() -> Error
            {
                setValue(this, instruction.a.value + 1, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::DECR,
            [&]() -> Error
            {
                setValue(this, instruction.a.value - 1, instruction.b.reg);
                return Error::OK;
            }
        },
        {
            Opcode::MOVII,
            [&]() -> Error
            {
                setValue(this, instruction.a.value, instruction.b.reg);
                registers[REG_RI]++;
                return Error::OK;
            }
        },
        {
            Opcode::MOVIJ,
            [&]() -> Error
            {
                setValue(this, instruction.a.value, instruction.b.reg);
                registers[REG_RJ]++;
                return Error::OK;
            }
        },
        {
            Opcode::MOVDI,
            [&]() -> Error
            {
                setValue(this, instruction.a.value, instruction.b.reg);
                registers[REG_RI]--;
                return Error::OK;
            }
        },
        {
            Opcode::MOVDJ,
            [&]() -> Error
            {
                setValue(this, instruction.a.value, instruction.b.reg);
                registers[REG_RJ]--;
                return Error::OK;
            }
        },
        {
            Opcode::JMP,
            [&]() -> Error
            {
                registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JZ,
            [&]() -> Error
            {
                if(flags & FLAG_ZF)
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JNZ,
            [&]() -> Error
            {
                if(!(flags & FLAG_ZF))
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JE,
            [&]() -> Error
            {
                if(flags & FLAG_EF)
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JNE,
            [&]() -> Error
            {
                if(!(flags & FLAG_EF))
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JG,
            [&]() -> Error
            {
                if(flags & FLAG_GF)
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JGE,
            [&]() -> Error
            {
                if((flags & FLAG_GF) || (flags & FLAG_EF))
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JL,
            [&]() -> Error
            {
                if(flags & FLAG_LF)
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JLE,
            [&]() -> Error
            {
                if((flags & FLAG_LF) || (flags & FLAG_EF))
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::JD,
            [&]() -> Error
            {
                if(flags & FLAG_DF)
                    registers[REG_IP] = instruction.a.value;
                return Error::OK;
            }
        },
        {
            Opcode::LOOPI,
            [&]() -> Error
            {
                if(registers[REG_RI]) {
                    registers[REG_IP] = instruction.a.value;
                    registers[REG_RI]--;
                }

                return Error::OK;
            }
        },
        {
            Opcode::LOOPJ,
            [&]() -> Error
            {
                if(registers[REG_RJ]) {
                    registers[REG_IP] = instruction.a.value;
                    registers[REG_RJ]--;
                }

                return Error::OK;
            }
        },
    };

    const auto it = instructions.find(instruction.opcode);
    if(it != instructions.cend()) {

        return it->second();
    }

    return Error::OK;
}
