#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vcpu16.h>

static const char *get_mnemonic(unsigned int id)
{
    #define _mnemonic_x(x) if(id == VCPU_OPCODE_##x) return #x

    _mnemonic_x(NOP);
    _mnemonic_x(HLT);
    _mnemonic_x(PTS);
    _mnemonic_x(PFS);
    _mnemonic_x(CAL);
    _mnemonic_x(RET);
    _mnemonic_x(IOR);
    _mnemonic_x(IOW);
    _mnemonic_x(MRD);
    _mnemonic_x(MWR);
    _mnemonic_x(CLI);
    _mnemonic_x(STI);
    _mnemonic_x(INT);
    _mnemonic_x(RFI);
    _mnemonic_x(IEQ);
    _mnemonic_x(INE);
    _mnemonic_x(IGT);
    _mnemonic_x(IGE);
    _mnemonic_x(ILT);
    _mnemonic_x(ILE);
    _mnemonic_x(MOV);
    _mnemonic_x(ADD);
    _mnemonic_x(SUB);
    _mnemonic_x(MUL);
    _mnemonic_x(DIV);
    _mnemonic_x(MOD);
    _mnemonic_x(SHL);
    _mnemonic_x(SHR);
    _mnemonic_x(AND);
    _mnemonic_x(BOR);
    _mnemonic_x(XOR);
    _mnemonic_x(NOT);
    _mnemonic_x(INC);
    _mnemonic_x(DEC);
    return "???";

    #undef _mnemonic_x
}

static const char *get_register(unsigned int id)
{
    #define _register_x(x) if(id == VCPU_REGISTER_##x) return #x

    _register_x(R0);
    _register_x(R1);
    _register_x(R2);
    _register_x(R3);
    _register_x(R4);
    _register_x(R5);
    _register_x(R6);
    _register_x(R7);
    _register_x(R8);
    _register_x(R9);
    _register_x(RI);
    _register_x(RJ);
    _register_x(IA);
    _register_x(OF);
    _register_x(SP);
    _register_x(PC);
    return "??";

    #undef _register_x
}

static void dis_print(FILE *fp, int offsets, int words, unsigned short addr, unsigned short word, const struct vcpu_instruction *instruction, const unsigned short *imms)
{
    if(offsets)
        fprintf(fp, "%04X  ", addr);
    if(words) {
        fprintf(fp, "%04X ", word);
        fprintf(fp, instruction->a.imm ? "%04X " : "**** ", imms[0]);
        fprintf(fp, instruction->b.imm ? "%04X " : "**** ", imms[1]);
        fprintf(fp, " ");
    }
    fprintf(fp, "%s ", get_mnemonic(instruction->opcode));
    if(instruction->a.imm)
        fprintf(fp, "$0x%04X", imms[0]);
    else
        fprintf(fp, "%%%s", get_register(instruction->a.reg));
    if(instruction->b.imm)
        fprintf(fp, ", $0x%04X", imms[1]);
    else
        fprintf(fp, ", %%%s", get_register(instruction->b.reg));
    fprintf(fp, "\n");
}

static char print_buffer[4096] = { 0 };
static const char *argv_0 = NULL;
static const char *infile_name = NULL;

#define _ansi_reset     "\033[0m"
#define _ansi_warning   "\033[1;35m"
#define _ansi_error     "\033[1;31m"

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, ap);
    fprintf(stderr, "%s\n", print_buffer);
    va_end(ap);
}

static void error(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, va);
    if(infile_name)
        fprintf(stderr, "%s: %serror: %s%s\n", infile_name, _ansi_error, _ansi_reset, print_buffer);
    else
        fprintf(stderr, "%s: %sfatal: %s%s\n", argv_0, _ansi_error, _ansi_reset, print_buffer);
    va_end(va);
    exit(1);
}

int main(int argc, char **argv)
{
    int r;
    FILE *infile = NULL;
    size_t begin = 0x0000, end = VCPU_MEM_SIZE;
    int offsets = 0, words = 0;
    vcpu_memory_t memory;
    size_t size, i;
    unsigned short addr, word;
    unsigned short imms[2];
    struct vcpu_instruction instruction;

    argv_0 = argv[0];

    while((r = getopt(argc, argv, "b:e:OWvh")) != EOF) {
        switch(r) {
            case 'b':
                begin = (unsigned short)strtol(optarg, NULL, 16);
                break;
            case 'e':
                end = (unsigned short)strtol(optarg, NULL, 16);
                break;
            case 'O':
                offsets = 1;
                break;
            case 'W':
                words = 1;
                break;
            case 'v':
                lprintf("%s (VCPU DIS) version 0.0.x", argv_0);
                return 0;
            default:
                lprintf("Usage: %s [-b <hexaddr>] [-e <hexaddr>] [-O] [-W] [-h] <infile>", argv[0]);
                lprintf("Options:");
                lprintf("   -b <hexaddr>    : Set the begining offset.");
                lprintf("   -e <hexaddr>    : Set the ending offset.");
                lprintf("   -O              : Write offsets.");
                lprintf("   -W              : Write instruction words.");
                lprintf("   -v              : Print version and exit");
                lprintf("   -h              : Write this message and exit.");
                lprintf("   <infile>        : Input binary (ROM).");
                return (r == 'h');
        }
    }
    
    if(optind >= argc)
        error("no input files");

    infile_name = argv[optind];
    infile = fopen(infile_name, "rb");
    if(!infile)
        error("%s", strerror(errno));

    fseek(infile, 0, SEEK_END);
    size = ftell(infile) / sizeof(unsigned short);
    fseek(infile, 0, SEEK_SET);

    if(size > VCPU_MEM_SIZE)
        size = VCPU_MEM_SIZE;

    if(end > size)
        end = size;

    fread(memory, sizeof(unsigned short), size, infile);
    fclose(infile);

    // Patch endianness
    for(i = 0; i < VCPU_MEM_SIZE; i++)
        memory[i] = vcpu_be16_to_host(memory[i]);

    for(i = begin; i < end; i++) {
        addr = (unsigned short)i;
        word = memory[addr];
        instruction.opcode = (word >> 10) & 0x3F;
        instruction.a.imm = (word >> 9) & 0x01;
        instruction.a.reg = (word >> 5) & 0x0F;
        instruction.b.imm = (word >> 4) & 0x01;
        instruction.b.reg = word & 0x0F;
        if(instruction.a.imm && ++i < end)
            imms[0] = memory[i];
        if(instruction.b.imm && ++i < end)
            imms[1] = memory[i];
        dis_print(stdout, offsets, words, addr, word, &instruction, imms);
    }

    return 0;
}
