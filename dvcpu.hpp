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
#ifndef A00A0E14_6ABB_44A8_813C_6D7B2165CBAB
#define A00A0E14_6ABB_44A8_813C_6D7B2165CBAB
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#define dvcpu_be16_to_host(word) htons(word)
#define dvcpu_host_to_be16(word) ntohs(word)
#elif defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>
#define dvcpu_be16_to_host(word) be16toh(word)
#define dvcpu_host_to_be16(word) htobe16(word)
#elif defined(__OpenBSD__)
#include <sys/endian.h>
#define dvcpu_be16_to_host(word) be16toh(word)
#define dvcpu_host_to_be16(word) htobe16(word)
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
#define dvcpu_be16_to_host(word) betoh16(word)
#define dvcpu_host_to_be16(word) htobe16(word)
#elif defined(__APPLE__)
#include <machine/endian.h>
#define dvcpu_be16_to_host(word) htons(word)
#define dvcpu_host_to_be16(word) ntohs(word)
#else
#error Platform is unsupported!
#endif

namespace dvcpu
{
using byte_t = std::uint8_t;
using word_t = std::uint16_t;
using dword_t = std::uint32_t;

constexpr static const std::size_t ADDRESS_LIMIT = 0x10000;
constexpr static const std::size_t IOPORT_LIMIT = 0x10000;
constexpr static const std::size_t NUM_REGISTERS = 16;

constexpr static const word_t DVCPU_VENDOR_ID = 0x1F1F;
constexpr static const word_t MEMORY_DEVICE_ID = 0xFFFF;

using register_t = unsigned char;
constexpr static const register_t REG_ZR = 0x00; // Always zero
constexpr static const register_t REG_R0 = 0x01; // General-purpose
constexpr static const register_t REG_R1 = 0x02; // General-purpose
constexpr static const register_t REG_R2 = 0x03; // General-purpose
constexpr static const register_t REG_R3 = 0x04; // General-purpose
constexpr static const register_t REG_R4 = 0x05; // General-purpose
constexpr static const register_t REG_R5 = 0x06; // General-purpose
constexpr static const register_t REG_R6 = 0x07; // General-purpose
constexpr static const register_t REG_R7 = 0x08; // General-purpose
constexpr static const register_t REG_RI = 0x09; // Index register
constexpr static const register_t REG_RJ = 0x0A; // Index register
constexpr static const register_t REG_IA = 0x0B; // Interrupt address
constexpr static const register_t REG_EX = 0x0C; // Extra, 32-bit overflow
constexpr static const register_t REG_SP = 0x0D; // Stack pointer
constexpr static const register_t REG_IP = 0x0E; // Instruction pointer
constexpr static const register_t REG_TS = 0x0F; // Timestamp counter

using flag_t = unsigned short;
constexpr static const flag_t FLAG_IF = flag_t(1 << 0); // Interrupts
constexpr static const flag_t FLAG_DF = flag_t(1 << 1); // Device present
constexpr static const flag_t FLAG_ZF = flag_t(1 << 2); // Zero
constexpr static const flag_t FLAG_EF = flag_t(1 << 3); // Equals
constexpr static const flag_t FLAG_GF = flag_t(1 << 4); // Greater than
constexpr static const flag_t FLAG_LF = flag_t(1 << 5); // Lower than

enum class Error : unsigned short {
    OK          = 0x0000, // OK
    MELTDOWN    = 0x0001, // Device or CPU died a horrible death
    DEADLOCK    = 0x0002, // Halting with interrupts disabled
    NO_MEMORY   = 0x0003, // No memory device attached
};

enum class Opcode : byte_t {
    SKIP    = 0x00, // skip
    HALT    = 0x01, // halt

    PUSH    = 0x02, // push     <val>
    POP     = 0x03, // pop      <ref>
    CALL    = 0x04, // call     <val>
    RETURN  = 0x05, // return

    LDRI    = 0x06, // ldri     <val> <ref>
    LDRM    = 0x07, // ldrm     <val> <ref>
    STRI    = 0x08, // stri     <val> <val>
    STRM    = 0x09, // strm     <val> <val>

    CCIF    = 0x0A, // ccif
    SCIF    = 0x0B, // scif
    INT     = 0x0C, // int      <val>
    IRETURN = 0x0D, // ireturn

