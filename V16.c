#include <stdlib.h>
#include <string.h>
#include "V16.h"

typedef struct V16_instruction_internal {
    V16_instruction_t info;
    uint16_t av, *ap;
    uint16_t bv, *bp;
} V16_instruction_internal_t;

static inline void V16_parse(V16_vm_t *vm, V16_instruction_internal_t *instr)
{
    instr->info.word = vm->memory[vm->regs[V16_REGISTER_PC]++];

    // "A" operand
    if(instr->info.i.a_imm) {
        instr->ap = NULL;
        instr->av = vm->memory[vm->regs[V16_REGISTER_PC]++];
    }
    else {
        instr->ap = vm->regs + instr->info.i.a_reg;
        instr->av = instr->ap[0];
    }

    // "B" operand
    if(instr->info.i.b_imm) {
        instr->bp = NULL;
        instr->bv = vm->memory[vm->regs[V16_REGISTER_PC]++];
    }
    else {
        instr->bp = vm->regs + instr->info.i.b_reg;
        instr->bv = instr->bp[0];
    }
}

static inline void V16_set(V16_vm_t *vm, uint32_t value, uint16_t *dest)
{
    if(dest) {
        dest[0] = value & 0xFFFF;
        vm->regs[V16_REGISTER_OF] = (value >> 16) & 0xFFFF;
    }
}

int V16_open(V16_vm_t *vm)
{
    memset(vm, 0, sizeof(V16_vm_t));

    vm->memory = malloc(sizeof(uint16_t) * V16_MEM_SIZE);
    if(!vm->memory)
        return 0;
    
    vm->regs[V16_REGISTER_OF] = 0xFFFF;
    vm->regs[V16_REGISTER_SP] = 0xFFFF;
    vm->regs[V16_REGISTER_PC] = 0;

    vm->ioread = NULL;
    vm->iowrite = NULL;

    return 1;
}

int V16_close(V16_vm_t *vm)
{
    free(vm->memory);
    memset(vm, 0, sizeof(V16_vm_t));
    return 1;
}

int V16_step(V16_vm_t *vm)
{
    uint16_t ioread_temp = 0;
    V16_instruction_internal_t instr;
    V16_parse(vm, &instr);

    switch(instr.info.i.opcode) {
        case V16_OPCODE_NOP:
            return 1;
        case V16_OPCODE_HLT:
            return 0;
        case V16_OPCODE_PTS:
            vm->memory[vm->regs[V16_REGISTER_SP]--] = instr.av;
            return 1;
        case V16_OPCODE_PFS:
            V16_set(vm, vm->memory[++vm->regs[V16_REGISTER_SP]], instr.ap);
            return 1;
        case V16_OPCODE_SCL:
            vm->memory[vm->regs[V16_REGISTER_SP]--] = vm->regs[V16_REGISTER_PC];
            vm->regs[V16_REGISTER_PC] = instr.av;
            return 1;
        case V16_OPCODE_SRT:
            vm->regs[V16_REGISTER_PC] = vm->memory[++vm->regs[V16_REGISTER_PC]];
            return 1;
        case V16_OPCODE_IOR:
            if(vm->ioread && vm->ioread(vm, instr.av, &ioread_temp))
                V16_set(vm, ioread_temp, instr.bp);
            return 1;
        case V16_OPCODE_IOW:
            if(vm->iowrite)
                vm->iowrite(vm, instr.bv, instr.av);
            return 1;
        case V16_OPCODE_MRD:
            V16_set(vm, vm->memory[instr.av], instr.bp);
            return 1;
        case V16_OPCODE_MWR:
            vm->memory[instr.bv] = instr.av;
            return 1;
        
        case V16_OPCODE_MOV:
            V16_set(vm, instr.av, instr.bp);
            return 1;
        case V16_OPCODE_ADD:
            V16_set(vm, instr.bv + instr.av, instr.bp);
            return 1;
        case V16_OPCODE_SUB:
            V16_set(vm, instr.bv - instr.av, instr.bp);
            return 1;
        case V16_OPCODE_MUL:
            V16_set(vm, instr.bv * instr.av, instr.bp);
            return 1;
        case V16_OPCODE_DIV:
            V16_set(vm, instr.av ? (instr.bv / instr.av) : 0, instr.bp);
            return 1;
        case V16_OPCODE_MOD:
            V16_set(vm, instr.av ? (instr.bv / instr.av) : instr.bv, instr.bp);
            return 1;
        case V16_OPCODE_SHL:
            V16_set(vm, instr.bv << instr.av, instr.bp);
            return 1;
        case V16_OPCODE_SHR:
            V16_set(vm, instr.bv >> instr.av, instr.bp);
            return 1;
        case V16_OPCODE_AND:
            V16_set(vm, instr.bv & instr.av, instr.bp);
            return 1;
        case V16_OPCODE_BOR:
            V16_set(vm, instr.bv | instr.av, instr.bp);
            return 1;
        case V16_OPCODE_XOR:
            V16_set(vm, instr.bv ^ instr.av, instr.bp);
            return 1;
        case V16_OPCODE_NOT:
            V16_set(vm, ~instr.av, instr.ap);
            return 1;
        
        case V16_OPCODE_IEQ:
            if(!(instr.bv == instr.av))
                V16_parse(vm, &instr);
            return 1;
        case V16_OPCODE_INE:
            if(!(instr.bv != instr.av))
                V16_parse(vm, &instr);
            return 1;
        case V16_OPCODE_IGT:
            if(!(instr.bv > instr.av))
                V16_parse(vm, &instr);
            return 1;
        case V16_OPCODE_IGE:
            if(!(instr.bv >= instr.av))
                V16_parse(vm, &instr);
            return 1;
        case V16_OPCODE_ILT:
            if(!(instr.bv < instr.av))
                V16_parse(vm, &instr);
            return 1;
        case V16_OPCODE_ILE:
            if(!(instr.bv <= instr.av))
                V16_parse(vm, &instr);
            return 1;
    }

    return 1;
}
