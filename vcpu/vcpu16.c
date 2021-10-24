#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "vcpu16.h"

#define RUNTIME_FLAG_HALT           (1 << 0)
#define RUNTIME_FLAG_SHARED_MEMORY  (1 << 1)

struct instruction_internal {
    struct vcpu_instruction instruction;
    struct {
        unsigned short value;
        unsigned short *ref;
    } a, b;
};

static void vcpu_parse(struct vcpu *cpu, struct instruction_internal *instruction)
{
    unsigned short word = (*cpu->memory)[cpu->regs[VCPU_REGISTER_PC]++];

    instruction->instruction.opcode = (word >> 10) & 0x3F;
    instruction->instruction.a.imm = (word >> 9) & 0x01;
    instruction->instruction.a.reg = (word >> 5) & 0x0F;
    instruction->instruction.b.imm = (word >> 4) & 0x01;
    instruction->instruction.b.reg = word & 0x0F;

    /* Parse A */
    if(instruction->instruction.a.imm) {
        instruction->a.ref = NULL;
        instruction->a.value = (*cpu->memory)[cpu->regs[VCPU_REGISTER_PC]++];
    }
    else {
        instruction->a.ref = cpu->regs + instruction->instruction.a.reg;
        instruction->a.value = *instruction->a.ref;
    }

    /* Parse B */
    if(instruction->instruction.b.imm) {
        instruction->b.ref = NULL;
        instruction->b.value = (*cpu->memory)[cpu->regs[VCPU_REGISTER_PC]++];
    }
    else {
        instruction->b.ref = cpu->regs + instruction->instruction.b.reg;
        instruction->b.value = *instruction->b.ref;
    }
}

static void vcpu_set_value(struct vcpu *cpu, unsigned int value, unsigned short *destination)
{
    if(destination)
        *destination = value & 0xFFFF;
    cpu->regs[VCPU_REGISTER_OF] = (value >> 16) & 0xFFFF;
}

void init_vcpu(struct vcpu *cpu, vcpu_memory_t *shared_memory)
{
    memset(cpu, 0, sizeof(struct vcpu));

    if(shared_memory) {
        cpu->runtime_flags |= RUNTIME_FLAG_SHARED_MEMORY;
        cpu->memory = shared_memory;
    }
    else {
        cpu->memory = malloc(sizeof(vcpu_memory_t));
        assert(("Out of memory!", cpu->memory));
        memset(*cpu->memory, 0, sizeof(vcpu_memory_t));
    }

    cpu->regs[VCPU_REGISTER_IA] = 0x0000;
    cpu->regs[VCPU_REGISTER_OF] = 0x0000;
    cpu->regs[VCPU_REGISTER_SP] = 0xFFFF;
    cpu->regs[VCPU_REGISTER_PC] = 0x0000;

    cpu->on_ioread = NULL;
    cpu->on_iowrite;

    cpu->cpi.vendor_id = VCPU_CPI_DEF_VENDOR_ID;
    cpu->cpi.speed = VCPU_CPI_DEF_SPEED;
}

void shutdown_vcpu(struct vcpu *cpu)
{
    if(!(cpu->runtime_flags & RUNTIME_FLAG_SHARED_MEMORY))
        free(cpu->memory);
    memset(cpu, 0, sizeof(struct vcpu));
}

void vcpu_interrupt(struct vcpu *cpu, unsigned short message)
{
    if(cpu->interrupts.enabled) {
        if(cpu->interrupts.queue_size >= VCPU_MAX_INTERRUPTS) {
            cpu->runtime_flags |= RUNTIME_FLAG_HALT;
            cpu->interrupts.enabled = 0;
            return;
        }

        cpu->runtime_flags &= ~RUNTIME_FLAG_HALT;
        cpu->interrupts.queue[cpu->interrupts.queue_size++] = message;
    }
}