    CPUDEF  = 0x0E, // cpudef   <val> <ref>

    CMP     = 0x0F, // cmp      <val> <val>

    MOV     = 0x10, // mov      <val> <ref>
    ADD     = 0x11, // add      <val> <ref>
    SUB     = 0x12, // sub      <val> <ref>
    MUL     = 0x13, // mul      <val> <ref>
    DIV     = 0x14, // div      <val> <ref>
    MOD     = 0x15, // mod      <val> <ref>
    SHL     = 0x16, // shl      <val> <ref>
    SHR     = 0x17, // shr      <val> <ref>
    AND     = 0x18, // and      <val> <ref>
    BOR     = 0x19, // bor      <val> <ref>
    XOR     = 0x1A, // xor      <val> <ref>
    NOT     = 0x1B, // not      <ref>

    INCR    = 0x1C, // incr     <ref>
    DECR    = 0x1D, // decr     <ref>
    MOVII   = 0x1E, // movii    <val> <ref>
    MOVIJ   = 0x1F, // movij    <val> <ref>
    MOVDI   = 0x20, // movdi    <val> <ref>
    MOVDJ   = 0x21, // movdj    <val> <ref>

    JMP     = 0x22, // jmp      <val>
    JZ      = 0x23, // jz       <val>
    JNZ     = 0x24, // jnz      <val>
    JE      = 0x25, // je       <val>
    JNE     = 0x26, // jne      <val>
    JG      = 0x27, // jg       <val>
    JGE     = 0x28, // jge      <val>
    JL      = 0x29, // jl       <val>
    JLE     = 0x2A, // jle      <val>
    JD      = 0x2B, // jd       <val>
    LOOPI   = 0x2C, // loopi    <val>
    LOOPJ   = 0x2D, // loopj    <val>
};

struct Instruction final {
    Opcode opcode { Opcode::SKIP };
    struct {
        word_t *reg { nullptr };
        word_t value { 0x0000 };
    } a, b;
};

using parse_flags_t = unsigned short;
constexpr static const parse_flags_t PARSE_CNTRL = parse_flags_t(1 << 0);
constexpr static const parse_flags_t PARSE_IMM_1 = parse_flags_t(1 << 1);
constexpr static const parse_flags_t PARSE_IMM_2 = parse_flags_t(1 << 1);

class CPU;
class IDevice {
public:
    virtual ~IDevice() = default;
    virtual void init(CPU *cpu) = 0;
    virtual void ldri(CPU *cpu, word_t io, word_t &value) = 0;
    virtual void stri(CPU *cpu, word_t io, word_t value) = 0;
    virtual word_t getVendorID() const = 0;
    virtual word_t getDeviceID() const = 0;
};

class Memory final : public IDevice {
public:
    Memory(std::size_t size);
    virtual ~Memory();

    void init(CPU *cpu) override;
    void ldri(CPU *cpu, word_t io, word_t &value) override;
    void stri(CPU *cpu, word_t io, word_t value) override;
    word_t getVendorID() const override;
    word_t getDeviceID() const override;

    std::size_t getSize() const;
    word_t read(word_t address) const;
    void write(word_t address, word_t value);

private:
    word_t *memory;
    std::size_t size;
};

class CPU final {
public:
    CPU(std::size_t speed);
    virtual ~CPU() = default;

    // Attaching devices. Memory is a special
    // case because we need its functions without
    // using any bullshit fucking garbage RTTI is.
    dword_t attach(std::shared_ptr<Memory> memory);
    dword_t attach(std::shared_ptr<IDevice> device);

    Error interrupt(word_t data);

    Error pushToStack(word_t value);
    Error popFromStack(word_t &value);

    void ldri(word_t io, word_t &value);
    void stri(word_t io, word_t value);

    Error tick();

public:
    flag_t flags;
    Error device_error;
    std::shared_ptr<Memory> memory;
    std::array<word_t, NUM_REGISTERS> registers;
    std::vector<std::shared_ptr<IDevice>> devices;
    std::unordered_map<word_t, std::shared_ptr<IDevice>> io_map;
    parse_flags_t parse_flags;
    Instruction instruction;
};
} // namespace dvcpu

#endif /* A00A0E14_6ABB_44A8_813C_6D7B2165CBAB */
