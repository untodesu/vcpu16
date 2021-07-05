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
    uint16_t word = vm->memory[vm->regs[V16_REGISTER_PC]++];

    instr->info.opcode = (word >> 10) & 0x3F;
    instr->info.a_imm = (word >> 9) & 0x01;
    instr->info.a_reg = (word >> 5) & 0x0F;
    instr->info.b_imm = (word >> 4) & 0x01;
    instr->info.b_reg = word & 0x0F;

    // "A" operand
    if(instr->info.a_imm) {
        instr->ap = NULL;
        instr->av = vm->memory[vm->regs[V16_REGISTER_PC]++];
    }
    else {
        instr->ap = vm->regs + instr->info.a_reg;
        instr->av = instr->ap[0];
    }

    // "B" operand
    if(instr->info.b_imm) {
        instr->bp = NULL;
        instr->bv = vm->memory[vm->regs[V16_REGISTER_PC]++];
    }
    else {
        instr->bp = vm->regs + instr->info.b_reg;
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

    switch(instr.info.opcode) {
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
        case V16_OPCODE_INC:
            V16_set(vm, instr.av + 1, instr.ap);
            return 1;
        case V16_OPCODE_DEC:
            V16_set(vm, instr.av - 1, instr.ap);
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