int vcpu_step(struct vcpu *cpu)
{
    int result = 1;
    unsigned short scratch;
    struct instruction_internal instruction;

    if(cpu->runtime_flags & RUNTIME_FLAG_HALT)
        return cpu->interrupts.enabled;

    if(cpu->interrupts.enabled && !cpu->interrupts.busy && cpu->interrupts.queue_size > 0) {
        cpu->interrupts.busy = 1;
        (*cpu->memory)[cpu->regs[VCPU_REGISTER_SP]--] = cpu->regs[VCPU_REGISTER_PC];
        (*cpu->memory)[cpu->regs[VCPU_REGISTER_SP]--] = cpu->regs[VCPU_REGISTER_R0];
        cpu->regs[VCPU_REGISTER_PC] = cpu->regs[VCPU_REGISTER_IA];
        cpu->regs[VCPU_REGISTER_R0] = cpu->interrupts.queue[--cpu->interrupts.queue_size];
    }

    vcpu_parse(cpu, &instruction);
    switch(instruction.instruction.opcode) {
        case VCPU_OPCODE_NOP:
            return 1;
        case VCPU_OPCODE_HLT:
            if(!cpu->interrupts.enabled)
                return 0;
            cpu->runtime_flags |= RUNTIME_FLAG_HALT;
            return 1;
        case VCPU_OPCODE_PTS:
            (*cpu->memory)[cpu->regs[VCPU_REGISTER_SP]--] = instruction.a.value;
            return 1;
        case VCPU_OPCODE_PFS:
            vcpu_set_value(cpu, (*cpu->memory)[++cpu->regs[VCPU_REGISTER_SP]], instruction.a.ref);
            return 1;
        case VCPU_OPCODE_CAL:
            (*cpu->memory)[cpu->regs[VCPU_REGISTER_SP]--] = cpu->regs[VCPU_REGISTER_PC];
            cpu->regs[VCPU_REGISTER_PC] = instruction.a.value;
            return 1;
        case VCPU_OPCODE_RET:
            cpu->regs[VCPU_REGISTER_PC] = (*cpu->memory)[++cpu->regs[VCPU_REGISTER_SP]];
            return 1;
        case VCPU_OPCODE_IOR:
            if(cpu->on_ioread && instruction.b.ref)
                cpu->on_ioread(cpu, instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_IOW:
            if(cpu->on_iowrite)
                cpu->on_iowrite(cpu, instruction.b.value, instruction.a.value);
            return 1;
        case VCPU_OPCODE_MRD:
            vcpu_set_value(cpu, (*cpu->memory)[instruction.a.value], instruction.b.ref);
            return 1;
        case VCPU_OPCODE_MWR:
            (*cpu->memory)[instruction.b.value] = instruction.a.value;
            return 1;
        case VCPU_OPCODE_CLI:
            cpu->interrupts.enabled = 0;
            return 1;
        case VCPU_OPCODE_STI:
            cpu->interrupts.enabled = 1;
            return 1;
        case VCPU_OPCODE_INT:
            vcpu_interrupt(cpu, instruction.a.value);
            return 1;
        case VCPU_OPCODE_RFI:
            cpu->regs[VCPU_REGISTER_R0] = (*cpu->memory)[++cpu->regs[VCPU_REGISTER_SP]];
            cpu->regs[VCPU_REGISTER_PC] = (*cpu->memory)[++cpu->regs[VCPU_REGISTER_SP]];
            cpu->interrupts.busy = 0;
            return 1;
        case VCPU_OPCODE_CPI:
            cpu->regs[VCPU_REGISTER_R0] = cpu->cpi.vendor_id;
            cpu->regs[VCPU_REGISTER_R1] = (cpu->cpi.speed >> 16) & 0xFFFF;
            cpu->regs[VCPU_REGISTER_R2] = cpu->cpi.speed & 0xFFFF;
            return 1;
        case VCPU_OPCODE_IEQ:
            if(!(instruction.b.value == instruction.a.value))
                vcpu_parse(cpu, &instruction);
            return 1;
        case VCPU_OPCODE_INE:
            if(!(instruction.b.value != instruction.a.value))
                vcpu_parse(cpu, &instruction);
            return 1;
        case VCPU_OPCODE_IGT:
            if(!(instruction.b.value > instruction.a.value))
                vcpu_parse(cpu, &instruction);
            return 1;
        case VCPU_OPCODE_IGE:
            if(!(instruction.b.value >= instruction.a.value))
                vcpu_parse(cpu, &instruction);
            return 1;
        case VCPU_OPCODE_ILT:
            if(!(instruction.b.value < instruction.a.value))
                vcpu_parse(cpu, &instruction);
            return 1;
        case VCPU_OPCODE_ILE:
            if(!(instruction.b.value <= instruction.a.value))
                vcpu_parse(cpu, &instruction);
            return 1;
        case VCPU_OPCODE_MOV:
            vcpu_set_value(cpu, instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_ADD:
            vcpu_set_value(cpu, instruction.b.value + instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_SUB:
            vcpu_set_value(cpu, instruction.b.value - instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_MUL:
            vcpu_set_value(cpu, instruction.b.value * instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_DIV:
            vcpu_set_value(cpu, instruction.a.value ? (instruction.b.value / instruction.a.value) : 0, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_MOD:
            vcpu_set_value(cpu, instruction.a.value ? (instruction.b.value / instruction.a.value) : instruction.b.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_SHL:
            vcpu_set_value(cpu, instruction.b.value << instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_SHR:
            vcpu_set_value(cpu, instruction.b.value >> instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_AND:
            vcpu_set_value(cpu, instruction.b.value & instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_BOR:
            vcpu_set_value(cpu, instruction.b.value | instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_XOR:
            vcpu_set_value(cpu, instruction.b.value ^ instruction.a.value, instruction.b.ref);
            return 1;
        case VCPU_OPCODE_NOT:
            vcpu_set_value(cpu, ~instruction.a.value, instruction.a.ref);
            return 1;
        case VCPU_OPCODE_INC:
            vcpu_set_value(cpu, instruction.a.value + 1, instruction.a.ref);
            return 1;
        case VCPU_OPCODE_DEC:
            vcpu_set_value(cpu, instruction.a.value - 1, instruction.a.ref);
            return 1;
    }
}
