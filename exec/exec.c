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
#include <time.h>
#include <V16.h>

static const char *EXEC_register(uint16_t id)
{
    #define EXEC_REGISTER_X(x) if(id == V16_REGISTER_##x) return #x

    EXEC_REGISTER_X(R0);
    EXEC_REGISTER_X(R1);
    EXEC_REGISTER_X(R2);
    EXEC_REGISTER_X(R3);
    EXEC_REGISTER_X(R4);
    EXEC_REGISTER_X(R5);
    EXEC_REGISTER_X(R6);
    EXEC_REGISTER_X(R7);
    EXEC_REGISTER_X(R8);
    EXEC_REGISTER_X(R9);
    EXEC_REGISTER_X(RI);
    EXEC_REGISTER_X(RJ);
    EXEC_REGISTER_X(IA);
    EXEC_REGISTER_X(OF);
    EXEC_REGISTER_X(SP);
    EXEC_REGISTER_X(PC);

    #undef EXEC_REGISTER_X
    return "??";
}

static void EXEC_dumpMemory(FILE *fp, const V16_vm_t *vm, uint16_t begin, uint16_t end, int grad)
{
    for(uint16_t i = begin; i < end; i++) {
        fprintf(fp, "%04X ", vm->memory[i]);
        if(!((i + 1) % grad))
            fprintf(fp, "\n");
    }
}

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static bool EXEC_ioread(V16_vm_t *vm, uint16_t port, uint16_t *value)
{
    if(port == 0x00FF) {
        value[0] = (uint16_t)fgetc(stdin);
        return true;
    }

    return false;
}

static void EXEC_iowrite(V16_vm_t *vm, uint16_t port, uint16_t value)
{
    if(port == 0x00FF) {
        fputc(value & 0xFF, stdout);
        return;
    }
}

int main(int argc, char **argv)
{
    FILE *infile = NULL;
    unsigned int maxtime = 10;
    int regdump = 0;
    V16_vm_t vm;

    if(!V16_open(&vm)) {
        lprintf("V16EXEC: fatal: unable to create VM!\n");
        return 1;
    }

    vm.ioread = EXEC_ioread;
    vm.iowrite = EXEC_iowrite;

    int r;
    while((r = getopt(argc, argv, "t:Rh")) != EOF) {
        switch(r) {
            case 't':
                maxtime = (unsigned int)strtoul(optarg, NULL, 10);
                break;
            case 'R':
                regdump = 1;
                break;
            default:
                lprintf("Usage: V16EXEC [-t <time>] [-h] <infile>\n");
                return 1;
        }
    }

    if(optind >= argc) {
        lprintf("V16EXEC: fatal: infile is null!\n");
        return 1;
    }

    infile = fopen(argv[optind], "rb");
    if(!infile) {
        lprintf("V16EXEC: fatal: infile is null!\n");
        return 1;
    }

    fseek(infile, 0, SEEK_END);
    long size = ftell(infile) / sizeof(uint16_t);
    fseek(infile, 0, SEEK_SET);

    if(size > V16_MEM_SIZE)
        size = V16_MEM_SIZE;
    
    fread(vm.memory, sizeof(uint16_t), size, infile);
    fclose(infile);

    // Patch endianness
    for(size_t i = 0; i < V16_MEM_SIZE; i++)
        vm.memory[i] = V16_BE16ToHost(vm.memory[i]);

    clock_t starttime = clock();
    while(V16_step(&vm)) {
        if(maxtime) {
            clock_t curtime = clock();
            if((curtime - starttime) / CLOCKS_PER_SEC >= maxtime) {
                lprintf("V16EXEC: stopped after %u seconds\n", maxtime);
                break;
            }
        }
    }

    if(regdump) {
        for(uint16_t i = 0; i < V16_REGISTER_COUNT; i++)
            lprintf("%s = 0x%04X\n", EXEC_register(i), vm.regs[i]);
    }

    V16_close(&vm);
    return 0;
}
