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
#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <V16.h>

static const char *DASM_mnemonic(unsigned int id)
{
    #define DASM_MNEMONIC_X(x) if(id == V16_OPCODE_##x) return #x

    DASM_MNEMONIC_X(NOP);
    DASM_MNEMONIC_X(HLT);
    DASM_MNEMONIC_X(PTS);
    DASM_MNEMONIC_X(PFS);
    DASM_MNEMONIC_X(CAL);
    DASM_MNEMONIC_X(RET);
    DASM_MNEMONIC_X(IOR);
    DASM_MNEMONIC_X(IOW);
    DASM_MNEMONIC_X(MRD);
    DASM_MNEMONIC_X(MWR);
    DASM_MNEMONIC_X(CLI);
    DASM_MNEMONIC_X(STI);
    DASM_MNEMONIC_X(INT);
    DASM_MNEMONIC_X(RFI);

    DASM_MNEMONIC_X(MOV);
    DASM_MNEMONIC_X(ADD);
    DASM_MNEMONIC_X(SUB);
    DASM_MNEMONIC_X(MUL);
    DASM_MNEMONIC_X(DIV);
    DASM_MNEMONIC_X(MOD);
    DASM_MNEMONIC_X(SHL);
    DASM_MNEMONIC_X(SHR);
    DASM_MNEMONIC_X(AND);
    DASM_MNEMONIC_X(BOR);
    DASM_MNEMONIC_X(XOR);
    DASM_MNEMONIC_X(NOT);
    DASM_MNEMONIC_X(INC);
    DASM_MNEMONIC_X(DEC);

    DASM_MNEMONIC_X(IEQ);
    DASM_MNEMONIC_X(INE);
    DASM_MNEMONIC_X(IGT);
    DASM_MNEMONIC_X(IGE);
    DASM_MNEMONIC_X(ILT);
    DASM_MNEMONIC_X(ILE);

    #undef DASM_MNEMONIC_X
    return "???";
}

static const char *DASM_register(unsigned int id)
{
    #define DASM_REGISTER_X(x) if(id == V16_REGISTER_##x) return #x

    DASM_REGISTER_X(R0);
    DASM_REGISTER_X(R1);
    DASM_REGISTER_X(R2);
    DASM_REGISTER_X(R3);
    DASM_REGISTER_X(R4);
    DASM_REGISTER_X(R5);
    DASM_REGISTER_X(R6);
    DASM_REGISTER_X(R7);
    DASM_REGISTER_X(R8);
    DASM_REGISTER_X(R9);
    DASM_REGISTER_X(RI);
    DASM_REGISTER_X(RJ);
    DASM_REGISTER_X(IA);
    DASM_REGISTER_X(OF);
    DASM_REGISTER_X(SP);
    DASM_REGISTER_X(PC);

    #undef DASM_REGISTER_X
    return "??";
}

static void DASM_print(FILE *fp, int offsets, int words, uint16_t addr, uint16_t word, const V16_instruction_t *instr, const uint16_t *imms)
{
    if(offsets)
        fprintf(fp, "%04X  ", addr);
    if(words) {
        fprintf(fp, "%04X ", word);
        fprintf(fp, instr->a_imm ? "%04X " : "**** ", imms[0]);
        fprintf(fp, instr->b_imm ? "%04X " : "**** ", imms[1]);
        fprintf(fp, " ");
    }
    fprintf(fp, "%s ", DASM_mnemonic(instr->opcode));
    if(instr->a_imm)
        fprintf(fp, "$0x%04X", imms[0]);
    else
        fprintf(fp, "%%%s", DASM_register(instr->a_reg));
    if(instr->b_imm)
        fprintf(fp, ", $0x%04X", imms[1]);
    else
        fprintf(fp, ", %%%s", DASM_register(instr->b_reg));
    fprintf(fp, "\n");
}

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

int main(int argc, char **argv)
{
    FILE *infile = NULL;
    size_t begin = 0x0000, end = V16_MEM_SIZE;
    int offsets = 0, words = 0;

    int r;
    while((r = getopt(argc, argv, "b:e:OWh")) != EOF) {
        switch(r) {
            case 'b':
                begin = (uint16_t)strtol(optarg, NULL, 16);
                break;
            case 'e':
                end = (uint16_t)strtol(optarg, NULL, 16);
                break;
            case 'O':
                offsets = 1;
                break;
            case 'W':
                words = 1;
                break;
            default:
                lprintf("Usage: V16DASM [-b <hexaddr>] [-e <hexaddr>] [-O] [-W] [-h] <infile>\n");
                return 1;
        }
    }
    
    if(optind >= argc) {
        lprintf("V16DASM: fatal: infile is null!\n");
        return 1;
    }

    infile = fopen(argv[optind], "rb");
    if(!infile) {
        lprintf("V16DASM: fatal: infile is null!\n");
        return 1;
    }

    uint16_t *memory = malloc(sizeof(uint16_t) * V16_MEM_SIZE);

    fseek(infile, 0, SEEK_END);
    long size = ftell(infile) / sizeof(uint16_t);
    fseek(infile, 0, SEEK_SET);

    if(size > V16_MEM_SIZE)
        size = V16_MEM_SIZE;

    if(end > size)
        end = size;

    fread(memory, sizeof(uint16_t), size, infile);
    fclose(infile);

    // Patch endianness
    for(size_t i = 0; i < V16_MEM_SIZE; i++)
        memory[i] = V16_BE16ToHost(memory[i]);

    fprintf(stdout, "%c V16 disassembler (V16DASM)\n", (offsets || words) ? ' ' : '#');
    fprintf(stdout, "%c source file: %s\n", (offsets || words) ? ' ' : '#', argv[optind]);

    for(size_t i = begin; i < end; i++) {
        uint16_t addr = (uint16_t)i;
        uint16_t word = memory[addr];

        V16_instruction_t instr = { 
            .opcode = (word >> 10) & 0x3F,
            .a_imm = (word >> 9) & 0x01,
            .a_reg = (word >> 5) & 0x0F,
            .b_imm = (word >> 4) & 0x01,
            .b_reg = word & 0x0F,
        };

        uint16_t imms[2] = { 0 };
        if(instr.a_imm && ++i < end)
            imms[0] = memory[i];
        if(instr.b_imm && ++i < end)
            imms[1] = memory[i];
        DASM_print(stdout, offsets, words, addr, word, &instr, imms);
    }

    free(memory);
    return 0;
}
