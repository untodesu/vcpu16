#ifndef _VCPU16_H_
#define _VCPU16_H_ 1
#include <stddef.h>

#define VCPU_MEM_SIZE       0x10000
#define VCPU_MAX_INTERRUPTS 0x100

#define VCPU_OPCODE_NOP 0x00
#define VCPU_OPCODE_HLT 0x01
#define VCPU_OPCODE_PTS 0x02
#define VCPU_OPCODE_PFS 0x03
#define VCPU_OPCODE_CAL 0x04
#define VCPU_OPCODE_RET 0x05
#define VCPU_OPCODE_IOR 0x06
#define VCPU_OPCODE_IOW 0x07
#define VCPU_OPCODE_MRD 0x08
#define VCPU_OPCODE_MWR 0x09
#define VCPU_OPCODE_CLI 0x0A
#define VCPU_OPCODE_STI 0x0B
#define VCPU_OPCODE_INT 0x0C
#define VCPU_OPCODE_RFI 0x0D
#define VCPU_OPCODE_IEQ 0x20
#define VCPU_OPCODE_INE 0x21
#define VCPU_OPCODE_IGT 0x22
#define VCPU_OPCODE_IGE 0x23
#define VCPU_OPCODE_ILT 0x24
#define VCPU_OPCODE_ILE 0x25
#define VCPU_OPCODE_MOV 0x30
#define VCPU_OPCODE_ADD 0x31
#define VCPU_OPCODE_SUB 0x32
#define VCPU_OPCODE_MUL 0x33
#define VCPU_OPCODE_DIV 0x34
#define VCPU_OPCODE_MOD 0x35
#define VCPU_OPCODE_SHL 0x36
#define VCPU_OPCODE_SHR 0x37
#define VCPU_OPCODE_AND 0x38
#define VCPU_OPCODE_BOR 0x39
#define VCPU_OPCODE_XOR 0x3A
#define VCPU_OPCODE_NOT 0x3B
#define VCPU_OPCODE_INC 0x3C
#define VCPU_OPCODE_DEC 0x3D

#define VCPU_REGISTER_R0 0x00
#define VCPU_REGISTER_R1 0x01
#define VCPU_REGISTER_R2 0x02
#define VCPU_REGISTER_R3 0x03
#define VCPU_REGISTER_R4 0x04
#define VCPU_REGISTER_R5 0x05
#define VCPU_REGISTER_R6 0x06
#define VCPU_REGISTER_R7 0x07
#define VCPU_REGISTER_R8 0x08
#define VCPU_REGISTER_R9 0x09
#define VCPU_REGISTER_RI 0x0A
#define VCPU_REGISTER_RJ 0x0B
#define VCPU_REGISTER_IA 0x0C
#define VCPU_REGISTER_OF 0x0D
#define VCPU_REGISTER_SP 0x0E
#define VCPU_REGISTER_PC 0x0F

struct vcpu_instruction {
    unsigned char opcode;
    struct {
        unsigned char imm;
        unsigned char reg;
    } a, b;
};

struct vcpu;
typedef void(*vcpu_ioread_t)(struct vcpu *cpu, unsigned short port, unsigned short *value);
typedef void(*vcpu_iowrite_t)(struct vcpu *cpu, unsigned short port, unsigned short value);

struct vcpu_interrupt_queue {
    int busy, enabled;
    int queue_size;
    unsigned short queue[VCPU_MAX_INTERRUPTS];
};

typedef unsigned short vcpu_memory_t[VCPU_MEM_SIZE];

struct vcpu {
    int runtime_flags;
    vcpu_memory_t *memory;
    unsigned short regs[16];
    struct vcpu_interrupt_queue interrupts;
    vcpu_ioread_t on_ioread;
    vcpu_iowrite_t on_iowrite;
};

void init_vcpu(struct vcpu *cpu, vcpu_memory_t *shared_memory);
void shutdown_vcpu(struct vcpu *cpu);
void vcpu_interrupt(struct vcpu *cpu, unsigned short message);
int vcpu_step(struct vcpu *cpu);

#if defined(_WIN32)
#include <windows.h>
static inline unsigned short vcpu_be16_to_host(unsigned short word) { return htons(word); }
static inline unsigned short vcpu_host_to_be16(unsigned short host) { return ntohs(host); }
#elif defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>
static inline unsigned short vcpu_be16_to_host(unsigned short word) { return be16toh(word); }
static inline unsigned short vcpu_host_to_be16(unsigned short host) { return htobe16(host); }
#elif defined(__OpenBSD__)
#include <sys/endian.h>
static inline unsigned short vcpu_be16_to_host(unsigned short word) { return be16toh(word); }
static inline unsigned short vcpu_host_to_be16(unsigned short host) { return htobe16(host); }
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
static inline unsigned short vcpu_be16_to_host(unsigned short word) { return betoh16(word); }
static inline unsigned short vcpu_host_to_be16(unsigned short host) { return htobe16(host); }
#else
#error Platform is unsupported!
#endif

#endif
