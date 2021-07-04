#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <V16.h>

static const char *DASM_mnemonic(uint16_t id)
{
    #define DASM_MNEMONIC_X(x) if(id == V16_OPCODE_##x) return #x

    DASM_MNEMONIC_X(NOP);
    DASM_MNEMONIC_X(HLT);
    DASM_MNEMONIC_X(PTS);
    DASM_MNEMONIC_X(PFS);
    DASM_MNEMONIC_X(SCL);
    DASM_MNEMONIC_X(SRT);
    DASM_MNEMONIC_X(IOR);
    DASM_MNEMONIC_X(IOW);
    DASM_MNEMONIC_X(MRD);
    DASM_MNEMONIC_X(MWR);
//  DASM_MNEMONIC_X(CID);

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

    DASM_MNEMONIC_X(IEQ);
    DASM_MNEMONIC_X(INE);
    DASM_MNEMONIC_X(IGT);
    DASM_MNEMONIC_X(IGE);
    DASM_MNEMONIC_X(ILT);
    DASM_MNEMONIC_X(ILE);

    #undef DASM_MNEMONIC_X
    return "???";
}

static const char *DASM_register(uint16_t id)
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

static void DASM_print(FILE *fp, int offsets, int words, uint16_t addr, const V16_instruction_t *instr, const uint16_t *imms)
{
    if(offsets)
        fprintf(fp, "%04X  ", addr);
    if(words) {
        fprintf(fp, "%04X ", instr->word);
        fprintf(fp, instr->i.a_imm ? "%04X " : "**** ", imms[0]);
        fprintf(fp, instr->i.b_imm ? "%04X " : "**** ", imms[1]);
        fprintf(fp, " ");
    }
    fprintf(fp, "%s ", DASM_mnemonic(instr->i.opcode));
    if(instr->i.a_imm)
        fprintf(fp, "$0x%04X", imms[0]);
    else
        fprintf(fp, "%%%s", DASM_register(instr->i.a_reg));
    if(instr->i.b_imm)
        fprintf(fp, ", $0x%04X", imms[1]);
    else
        fprintf(fp, ", %%%s", DASM_register(instr->i.b_reg));
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
        V16_instruction_t instr = { .word = memory[i] };
        uint16_t imms[2] = { 0 };
        if(instr.i.a_imm && ++i < end)
            imms[0] = memory[i];
        if(instr.i.b_imm && ++i < end)
            imms[1] = memory[i];
        DASM_print(stdout, offsets, words, addr, &instr, imms);
    }

    free(memory);
    return 0;
}
