// Copyright (c) 2021, Kirill GPRB
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef V16_H_
#define V16_H_ 1
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define V16_MEM_SIZE        (0x10000)
#define V16_FREQUENCY       (100000)
#define V16_MAX_INTERRUPTS  (256)

typedef enum {
    V16_OPCODE_NOP = 0x00,
    V16_OPCODE_HLT = 0x01,
    V16_OPCODE_PTS = 0x02,
    V16_OPCODE_PFS = 0x03,
    V16_OPCODE_CAL = 0x04,
    V16_OPCODE_RET = 0x05,
    V16_OPCODE_IOR = 0x06,
    V16_OPCODE_IOW = 0x07,
    V16_OPCODE_MRD = 0x08,
    V16_OPCODE_MWR = 0x09,
    V16_OPCODE_CLI = 0x0A,
    V16_OPCODE_STI = 0x0B,
    V16_OPCODE_INT = 0x0C,
    V16_OPCODE_RFI = 0x0D,

    V16_OPCODE_MOV = 0x10,
    V16_OPCODE_ADD = 0x11,
    V16_OPCODE_SUB = 0x12,
    V16_OPCODE_MUL = 0x13,
    V16_OPCODE_DIV = 0x14,
    V16_OPCODE_MOD = 0x15,
    V16_OPCODE_SHL = 0x16,
    V16_OPCODE_SHR = 0x17,
    V16_OPCODE_AND = 0x18,
    V16_OPCODE_BOR = 0x19,
    V16_OPCODE_XOR = 0x1A,
    V16_OPCODE_NOT = 0x1B,
    V16_OPCODE_INC = 0x1C,
    V16_OPCODE_DEC = 0x1D,

    V16_OPCODE_IEQ = 0x20,
    V16_OPCODE_INE = 0x21,
    V16_OPCODE_IGT = 0x22,
    V16_OPCODE_IGE = 0x23,
    V16_OPCODE_ILT = 0x24,
    V16_OPCODE_ILE = 0x25,

} V16_opcode_t;

typedef enum {
    V16_REGISTER_R0 = 0x00,
    V16_REGISTER_R1 = 0x01,
    V16_REGISTER_R2 = 0x02,
    V16_REGISTER_R3 = 0x03,
    V16_REGISTER_R4 = 0x04,
    V16_REGISTER_R5 = 0x05,
    V16_REGISTER_R6 = 0x06,
    V16_REGISTER_R7 = 0x07,
    V16_REGISTER_R8 = 0x08,
    V16_REGISTER_R9 = 0x09,
    V16_REGISTER_RI = 0x0A,
    V16_REGISTER_RJ = 0x0B,
    V16_REGISTER_IA = 0x0C,
    V16_REGISTER_OF = 0x0D,
    V16_REGISTER_SP = 0x0E,
    V16_REGISTER_PC = 0x0F,
    V16_REGISTER_COUNT = 16
} V16_register_t;

typedef struct V16_instruction {
    uint8_t opcode : 6;
    uint8_t a_imm : 1;
    uint8_t a_reg : 4;
    uint8_t b_imm : 1;
    uint8_t b_reg : 4;
} V16_instruction_t;

struct V16_vm;

typedef bool(*V16_ioread_func_t)(struct V16_vm *vm, uint16_t port, uint16_t *value);
typedef void(*V16_iowrite_func_t)(struct V16_vm *vm, uint16_t port, uint16_t value);

typedef struct V16_intqueue {
    bool busy;
    bool enabled;
    int queue_size;
    uint16_t queue[V16_MAX_INTERRUPTS];
} V16_intqueue_t;

typedef struct V16_vm {
    int halt;
    uint16_t *memory;
    uint16_t regs[V16_REGISTER_COUNT];
    V16_intqueue_t intqueue;
    V16_ioread_func_t ioread;
    V16_iowrite_func_t iowrite;
} V16_vm_t;

bool V16_open(V16_vm_t *vm);
void V16_close(V16_vm_t *vm);
bool V16_step(V16_vm_t *vm, size_t *cycles);
void V16_interrupt(V16_vm_t *vm, uint16_t id);

#if defined(_WIN32)
#include <windows.h>
static inline uint16_t V16_BE16ToHost(uint16_t word) { return htons(word); }
static inline uint16_t V16_hostToBE16(uint16_t host) { return ntohs(host); }
#elif defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>
static inline uint16_t V16_BE16ToHost(uint16_t word) { return be16toh(word); }
static inline uint16_t V16_hostToBE16(uint16_t host) { return htobe16(host); }
#elif defined(__OpenBSD__)
#include <sys/endian.h>
static inline uint16_t V16_BE16ToHost(uint16_t word) { return be16toh(word); }
static inline uint16_t V16_hostToBE16(uint16_t host) { return htobe16(host); }
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
static inline uint16_t V16_BE16ToHost(uint16_t word) { return betoh16(word); }
static inline uint16_t V16_hostToBE16(uint16_t host) { return htobe16(host); }
#else
#error Platform is unsupported!
#endif

#endif
